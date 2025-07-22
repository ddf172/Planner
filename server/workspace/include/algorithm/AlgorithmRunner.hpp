#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include "extern/nlohmann/json.hpp"

using json = nlohmann::json;

class AlgorithmRunner {
public:
    // Callback types
    using ProgressCallback = std::function<void(float progress, const std::string& status, const json& data)>;
    using CompletionCallback = std::function<void(const json& result)>;
    
    AlgorithmRunner();
    ~AlgorithmRunner();
    
    bool start(const std::string& algorithmPath, const json& inputData, const json& config, 
               ProgressCallback progressCb = nullptr, CompletionCallback completionCb = nullptr, 
               int timeoutSeconds = 300);
    void stop();
    bool isRunning() const;
    float getProgress() const;
    std::string getStatus() const;
    json getResult() const;
    
private:
    std::string algorithmPath;
    std::string inputFile;
    std::string outputFile;
    std::string configFile;
    std::string progressFile;
    std::thread processThread;
    std::atomic<bool> running;
    std::atomic<bool> stopRequested;
    std::atomic<float> progress;
    json resultData;
    std::string statusMessage;
    int exitCode;
    int timeoutSeconds;
    std::chrono::steady_clock::time_point startTime;
    
    // Callbacks
    ProgressCallback progressCallback;
    CompletionCallback completionCallback;
    
    void runAlgorithmProcess();
    void monitorProgress();
    void cleanupTempFiles();
    std::string generateTempFile(const std::string& prefix);
    bool validateResult(const json& result);
    void updateProgress();
};
