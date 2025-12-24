#include <iostream>
#include <string>
#include "someip.hpp"
#include "logging.hpp"

class HMIClient {
public:
    HMIClient() {
        // Initialize the client
        logInfo("HMI Client initialized.");
    }

    void run() {
        // Main loop for handling user interactions
        std::string command;
        while (true) {
            std::cout << "Enter command (play, pause, stop, exit): ";
            std::getline(std::cin, command);

            if (command == "exit") {
                logInfo("Exiting HMI Client.");
                break;
            } else {
                handleCommand(command);
            }
        }
    }

private:
    void handleCommand(const std::string& command) {
        if (command == "play") {
            // Send play command to Media Service
            logInfo("Sending play command to Media Service.");
            // SOME/IP call to Media Service would go here
        } else if (command == "pause") {
            // Send pause command to Media Service
            logInfo("Sending pause command to Media Service.");
            // SOME/IP call to Media Service would go here
        } else if (command == "stop") {
            // Send stop command to Media Service
            logInfo("Sending stop command to Media Service.");
            // SOME/IP call to Media Service would go here
        } else {
            logWarning("Unknown command: " + command);
        }
    }
};

int main() {
    HMIClient client;
    client.run();
    return 0;
}