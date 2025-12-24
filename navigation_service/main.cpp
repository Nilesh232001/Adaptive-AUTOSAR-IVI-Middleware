// Navigation Service
// Main entry point placeholder for service or HMI
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include "../common/include/logging.hpp"
#include "../common/include/persistence.hpp"
#include "../common/include/someip_shim.hpp"

using json = nlohmann::json;
using namespace common;
using namespace common::shim;

int main()
{
    log_info("Navigation Service starting");
    const int rpc_port = 5002;
    std::atomic_bool running{false};
    std::thread server_thread;
    std::mutex state_mtx;

    // persistent state
    json state;
    const std::string persist_file = "navigation_state.json";
    load_json(persist_file, state);
    if (state.empty()) {
        state["destination"] = "";
        state["status"] = "idle";
        state["progress"] = 0;
    }

    // register with Service Manager
    json reg;
    reg["type"] = "register";
    reg["service"] = "navigation";
    reg["host"] = "127.0.0.1";
    reg["port"] = rpc_port;
    json ignored_reply;
    send_message("127.0.0.1", 4000, reg, ignored_reply);

    // RPC handler
    auto handler = [&](const json &req, const std::string &peer){
        json resp;
        try {
            std::string method = req.value("method", "");
            if (method == "set_destination") {
                std::lock_guard<std::mutex> lk(state_mtx);
                state["destination"] = req.at("params").value("destination", "");
                state["status"] = "navigating";
                state["progress"] = 0;
                save_json(persist_file, state);
                resp["result"] = "ok";
                resp["state"] = state;
                log_info("set_destination -> " + state["destination"].get<std::string>());
            } else if (method == "get_status") {
                std::lock_guard<std::mutex> lk(state_mtx);
                resp["state"] = state;
            } else if (method == "cancel") {
                std::lock_guard<std::mutex> lk(state_mtx);
                state["status"] = "idle";
                state["destination"] = "";
                state["progress"] = 0;
                save_json(persist_file, state);
                resp["result"] = "cancelled";
            } else {
                resp["error"] = "unknown_method";
            }
        } catch (std::exception &e) {
            resp["error"] = std::string("exception: ") + e.what();
        }

        // If requester provided reply_host/reply_port, send response back explicitly.
        if (req.contains("reply_host") && req.contains("reply_port")) {
            std::string rh = req.value("reply_host", std::string("127.0.0.1"));
            int rp = req.value("reply_port", 0);
            if (rp > 0) {
                json rback = { {"reply_to", "navigation"}, {"response", resp} };
                json rcv;
                if (!send_message(rh, rp, rback, rcv)) {
                    log_warn("Failed to send RPC reply back to client at " + rh + ":" + std::to_string(rp));
                }
            }
        } else {
            // For the simplified shim: also log the response so callers can see it.
            std::cout << "Handled RPC (peer=" << peer << "): " << req.dump() << " -> " << resp.dump() << std::endl;
        }
    };

    if (!start_server(rpc_port, handler, server_thread, running)) {
        log_error("Failed to start navigation RPC server");
        return 1;
    }
    running = true;
    log_info("Navigation Service RPC listening on port " + std::to_string(rpc_port));

    // GPS simulation / progress events thread
    std::thread gps_thread([&](){
        std::srand((unsigned)std::time(nullptr));
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            json ev;
            bool should_send = false;
            {
                std::lock_guard<std::mutex> lk(state_mtx);
                if (state.value("status", std::string("")) == "navigating") {
                    int prog = state.value("progress", 0);
                    prog = std::min(100, prog + (5 + (std::rand() % 10)));
                    state["progress"] = prog;
                    if (prog >= 100) {
                        state["status"] = "arrived";
                    }
                    save_json(persist_file, state);
                    ev["type"] = "event";
                    ev["service"] = "navigation";
                    ev["event"] = "progress";
                    ev["progress"] = state["progress"];
                    ev["destination"] = state["destination"];
                    should_send = true;
                }
            }
            if (should_send) {
                json r;
                if (!send_message("127.0.0.1", 4000, ev, r)) {
                    log_warn("Failed to send navigation progress event to service manager");
                }
            }
        }
    });

    // Simple CLI for manual control
    std::string line;
    while (true) {
        std::cout << "navigation> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line.rfind("go ", 0) == 0) {
            std::string dest = line.substr(3);
            json req; req["method"] = "set_destination"; req["params"] = { {"destination", dest} };
            // call self endpoint to demonstrate client behavior
            json r;
            if (send_message("127.0.0.1", rpc_port, req, r)) {
                std::cout << "set_destination reply: " << r.dump() << std::endl;
            } else {
                std::cout << "failed to call local RPC endpoint\n";
            }
            continue;
        }
        if (line == "state") {
            std::lock_guard<std::mutex> lk(state_mtx);
            std::cout << state.dump(2) << std::endl;
            continue;
        }
        if (line == "cancel") {
            json req; req["method"] = "cancel";
            json r;
            if (send_message("127.0.0.1", rpc_port, req, r)) {
                std::cout << "cancel reply: " << r.dump() << std::endl;
            } else {
                std::cout << "failed to call local RPC endpoint\n";
            }
            continue;
        }
        std::cout << "commands: go <destination> | state | cancel | exit\n";
    }

    running = false;
    if (server_thread.joinable()) server_thread.join();
    if (gps_thread.joinable()) gps_thread.join();
    log_info("Navigation Service exiting");
    return 0;
}