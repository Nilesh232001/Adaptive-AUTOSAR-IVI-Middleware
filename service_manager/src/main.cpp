#include <iostream>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <nlohmann/json.hpp>
#include "../../common/include/logging.hpp"
#include "../../common/include/persistence.hpp"
#include "../../common/include/someip_shim.hpp"

using json = nlohmann::json;
using namespace common;
using namespace common::shim;

struct ServiceInfo {
    std::string name;
    std::string host;
    int port;
    std::chrono::system_clock::time_point last_heartbeat;
    bool is_alive;
};

class ServiceManager {
private:
    std::unordered_map<std::string, ServiceInfo> services;
    std::mutex services_mtx;
    std::atomic_bool running{false};
    std::thread registration_server_thread;
    std::thread query_server_thread;
    std::thread heartbeat_monitor_thread;
    const int REGISTRATION_PORT = 4000;
    const int QUERY_PORT = 4001;
    const int HEARTBEAT_TIMEOUT_SEC = 30;

public:
    ServiceManager() = default;
    ~ServiceManager() = default;

    void initialize() {
        log_info("Service Manager initializing");
        running = true;
        
        // Start registration server
        if (!startRegistrationServer()) {
            log_error("Failed to start registration server");
            return;
        }
        
        // Start query server
        if (!startQueryServer()) {
            log_error("Failed to start query server");
            return;
        }
        
        // Start heartbeat monitor
        startHeartbeatMonitor();
        
        log_info("Service Manager initialized successfully");
    }

    bool startRegistrationServer() {
        auto handler = [this](const json &req, const std::string &peer) {
            this->handleRegistration(req, peer);
        };
        
        if (!start_server(REGISTRATION_PORT, handler, registration_server_thread, running)) {
            return false;
        }
        log_info("Registration server started on port " + std::to_string(REGISTRATION_PORT));
        return true;
    }

    bool startQueryServer() {
        auto handler = [this](const json &req, const std::string &peer) {
            this->handleQuery(req, peer);
        };
        
        if (!start_server(QUERY_PORT, handler, query_server_thread, running)) {
            return false;
        }
        log_info("Query server started on port " + std::to_string(QUERY_PORT));
        return true;
    }

