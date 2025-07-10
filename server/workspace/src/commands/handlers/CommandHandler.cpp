#include "commands/handlers/CommandHandler.hpp"
#include "core/System.hpp"
#include "extern/nlohmann/json.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <ctime>

using json = nlohmann::json;

void CommandHandler::handle(const std::string& messageId, const std::string& payload, System& system) {
    std::cout << "CommandHandler: Received message " << messageId << std::endl;
    std::cout << "Payload: " << payload << std::endl;
    
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
                system.sendResponse(messageId, response.dump(), MessageType::Command);
            }
        } else {
            json response = {
                {"status", "error"},
                {"message", "No 'command' field found in payload"},
                {"error_code", "MISSING_COMMAND_FIELD"}
            };
            system.sendResponse(messageId, response.dump(), MessageType::Command);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "CommandHandler: Error parsing payload: " << e.what() << std::endl;
        json response = {
            {"status", "error"},
            {"message", "Invalid JSON format"},
            {"error_code", "INVALID_JSON"}
        };
        system.sendResponse(messageId, response.dump(), MessageType::Command);
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
    
    system.sendResponse(messageId, response.dump(), MessageType::Command);
    
    // Stop the system after sending response
    std::thread([&system]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Give time to send response
        system.stop();
    }).detach();
}

void CommandHandler::handleStatusCommand(const std::string& messageId, const json& commandData, System& system) {
    std::cout << "Executing STATUS command" << std::endl;
    
    json response = {
        {"status", "success"},
        {"command", "status"},
        {"data", {
            {"server_running", system.isRunning()},
            {"client_connected", system.isClientConnected()},
            {"uptime", "unknown"} // Możesz dodać rzeczywisty uptime później
        }}
    };
    
    system.sendResponse(messageId, response.dump(), MessageType::Command);
}

void CommandHandler::handlePingCommand(const std::string& messageId, const json& commandData, System& system) {
    std::cout << "Executing PING command" << std::endl;
    
    json response = {
        {"status", "success"},
        {"command", "ping"},
        {"message", "pong"},
        {"timestamp", std::time(nullptr)}
    };
    
    system.sendResponse(messageId, response.dump(), MessageType::Command);
}
