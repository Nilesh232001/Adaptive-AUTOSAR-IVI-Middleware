#include <iostream>
#include "someip.hpp"
#include "logging.hpp"
#include "persistence.hpp"

int main() {
    // Initialize logging
    initLogging();

    // Load persistent state
    auto navigationState = loadNavigationState();

    // Initialize SOME/IP communication
    SomeIpCommunication someIpComm;
    someIpComm.initialize();

    // Start the navigation service
    std::cout << "Starting Navigation Service..." << std::endl;

    // Main service loop
    while (true) {
        // Handle incoming SOME/IP messages
        someIpComm.handleMessages();

        // Update navigation state and publish events
        updateNavigationState(navigationState);
        publishNavigationUpdates(navigationState);
    }

    return 0;
}