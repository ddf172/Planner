#include "control/handlers/DebugHandler.hpp"
#include "core/System.hpp"
#include "extern/nlohmann/json.hpp"
#include <iostream>
#include <ctime>
#include <chrono>

using json = nlohmann::json;

void DebugHandler::handle(const std::string& messageId, const std::string& payload, System& system) {
    std::cout << "DebugHandler: Received debug message " << messageId << std::endl;
    
    try {
        // Try to parse as JSON
        json debugData = json::parse(payload);
        
        if (debugData.contains("command")) {
            std::string debugCmd = debugData["command"];
            
            if (debugCmd == "print_payload") {
                handlePrintPayload(messageId, debugData, system);
            } else if (debugCmd == "uptime") {
                handleUptime(messageId, debugData, system);
            } else if (debugCmd == "server_info") {
                handleServerInfo(messageId, debugData, system);
            } else {
                // Unknown debug command
                json response = {
                    {"status", "error"},
                    {"message", "Unknown debug command: " + debugCmd},
                    {"error_code", "UNKNOWN_DEBUG_COMMAND"},
                    {"available_commands", {"print_payload", "uptime", "server_info"}}
                };
                system.sendMessage(messageId, response.dump(), MessageType::Debug);
            }
        } else {
            json response = {
                {"status", "error"},
                {"message", "No 'command' field found in payload"},
                {"error_code", "MISSING_COMMAND_FIELD"}
            };
            system.sendMessage(messageId, response.dump(), MessageType::Debug);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "DebugHandler: Error parsing payload: " << e.what() << std::endl;
        json response = {
            {"status", "error"},
            {"message", "Invalid JSON format"},
            {"error_code", "INVALID_JSON"}
        };
        system.sendMessage(messageId, response.dump(), MessageType::Debug);
    }
}

void DebugHandler::handlePrintPayload(const std::string& messageId, const json& debugData, System& system) {
    std::cout << "=== DEBUG: PRINT PAYLOAD ===" << std::endl;
    std::cout << "Message ID: " << messageId << std::endl;
    std::cout << "Full payload: " << debugData.dump(4) << std::endl;
    std::cout << "===========================" << std::endl;
    
    json response = {
        {"status", "success"},
        {"command", "print_payload"},
        {"message", "Payload printed to server console"},
        {"timestamp", std::time(nullptr)}
    };
    
    system.sendMessage(messageId, response.dump(), MessageType::Debug);
}

void DebugHandler::handleUptime(const std::string& messageId, const json& debugData, System& system) {
    std::cout << "=== DEBUG: SERVER UPTIME ===" << std::endl;
    std::cout << "Server uptime: [Not implemented yet]" << std::endl;
    std::cout << "Current time: " << std::time(nullptr) << std::endl;
    std::cout << "============================" << std::endl;
    
    json response = {
        {"status", "success"},
        {"command", "uptime"},
        {"message", "Uptime info printed to server console"},
        {"current_timestamp", std::time(nullptr)}, // to be implemented
        {"uptime_seconds", "not_implemented"}
    };
    
    system.sendMessage(messageId, response.dump(), MessageType::Debug);
}

void DebugHandler::handleServerInfo(const std::string& messageId, const json& debugData, System& system) {
    std::cout << "=== DEBUG: SERVER INFO ===" << std::endl;
    std::cout << "Server running: " << (system.isRunning() ? "YES" : "NO") << std::endl;
    std::cout << "Client connected: " << (system.isClientConnected() ? "YES" : "NO") << std::endl;
    std::cout << "Current timestamp: " << std::time(nullptr) << std::endl;
    std::cout << "==========================" << std::endl;
    
    json response = {
        {"status", "success"},
        {"command", "server_info"},
        {"data", {
            {"server_running", system.isRunning()},
            {"client_connected", system.isClientConnected()},
            {"timestamp", std::time(nullptr)}
        }}
    };
    
    system.sendMessage(messageId, response.dump(), MessageType::Debug);
}

MessageType DebugHandler::getHandledType() const {
    return MessageType::Debug;
}