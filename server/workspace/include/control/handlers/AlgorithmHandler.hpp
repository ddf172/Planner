#pragma once

#include "control/IMessageHandler.hpp"
#include "extern/nlohmann/json.hpp"

using json = nlohmann::json;

class AlgorithmHandler : public IMessageHandler {
public:
    void handle(const std::string& messageId, const std::string& payload, System& system) override;
    MessageType getHandledType() const override;

private:
    void handleList(const std::string& messageId, System& system);
    void handleRun(const std::string& messageId, const json& request, System& system);
    void handleStop(const std::string& messageId, System& system);
    void handleStatus(const std::string& messageId, System& system);
    
    // Callback functions
    void onProgress(float progress, const std::string& status, const json& progressData, 
                   const std::string& messageId, System& system);
    void onCompletion(const json& resultData, const std::string& messageId, System& system);
};
