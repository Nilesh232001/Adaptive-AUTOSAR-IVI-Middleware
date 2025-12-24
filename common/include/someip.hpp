#ifndef SOMEIP_HPP
#define SOMEIP_HPP

#include <string>
#include <vector>
#include <cstdint>

class SomeIPMessage {
public:
    SomeIPMessage(uint16_t serviceId, uint16_t methodId, const std::vector<uint8_t>& payload);
    
    uint16_t getServiceId() const;
    uint16_t getMethodId() const;
    const std::vector<uint8_t>& getPayload() const;

private:
    uint16_t serviceId;
    uint16_t methodId;
    std::vector<uint8_t> payload;
};

class SomeIPClient {
public:
    SomeIPClient(const std::string& host, uint16_t port);
    void sendMessage(const SomeIPMessage& message);
    SomeIPMessage receiveMessage();

private:
    std::string host;
    uint16_t port;
};

class SomeIPServer {
public:
    SomeIPServer(uint16_t port);
    void start();
    void stop();
    void registerService(uint16_t serviceId);
    void handleRequests();

private:
    uint16_t port;
};

#endif // SOMEIP_HPP