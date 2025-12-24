#include <iostream>
#include "someip.hpp"
#include "logging.hpp"
#include "persistence.hpp"

int main() {
    // Initialize logging
    initLogging();

    // Load persistent state
    auto mediaState = loadMediaState();

    // Initialize SOME/IP communication
    SomeIpService mediaService;

    // Start the media service
    mediaService.start();

    std::cout << "Media Service is running..." << std::endl;

    // Main loop for handling requests
    while (true) {
        // Handle incoming requests and events
        mediaService.handleRequests();
    }

    // Cleanup and shutdown
    mediaService.stop();
    saveMediaState(mediaState);

    return 0;
}