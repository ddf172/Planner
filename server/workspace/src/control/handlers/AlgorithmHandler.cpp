#include "control/handlers/AlgorithmHandler.hpp"
#include "core/System.hpp"
#include <iostream>

void AlgorithmHandler::handle(const std::string& messageId, const std::string& payload, System& system) {
    std::cout << "AlgorithmHandler: Received message " << messageId << std::endl;
    
    try {
        json algorithmData = json::parse(payload);
        
        if (algorithmData.contains("command")) {
            std::string algorithmCmd = algorithmData["command"];
            
            if (algorithmCmd == "list") {
                handleList(messageId, system);
            } else if (algorithmCmd == "run") {
                handleRun(messageId, algorithmData, system);
            } else if (algorithmCmd == "stop") {
                handleStop(messageId, system);
            } else if (algorithmCmd == "status") {
                handleStatus(messageId, system);
            } else {
                json response = {
                    {"status", "error"},
                    {"message", "Unknown algorithm command: " + algorithmCmd},
                    {"error_code", "UNKNOWN_ALGORITHM_COMMAND"},
                    {"available_commands", {"list", "run", "stop", "status"}}
                };
                system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
            }
        } else {
            json response = {
                {"status", "error"},
                {"message", "No 'command' field found in payload"},
                {"error_code", "MISSING_COMMAND_FIELD"}
            };
            system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "AlgorithmHandler: Error parsing payload: " << e.what() << std::endl;
        json response = {
            {"status", "error"},
            {"message", "Invalid JSON format"},
            {"error_code", "INVALID_JSON"}
        };
        system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
    }
}

MessageType AlgorithmHandler::getHandledType() const {
    return MessageType::Algorithm;
}

void AlgorithmHandler::handleList(const std::string& messageId, System& system) {
    std::cout << "=== ALGORITHM: LIST ===" << std::endl;
    
    auto algorithms = system.getAlgorithmScanner().getAlgorithms();
    
    json response = {
        {"status", "success"},
        {"algorithms", json::array()}
    };
    
    for (const auto& algo : algorithms) {
        json algoJson = {
            {"name", algo.name},
            {"displayName", algo.displayName},
            {"version", algo.version},
            {"description", algo.description},
            {"author", algo.author},
            {"type", algo.type},
            {"supportsProgress", algo.supportsProgress}
        };
        
        if (!algo.parameters.empty()) {
            algoJson["parameters"] = algo.parameters;
        }
        
        response["algorithms"].push_back(algoJson);
    }
    
    std::cout << "Found " << algorithms.size() << " algorithms" << std::endl;
    system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
}

void AlgorithmHandler::handleRun(const std::string& messageId, const json& request, System& system) {
    std::cout << "=== ALGORITHM: RUN ===" << std::endl;
    
    if (!request.contains("name")) {
        json response = {
            {"status", "error"},
            {"message", "Missing 'name' field"},
            {"error_code", "MISSING_NAME"}
        };
        system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
        return;
    }
    
    if (!request.contains("data")) {
        json response = {
            {"status", "error"},
            {"message", "Missing 'data' field"},
            {"error_code", "MISSING_DATA"}
        };
        system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
        return;
    }
    
    // Check if algorithm is already running
    if (system.getAlgorithmRunner().isRunning()) {
        json response = {
            {"status", "error"},
            {"message", "Algorithm is already running"},
            {"error_code", "ALREADY_RUNNING"}
        };
        system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
        return;
    }
    
    std::string algorithmName = request["name"];
    json inputData = request["data"];
    json config = request.value("config", json::object());
    
    // Check if algorithm exists
    if (!system.getAlgorithmScanner().hasAlgorithm(algorithmName)) {
        json response = {
            {"status", "error"},
            {"message", "Algorithm not found: " + algorithmName},
            {"error_code", "ALGORITHM_NOT_FOUND"}
        };
        system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
        return;
    }
    
    // Validate configuration
    auto configErrors = system.getAlgorithmScanner().validateConfiguration(algorithmName, config);
    if (!configErrors.empty()) {
        json response = {
            {"status", "error"},
            {"message", "Configuration validation failed"},
            {"error_code", "INVALID_CONFIG"},
            {"errors", configErrors}
        };
        system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
        return;
    }
    
    std::cout << "Starting algorithm: " << algorithmName << std::endl;
    
    // Set up callbacks
    auto progressCallback = [this, messageId, &system](float progress, const std::string& status, const json& progressData) {
        this->onProgress(progress, status, progressData, messageId, system);
    };
    
    auto completionCallback = [this, messageId, &system](const json& resultData) {
        this->onCompletion(resultData, messageId, system);
    };
    
    // Get algorithm path and start
    std::string algorithmPath = system.getAlgorithmScanner().getAlgorithmPath(algorithmName);
    bool started = system.getAlgorithmRunner().start(algorithmPath, inputData, config, progressCallback, completionCallback);
    
    if (started) {
        json response = {
            {"status", "started"},
            {"algorithm", algorithmName},
            {"message", "Algorithm execution started"}
        };
        system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
    } else {
        json response = {
            {"status", "error"},
            {"message", "Failed to start algorithm"},
            {"error_code", "START_FAILED"}
        };
        system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
    }
}

void AlgorithmHandler::handleStop(const std::string& messageId, System& system) {
    std::cout << "=== ALGORITHM: STOP ===" << std::endl;
    
    if (!system.getAlgorithmRunner().isRunning()) {
        json response = {
            {"status", "error"},
            {"message", "No algorithm running"},
            {"error_code", "NOT_RUNNING"}
        };
        system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
        return;
    }
    
    bool stopped = system.getAlgorithmRunner().isRunning();
    if (stopped) {
        system.getAlgorithmRunner().stop();
    }
    
    json response = {
        {"status", stopped ? "success" : "error"},
        {"message", stopped ? "Algorithm stopped" : "No algorithm running"}
    };
    
    system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
}

void AlgorithmHandler::handleStatus(const std::string& messageId, System& system) {
    std::cout << "=== ALGORITHM: STATUS ===" << std::endl;
    
    auto& runner = system.getAlgorithmRunner();
    
    json algorithmStatus = {
        {"running", runner.isRunning()},
        {"progress", runner.getProgress()},
        {"status", runner.getStatus()}
    };
    
    if (!runner.isRunning()) {
        algorithmStatus["result"] = runner.getResult();
    }
    
    json response = {
        {"status", "success"},
        {"algorithm_status", algorithmStatus}
    };
    
    system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
}

void AlgorithmHandler::onProgress(float progress, const std::string& status, const json& progressData, 
                                 const std::string& messageId, System& system) {
    std::cout << "Algorithm progress: " << progress << ", status: " << status 
              << ", data: " << progressData.dump() << std::endl;
    // Progress updates could be sent as notifications if needed
}

void AlgorithmHandler::onCompletion(const json& resultData, const std::string& messageId, System& system) {
    std::cout << "Algorithm completed with result: " << resultData.dump() << std::endl;
    
    json response = {
        {"status", "completed"},
        {"message", "Algorithm execution completed"},
        {"result", resultData}
    };
    system.sendMessage(messageId, response.dump(), MessageType::Algorithm);
}
