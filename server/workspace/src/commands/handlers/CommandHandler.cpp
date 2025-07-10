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
    
    // Wysyłamy odpowiedź
    try {
        system.sendResponse(messageId, response.dump(), MessageType::Command);
    } catch (const std::exception& e) {
        std::cerr << "Error sending stop response: " << e.what() << std::endl;
    }
    
    // Dodajemy bardziej widoczne logi dla debugowania
    std::cout << "=============================================" << std::endl;
    std::cout << "STOPPING SYSTEM after STOP command" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // Dłuższe opóźnienie, aby upewnić się, że odpowiedź dotrze
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Zatrzymujemy system w NOWYM wątku, aby uniknąć deadlocka
    // To jest kluczowe - nie możemy zatrzymać systemu z wątku przetwarzania wiadomości
    std::thread stopThread([&system]() {
        std::cout << "Executing system.stop() in separate thread" << std::endl;
        try {
            system.stop();
        } catch (const std::exception& e) {
            std::cerr << "Exception during system.stop(): " << e.what() << std::endl;
        }
        std::cout << "System stop completed in separate thread" << std::endl;
    });
    
    // Ten wątek musi być detached, ponieważ CommandHandler nie będzie czekał na jego zakończenie
    // Nie możemy użyć join(), ponieważ cały system się zamyka
    stopThread.detach();
    
    // Dodajemy małe opóźnienie, aby dać wątkowi stopThread szansę na rozpoczęcie działania
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
