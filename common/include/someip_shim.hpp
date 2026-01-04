#ifndef SOMEIP_SHIM_HPP
#define SOMEIP_SHIM_HPP

#include <string>
#include <nlohmann/json.hpp>

// SOME/IP Shim - provides simplified interface for RPC and messaging

namespace common::shim {

using json = nlohmann::json;

// Load JSON from file
void load_json(const std::string& filename, json& data);

// Save JSON to file
void save_json(const std::string& filename, const json& data);

// Send message and optionally receive reply
void send_message(const std::string& host, int port, const json& msg, json& reply);

} // namespace common::shim

#endif // SOMEIP_SHIM_HPP
