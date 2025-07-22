#include "algorithm/AlgorithmRunner.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <random>
#include <cstdlib>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

AlgorithmRunner::AlgorithmRunner() 
    : running(false), stopRequested(false), progress(0.0f), exitCode(-1), timeoutSeconds(300) {
}

AlgorithmRunner::~AlgorithmRunner() {
    stop();
    cleanupTempFiles();
}

bool AlgorithmRunner::start(const std::string& algorithmPath, const json& inputData, const json& config, 
                          ProgressCallback progressCb, CompletionCallback completionCb, int timeoutSeconds) {
    if (running.load()) {
        std::cerr << "Algorithm is already running" << std::endl;
        return false;
    }
    
    this->algorithmPath = algorithmPath;
    this->timeoutSeconds = timeoutSeconds;
    this->progressCallback = progressCb;
    this->completionCallback = completionCb;
    stopRequested.store(false);
    progress.store(0.0f);
    statusMessage = "initializing";
    resultData = json();
    
    // Generate temporary files
    inputFile = generateTempFile("algorithm_input");
    outputFile = generateTempFile("algorithm_output");
    configFile = generateTempFile("algorithm_config");
    progressFile = generateTempFile("algorithm_progress");
    
    try {
        // Write input data
        std::ofstream inputStream(inputFile);
        inputStream << inputData.dump(2);
        inputStream.close();
        
        // Write config data
        std::ofstream configStream(configFile);
        configStream << config.dump(2);
        configStream.close();
        
        // Create empty progress file
        std::ofstream progressStream(progressFile);
        progressStream << "{}";
        progressStream.close();
        
    } catch (const std::exception& e) {
        std::cerr << "Error creating temporary files: " << e.what() << std::endl;
        cleanupTempFiles();
        return false;
    }
    
    running.store(true);
    startTime = std::chrono::steady_clock::now();
    processThread = std::thread(&AlgorithmRunner::runAlgorithmProcess, this);
    
    return true;
}

void AlgorithmRunner::stop() {
    if (!running.load()) {
        return;
    }
    
    stopRequested.store(true);
    
    if (processThread.joinable()) {
        processThread.join();
    }
    
    running.store(false);
}

bool AlgorithmRunner::isRunning() const {
    return running.load();
}

float AlgorithmRunner::getProgress() const {
    return progress.load();
}

std::string AlgorithmRunner::getStatus() const {
    return statusMessage;
}

json AlgorithmRunner::getResult() const {
    return resultData;
}

void AlgorithmRunner::runAlgorithmProcess() {
    statusMessage = "starting";
    
    // Build command
    std::string command = algorithmPath + "/algorithm " + 
                         inputFile + " " + outputFile + " " + 
                         configFile + " " + progressFile;
    
    std::cout << "Running algorithm: " << command << std::endl;
    
    // Start progress monitoring thread
    std::thread progressThread(&AlgorithmRunner::monitorProgress, this);
    
    // Execute algorithm
    exitCode = system(command.c_str());
    
    // Wait for progress thread to finish
    if (progressThread.joinable()) {
        progressThread.join();
    }
    
    if (stopRequested.load()) {
        statusMessage = "stopped";
        progress.store(0.0f);
    } else if (exitCode == 0) {
        statusMessage = "completed";
        progress.store(1.0f);
        
        // Read result
        try {
            std::ifstream outputStream(outputFile);
            if (outputStream.is_open()) {
                outputStream >> resultData;
                outputStream.close();
                
                if (!validateResult(resultData)) {
                    statusMessage = "error";
                    resultData["status"] = "error";
                    resultData["errorMessage"] = "Invalid result format";
                }
            } else {
                statusMessage = "error";
                resultData["status"] = "error";
                resultData["errorMessage"] = "Could not read output file";
            }
        } catch (const std::exception& e) {
            statusMessage = "error";
            resultData["status"] = "error";
            resultData["errorMessage"] = "Error reading result: " + std::string(e.what());
        }
    } else {
        statusMessage = "failed";
        resultData["status"] = "error";
        resultData["errorMessage"] = "Algorithm exited with code " + std::to_string(exitCode);
    }
    
    running.store(false);
    
    // Call completion callback
    if (completionCallback) {
        completionCallback(resultData);
    }
    
    cleanupTempFiles();
}

void AlgorithmRunner::monitorProgress() {
    while (running.load() && !stopRequested.load()) {
        updateProgress();
        
        // Check timeout
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
        
        if (elapsed > timeoutSeconds) {
            std::cout << "Algorithm timeout after " << elapsed << " seconds" << std::endl;
            stopRequested.store(true);
            statusMessage = "timeout";
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Faster polling
    }
}

void AlgorithmRunner::updateProgress() {
    try {
        std::ifstream progressStream(progressFile);
        if (progressStream.is_open()) {
            json progressData;
            progressStream >> progressData;
            progressStream.close();
            
            if (progressData.contains("progress")) {
                float newProgress = progressData["progress"];
                if (newProgress >= 0.0f && newProgress <= 1.0f) {
                    progress.store(newProgress);
                }
            }
            
            if (progressData.contains("status")) {
                statusMessage = progressData["status"];
            }
            
            // Call progress callback
            if (progressCallback) {
                float currentProgress = progress.load();
                progressCallback(currentProgress, statusMessage, progressData);
            }
        }
    } catch (const std::exception& e) {
        // Ignore progress reading errors - algorithm might not support progress
    }
}

void AlgorithmRunner::cleanupTempFiles() {
    auto removeFile = [](const std::string& file) {
        if (!file.empty() && std::filesystem::exists(file)) {
            std::filesystem::remove(file);
        }
    };
    
    removeFile(inputFile);
    removeFile(outputFile);
    removeFile(configFile);
    removeFile(progressFile);
}

std::string AlgorithmRunner::generateTempFile(const std::string& prefix) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    std::string tempDir = "/tmp";
    std::string filename = prefix + "_" + std::to_string(dis(gen)) + ".json";
    
    return tempDir + "/" + filename;
}

bool AlgorithmRunner::validateResult(const json& result) {
    if (!result.is_object()) {
        return false;
    }
    
    if (!result.contains("status")) {
        return false;
    }
    
    std::string status = result["status"];
    if (status != "success" && status != "no_solution" && status != "error") {
        return false;
    }
    
    // If success, must have schedule
    if (status == "success" && !result.contains("schedule")) {
        return false;
    }
    
    return true;
}
