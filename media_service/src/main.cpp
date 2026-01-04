#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include "../../common/include/logging.hpp"
#include "../../common/include/persistence.hpp"
#include "../../common/include/someip_shim.hpp"

using json = nlohmann::json;
using namespace common;
using namespace common::shim;

int main()
{
    log_info("Media Service starting");
    const int rpc_port = 5001;
    std::atomic_bool running{false};
    std::thread server_thread;
    std::mutex state_mtx;

    // persistent state
    json state;
    const std::string persist_file = "media_state.json";
    load_json(persist_file, state);
    if (state.empty()) {
        state["playing"] = false;
        state["volume"] = 50;
        state["track"] = "Unknown";
    }

    // register with Service Manager
    json reg;
    reg["type"] = "register";
    reg["service"] = "media";
    reg["host"] = "127.0.0.1";
    reg["port"] = rpc_port;
    json ignored_reply;
    send_message("127.0.0.1", 4000, reg, ignored_reply);

    // RPC handler
    auto handler = [&](const json &req, const std::string &peer){
        json resp;
        try {
            std::string method = req.value("method", "");
            if (method == "play") {
                std::lock_guard<std::mutex> lk(state_mtx);
                state["playing"] = true;
                resp["result"] = "ok";
                resp["state"] = state;
                save_json(persist_file, state);
                log_info("Playback started");
            } else if (method == "pause") {
                std::lock_guard<std::mutex> lk(state_mtx);
                state["playing"] = false;
                resp["result"] = "ok";
                resp["state"] = state;
                save_json(persist_file, state);
                log_info("Playback paused");
            } else if (method == "stop") {
                std::lock_guard<std::mutex> lk(state_mtx);
                state["playing"] = false;
                state["track"] = "Unknown";
                resp["result"] = "ok";
                resp["state"] = state;
                save_json(persist_file, state);
                log_info("Playback stopped");
            } else if (method == "get_state") {
                std::lock_guard<std::mutex> lk(state_mtx);
                resp["state"] = state;
            } else if (method == "set_volume") {
                int vol = req.at("params").value("volume", state.value("volume", 50));
                {
                    std::lock_guard<std::mutex> lk(state_mtx);
                    state["volume"] = vol;
                    save_json(persist_file, state);
                }
                resp["result"] = "ok";
                resp["state"] = state;
                log_info("Volume set to " + std::to_string(vol));
            } else if (method == "set_track") {
                std::string t = req.at("params").value("track", state.value("track", std::string("Unknown")));
                {
                    std::lock_guard<std::mutex> lk(state_mtx);
                    state["track"] = t;
                    save_json(persist_file, state);
                }
                resp["result"] = "ok";
                resp["state"] = state;
                log_info("Track set to " + t);
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
                json rback = { {"reply_to", "media"}, {"response", resp} };
                json rcv;
                send_message(rh, rp, rback, rcv);
                log_info("Sent RPC reply back to client at " + rh + ":" + std::to_string(rp));
            }
        } else {
            // log response for visibility (simplified shim doesn't reply on same socket)
            std::cout << "Handled RPC (peer=" << peer << "): " << req.dump() << " -> " << resp.dump() << std::endl;
        }
    };

    running = true;
    server_thread = std::thread([&]() {
        log_info("Media RPC server thread started");
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    log_info("Media Service RPC listening on port " + std::to_string(rpc_port));

    // Event thread: periodically broadcast track metadata to Service Manager
    std::thread ev_thread([&](){
        std::srand((unsigned)std::time(nullptr));
        int counter = 0;
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            json ev;
            {
                std::lock_guard<std::mutex> lk(state_mtx);
                counter++;
                // simulate track change occasionally
                if ((std::rand() % 4) == 0) {
                    state["track"] = std::string("Track #") + std::to_string(counter);
                    save_json(persist_file, state);
                }
                ev["type"] = "event";
                ev["service"] = "media";
                ev["event"] = "track_update";
                ev["track"] = state["track"];
                ev["playing"] = state["playing"];
            }
            json r;
            send_message("127.0.0.1", 4000, ev, r);
            log_info("Sent track update event");
        }
    });

    // Simple CLI
    std::string line;
    while (true) {
        std::cout << "media> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line == "state") {
            std::lock_guard<std::mutex> lk(state_mtx);
            std::cout << state.dump(2) << std::endl;
            continue;
        }
        if (line.rfind("play", 0) == 0) {
            json req; req["method"] = "play";
            json r;
            send_message("127.0.0.1", rpc_port, req, r);
            std::cout << "reply: " << r.dump() << std::endl;
            continue;
        }
        if (line.rfind("pause", 0) == 0) {
            json req; req["method"] = "pause";
            json r;
            send_message("127.0.0.1", rpc_port, req, r);
            std::cout << "reply: " << r.dump() << std::endl;
            continue;
        }
        if (line.rfind("volume ", 0) == 0) {
            int v = std::stoi(line.substr(7));
            json req; req["method"] = "set_volume"; req["params"] = { {"volume", v} };
            json r;
            send_message("127.0.0.1", rpc_port, req, r);
            std::cout << "reply: " << r.dump() << std::endl;
            continue;
        }
        if (line.rfind("track ", 0) == 0) {
            std::string t = line.substr(6);
            json req; req["method"] = "set_track"; req["params"] = { {"track", t} };
            json r;
            send_message("127.0.0.1", rpc_port, req, r);
            std::cout << "reply: " << r.dump() << std::endl;
            continue;
        }
        std::cout << "commands: state | play | pause | volume <n> | track <name> | exit\n";
    }

    running = false;
    if (server_thread.joinable()) server_thread.join();
    if (ev_thread.joinable()) ev_thread.join();
    log_info("Media Service exiting");
    return 0;
}