    void startHeartbeatMonitor() {
        heartbeat_monitor_thread = std::thread([this]() {
            while (running) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
                this->checkHeartbeats();
            }
        });
    }

    void handleRegistration(const json &req, const std::string &peer) {
        try {
            std::string type = req.value("type", "");
            
            if (type == "register") {
                std::lock_guard<std::mutex> lk(services_mtx);
                
                ServiceInfo info;
                info.name = req.at("service").get<std::string>();
                info.host = req.value("host", "127.0.0.1");
                info.port = req.at("port").get<int>();
                info.last_heartbeat = std::chrono::system_clock::now();
                info.is_alive = true;
                
                services[info.name] = info;
                log_info("Service registered: " + info.name + " at " + info.host + ":" + 
                         std::to_string(info.port) + " (peer: " + peer + ")");
                printServices();
            } 
            else if (type == "heartbeat") {
                std::lock_guard<std::mutex> lk(services_mtx);
                
                std::string service_name = req.at("service").get<std::string>();
                if (services.count(service_name)) {
                    services[service_name].last_heartbeat = std::chrono::system_clock::now();
                    services[service_name].is_alive = true;
                    log_info("Heartbeat received from: " + service_name);
                }
            }
            else if (type == "event") {
                std::string service_name = req.value("service", "unknown");
                std::string event_name = req.value("event", "unknown");
                log_info("Event from " + service_name + ": " + event_name);
            }
        } catch (std::exception &e) {
            log_error("Error handling registration: " + std::string(e.what()));
        }
    }

    void handleQuery(const json &req, const std::string &peer) {
        try {
            std::string cmd = req.value("cmd", "");
            json resp;
            
            if (cmd == "list") {
                std::lock_guard<std::mutex> lk(services_mtx);
                resp["services"] = json::array();
                
                for (auto &p : services) {
                    if (p.second.is_alive) {
                        json si;
                        si["service"] = p.second.name;
                        si["host"] = p.second.host;
                        si["port"] = p.second.port;
                        si["status"] = "alive";
                        resp["services"].push_back(si);
                    }
                }
                log_info("Query 'list' from peer: " + peer + " returned " + 
                         std::to_string(resp["services"].size()) + " services");
            } 
            else if (cmd == "get") {
                std::lock_guard<std::mutex> lk(services_mtx);
                std::string service_name = req.value("service", "");
                
                if (services.count(service_name) && services[service_name].is_alive) {
                    resp["service"] = service_name;
                    resp["host"] = services[service_name].host;
                    resp["port"] = services[service_name].port;
                    resp["status"] = "found";
                } else {
                    resp["status"] = "not_found";
                    resp["service"] = service_name;
                }
                log_info("Query 'get " + service_name + "' from peer: " + peer);
            }
            else if (cmd == "status") {
                std::lock_guard<std::mutex> lk(services_mtx);
                resp["total_services"] = (int)services.size();
                resp["alive_services"] = 0;
                
                for (auto &p : services) {
                    if (p.second.is_alive) {
                        resp["alive_services"] = resp["alive_services"].get<int>() + 1;
                    }
                }
                log_info("Query 'status' from peer: " + peer);
            }
            else {
                resp["error"] = "unknown_command";
            }
            
            // Response is automatically sent by the shim on same socket
            std::cout << "Query response: " << resp.dump() << std::endl;
        } catch (std::exception &e) {
            log_error("Error handling query: " + std::string(e.what()));
        }
    }

    void checkHeartbeats() {
        std::lock_guard<std::mutex> lk(services_mtx);
        auto now = std::chrono::system_clock::now();
        
        for (auto &p : services) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - p.second.last_heartbeat).count();
            
            if (elapsed > HEARTBEAT_TIMEOUT_SEC && p.second.is_alive) {
                p.second.is_alive = false;
                log_warn("Service marked as dead (no heartbeat): " + p.first + 
                        " (timeout after " + std::to_string(elapsed) + "s)");
            }
        }
    }

    void printServices() {
        std::lock_guard<std::mutex> lk(services_mtx);
        log_info("=== Current Registered Services ===");
        for (auto &p : services) {
            std::string status = p.second.is_alive ? "alive" : "dead";
            log_info(p.first + " -> " + p.second.host + ":" + 
                    std::to_string(p.second.port) + " [" + status + "]");
        }
    }

    void runInteractiveCLI() {
        std::string line;
        while (running) {
            std::cout << "service_manager> ";
            if (!std::getline(std::cin, line)) break;
            
            if (line == "exit" || line == "quit") {
                break;
            }
            else if (line == "list") {
                std::lock_guard<std::mutex> lk(services_mtx);
                std::cout << "=== Registered Services ===" << std::endl;
                for (auto &p : services) {
                    std::string status = p.second.is_alive ? "alive" : "dead";
                    std::cout << p.first << " -> " << p.second.host << ":" 
                             << p.second.port << " [" << status << "]" << std::endl;
                }
                if (services.empty()) {
                    std::cout << "No services registered" << std::endl;
                }
            }
            else if (line.rfind("info ", 0) == 0) {
                std::string name = line.substr(5);
                std::lock_guard<std::mutex> lk(services_mtx);
                if (services.count(name)) {
                    auto &s = services[name];
                    std::cout << "Service: " << s.name << std::endl;
                    std::cout << "Host: " << s.host << std::endl;
                    std::cout << "Port: " << s.port << std::endl;
                    std::cout << "Status: " << (s.is_alive ? "alive" : "dead") << std::endl;
                } else {
                    std::cout << "Service not found: " << name << std::endl;
                }
            }
            else if (line == "status") {
                std::lock_guard<std::mutex> lk(services_mtx);
                int alive_count = 0;
                for (auto &p : services) {
                    if (p.second.is_alive) alive_count++;
                }
                std::cout << "Total services: " << services.size() << std::endl;
                std::cout << "Alive services: " << alive_count << std::endl;
            }
            else if (line == "help") {
                std::cout << "Commands:" << std::endl;
                std::cout << "  list              - List all registered services" << std::endl;
                std::cout << "  info <service>    - Get info about a service" << std::endl;
                std::cout << "  status            - Show service manager status" << std::endl;
                std::cout << "  exit              - Shutdown service manager" << std::endl;
                std::cout << "  help              - Show this help message" << std::endl;
            }
            else {
                std::cout << "Unknown command. Type 'help' for available commands." << std::endl;
            }
        }
    }

    void shutdown() {
        log_info("Service Manager shutting down");
        running = false;
        
        if (registration_server_thread.joinable()) {
            registration_server_thread.join();
        }
        if (query_server_thread.joinable()) {
            query_server_thread.join();
        }
        if (heartbeat_monitor_thread.joinable()) {
            heartbeat_monitor_thread.join();
        }
        
        log_info("Service Manager shutdown complete");
    }
};

int main()
{
    ServiceManager service_manager;
    service_manager.initialize();
    service_manager.runInteractiveCLI();
    service_manager.shutdown();
    
    return 0;
}