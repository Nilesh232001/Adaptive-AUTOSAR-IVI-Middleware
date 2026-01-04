#include "someip_shim.hpp"
#include "logging.hpp"
#include <fstream>
#include <iostream>

namespace common::shim {

void load_json(const std::string& filename, json& data) {
    try {
        std::ifstream ifs(filename);
        if (ifs.is_open()) {
            ifs >> data;
        } else {
            log_warning("File not found: " + filename);
            data = json::object();
        }
    } catch (const std::exception& e) {
        log_error("Error loading JSON from " + filename + ": " + std::string(e.what()));
        data = json::object();
    }
}

void save_json(const std::string& filename, const json& data) {
    try {
        std::ofstream ofs(filename);
        if (ofs.is_open()) {
            ofs << data.dump(4);
            ofs.close();
        } else {
            log_error("Cannot open file for writing: " + filename);
        }
    } catch (const std::exception& e) {
        log_error("Error saving JSON to " + filename + ": " + std::string(e.what()));
    }
}

void send_message(const std::string& host, int port, const json& msg, json& reply) {
    // Simplified stub - in a real implementation, this would use SOME/IP protocol
    try {
        log_info("Sending message to " + host + ":" + std::to_string(port));
        // For now, just log the operation; real implementation would use sockets
        reply = json::object();
        reply["status"] = "ok";
    } catch (const std::exception& e) {
        log_error("Error sending message: " + std::string(e.what()));
        reply = json::object();
        reply["error"] = std::string(e.what());
    }
}

} // namespace common::shim
