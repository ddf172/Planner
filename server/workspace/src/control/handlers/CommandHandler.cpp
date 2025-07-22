#include "control/handlers/CommandHandler.hpp"
#include "core/System.hpp"
#include "extern/nlohmann/json.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>

using json = nlohmann::json;

void CommandHandler::handle(const std::string& messageId, const std::string& payload, System& system) {
    std::cout << "CommandHandler: Received message " << messageId << std::endl;
    
    try {
        // Try to parse as JSON
        json commandData = json::parse(payload);
        
        if (commandData.contains("command")) {
            std::string command = commandData["command"];
            
            if (command == "stop") {
                handleStopCommand(messageId, commandData, system);
            } else if (command == "status") {
                handleStatusCommand(messageId, commandData, system);
            } else if (command == "ping") {
                handlePingCommand(messageId, commandData, system);
            } else {
                // Unknown command
                json response = {
                    {"status", "error"},
                    {"message", "Unknown command: " + command},
                    {"error_code", "UNKNOWN_COMMAND"},
                    {"available_commands", {"stop", "status", "ping"}}
                };
                system.sendMessage(messageId, response.dump(), MessageType::Command);
            }
        } else {
            json response = {
                {"status", "error"},
                {"message", "No 'command' field found in payload"},
                {"error_code", "MISSING_COMMAND_FIELD"}
            };
            system.sendMessage(messageId, response.dump(), MessageType::Command);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "CommandHandler: Error parsing payload: " << e.what() << std::endl;
        json response = {
            {"status", "error"},
            {"message", "Invalid JSON format"},
            {"error_code", "INVALID_JSON"}
        };
        system.sendMessage(messageId, response.dump(), MessageType::Command);
    }
}

MessageType CommandHandler::getHandledType() const {
    return MessageType::Command;
}

void CommandHandler::handleStopCommand(const std::string& messageId, const json& commandData, System& system) {
    std::cout << "Executing STOP command - shutting down server" << std::endl;
    
    json response = {
        {"status", "success"},
        {"command", "stop"},
        {"message", "Server shutdown initiated"}
    };
    
    // Send response before stopping the system
    try {
        system.sendMessage(messageId, response.dump(), MessageType::Command);
    } catch (const std::exception& e) {
        std::cerr << "Error sending stop response: " << e.what() << std::endl;
    }

    std::cout << "=============================================" << std::endl;
    std::cout << "STOPPING SYSTEM after STOP command" << std::endl;
    std::cout << "=============================================" << std::endl;

    // Longer delay to ensure the response is sent
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Stop the system in a NEW thread to avoid deadlock
    // This is crucial - we cannot stop the system from the message processing thread
    std::thread stopThread([&system]() {
        try {
            system.stop();
        } catch (const std::exception& e) {
            std::cerr << "Exception during system.stop(): " << e.what() << std::endl;
        }
        std::cout << "System stop completed in separate thread" << std::endl;
    });

    // This thread must be detached, as CommandHandler will not wait for its completion
    stopThread.detach();

    // Add a small delay to give the stopThread a chance to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void CommandHandler::handleStatusCommand(const std::string& messageId, const json& commandData, System& system) {
    std::cout << "Executing STATUS command" << std::endl;
    
    json response = {
        {"status", "success"},
        {"command", "status"},
        {"data", {
            {"server_running", system.isRunning()},
            {"client_connected", system.isClientConnected()},
            {"uptime", "unknown"} // to be implemented
        }}
    };
    
    system.sendMessage(messageId, response.dump(), MessageType::Command);
}

void CommandHandler::handlePingCommand(const std::string& messageId, const json& commandData, System& system) {
    std::cout << "Executing PING command" << std::endl;
    
    json response = {
        {"status", "success"},
        {"command", "ping"},
        {"message", "pong"},
        {"timestamp", std::time(nullptr)}
    };
    
    system.sendMessage(messageId, response.dump(), MessageType::Command);
}
