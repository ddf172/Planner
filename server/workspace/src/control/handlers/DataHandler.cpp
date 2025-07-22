#include "control/handlers/DataHandler.hpp"
#include "core/System.hpp"
#include "extern/nlohmann/json.hpp"
#include <iostream>
#include <ctime>

using json = nlohmann::json;

void DataHandler::handle(const std::string& messageId, const std::string& payload, System& system) {
    std::cout << "DataHandler: Received message " << messageId << std::endl;
    
    // Send acknowledgment that data was received
    json ackResponse = {
        {"status", "success"},
        {"message", "Data received and processed"},
        {"message_id", messageId},
        {"timestamp", std::time(nullptr)}
    };
    
    system.sendMessage(messageId, ackResponse.dump(), MessageType::Data);
}

MessageType DataHandler::getHandledType() const {
    return MessageType::Data;
}