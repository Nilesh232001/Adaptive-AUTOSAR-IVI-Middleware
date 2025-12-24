#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <cstdlib>
#include <ctime>
#include <nlohmann/json.hpp>
#include "../../common/include/logging.hpp"
#include "../../common/include/persistence.hpp"
#include "../../common/include/someip_shim.hpp"

using json = nlohmann::json;
using namespace common;
using namespace common::shim;

int main()
{
    log_info("Climate Service starting");
    const int rpc_port = 5003;
    std::atomic_bool running{false};
    std::thread server_thread;
    std::mutex state_mtx;

    // persistent state
    json state;
    const std::string persist_file = "climate_state.json";
    load_json(persist_file, state);
    if (state.empty()) {
        state["temperature"] = 22;
        state["fan_speed"] = 3;
        state["mode"] = "auto";
        state["ac_enabled"] = true;
    }

    // register with Service Manager
    json reg;
    reg["type"] = "register";
    reg["service"] = "climate";
    reg["host"] = "127.0.0.1";
    reg["port"] = rpc_port;
    json ignored_reply;
    send_message("127.0.0.1", 4000, reg, ignored_reply);
    log_info("Registered with Service Manager");

    // RPC handler
    auto handler = [&](const json &req, const std::string &peer){
        json resp;
        try {
            std::string method = req.value("method", "");
            if (method == "set_temperature") {
                int temp = req.at("params").value("temperature", state.value("temperature", 22));
                // Clamp temperature between 16 and 32 Celsius
                temp = std::max(16, std::min(32, temp));
                {
                    std::lock_guard<std::mutex> lk(state_mtx);
                    state["temperature"] = temp;
                    save_json(persist_file, state);
                }
                resp["result"] = "ok";
                resp["state"] = state;
                log_info("Temperature set to " + std::to_string(temp) + "°C");
            } else if (method == "set_fan_speed") {
                int fan = req.at("params").value("fan_speed", state.value("fan_speed", 3));
                // Clamp fan speed between 0 (off) and 5 (max)
                fan = std::max(0, std::min(5, fan));
                {
                    std::lock_guard<std::mutex> lk(state_mtx);
                    state["fan_speed"] = fan;
                    save_json(persist_file, state);
                }
                resp["result"] = "ok";
                resp["state"] = state;
                log_info("Fan speed set to " + std::to_string(fan));
            } else if (method == "set_mode") {
                std::string mode = req.at("params").value("mode", state.value("mode", std::string("auto")));
                // Validate mode
                if (mode != "auto" && mode != "cool" && mode != "heat" && mode != "dry") {
                    resp["error"] = "invalid_mode";
                    return;
                }
                {
                    std::lock_guard<std::mutex> lk(state_mtx);
                    state["mode"] = mode;
                    save_json(persist_file, state);
                }
                resp["result"] = "ok";
                resp["state"] = state;
                log_info("Mode set to " + mode);
            } else if (method == "set_ac") {
                bool ac_on = req.at("params").value("ac_enabled", state.value("ac_enabled", true));
                {
                    std::lock_guard<std::mutex> lk(state_mtx);
                    state["ac_enabled"] = ac_on;
                    save_json(persist_file, state);
                }
                resp["result"] = "ok";
                resp["state"] = state;
                log_info(std::string("AC ") + (ac_on ? "enabled" : "disabled"));
            } else if (method == "get_state") {
                std::lock_guard<std::mutex> lk(state_mtx);
                resp["state"] = state;
            } else {
                resp["error"] = "unknown_method";
            }
        } catch (std::exception &e) {
            resp["error"] = std::string("exception: ") + e.what();
        }
        // Shim now handles sending reply back on same socket
        std::cout << "Handled RPC (peer=" << peer << "): req=" << req.dump() << ", resp=" << resp.dump() << std::endl;
    };

    if (!start_server(rpc_port, handler, server_thread, running)) {
        log_error("Failed to start climate RPC server");
        return 1;
    }
    running = true;
    log_info("Climate Service RPC listening on port " + std::to_string(rpc_port));

    // Sensor simulation / temperature monitoring thread
    std::thread sensor_thread([&](){
        std::srand((unsigned)std::time(nullptr));
        int ambient_temp = 25; // simulated ambient temperature
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(8));
            json ev;
            bool should_send = false;
            {
                std::lock_guard<std::mutex> lk(state_mtx);
                // Simulate ambient temperature change
                ambient_temp += (std::rand() % 3) - 1; // +1, 0, or -1
                ambient_temp = std::max(15, std::min(35, ambient_temp));
                
                // Simulate temperature adjustment based on AC mode
                int current_temp = state.value("temperature", 22);
                if (state.value("ac_enabled", true)) {
                    if (state.value("mode", std::string("auto")) == "cool" && current_temp > ambient_temp - 2) {
                        current_temp--;
                    } else if (state.value("mode", std::string("auto")) == "heat" && current_temp < ambient_temp + 2) {
                        current_temp++;
                    }
                }
                state["temperature"] = current_temp;
                save_json(persist_file, state);
                
                ev["type"] = "event";
                ev["service"] = "climate";
                ev["event"] = "temperature_update";
                ev["current_temperature"] = state["temperature"];
                ev["ambient_temperature"] = ambient_temp;
                ev["mode"] = state["mode"];
                ev["fan_speed"] = state["fan_speed"];
                should_send = true;
            }
            if (should_send) {
                json r;
                if (!send_message("127.0.0.1", 4000, ev, r)) {
                    log_warn("Failed to send climate event to service manager");
                } else {
                    log_info("Sent temperature update event");
                }
            }
        }
    });

    // Simple CLI
    std::string line;
    while (true) {
        std::cout << "climate> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line == "state") {
            std::lock_guard<std::mutex> lk(state_mtx);
            std::cout << state.dump(2) << std::endl;
            continue;
        }
        if (line.rfind("temp ", 0) == 0) {
            try {
                int t = std::stoi(line.substr(5));
                json req; req["method"] = "set_temperature"; req["params"] = { {"temperature", t} };
                json r;
                if (send_message("127.0.0.1", rpc_port, req, r)) {
                    std::cout << "reply: " << r.dump() << std::endl;
                } else {
                    std::cout << "call failed\n";
                }
            } catch (...) {
                std::cout << "invalid temperature\n";
            }
            continue;
        }
        if (line.rfind("fan ", 0) == 0) {
            try {
                int f = std::stoi(line.substr(4));
                json req; req["method"] = "set_fan_speed"; req["params"] = { {"fan_speed", f} };
                json r;
                if (send_message("127.0.0.1", rpc_port, req, r)) {
                    std::cout << "reply: " << r.dump() << std::endl;
                } else {
                    std::cout << "call failed\n";
                }
            } catch (...) {
                std::cout << "invalid fan speed\n";
            }
            continue;
        }
        if (line.rfind("mode ", 0) == 0) {
            std::string m = line.substr(5);
            json req; req["method"] = "set_mode"; req["params"] = { {"mode", m} };
            json r;
            if (send_message("127.0.0.1", rpc_port, req, r)) {
                std::cout << "reply: " << r.dump() << std::endl;
            } else {
                std::cout << "call failed\n";
            }
            continue;
        }
        if (line.rfind("ac ", 0) == 0) {
            std::string ac_cmd = line.substr(3);
            bool ac_on = (ac_cmd == "on" || ac_cmd == "1");
            json req; req["method"] = "set_ac"; req["params"] = { {"ac_enabled", ac_on} };
            json r;
            if (send_message("127.0.0.1", rpc_port, req, r)) {
                std::cout << "reply: " << r.dump() << std::endl;
            } else {
                std::cout << "call failed\n";
            }
            continue;
        }
        std::cout << "commands: state | temp <16-32> | fan <0-5> | mode <auto|cool|heat|dry> | ac <on|off> | exit\n";
    }

    running = false;
    if (server_thread.joinable()) server_thread.join();
    if (sensor_thread.joinable()) sensor_thread.join();
    log_info("Climate Service exiting");
    return 0;
}