#pragma once

#include "commands/IMessageHandler.hpp"
#include "extern/nlohmann/json.hpp"

using json = nlohmann::json;

class DebugHandler : public IMessageHandler {
public:
    void handle(const std::string& messageId, const std::string& payload, System& system) override;
    MessageType getHandledType() const override;

private:
    void handlePrintPayload(const std::string& messageId, const json& debugData, System& system);
    void handleUptime(const std::string& messageId, const json& debugData, System& system);
    void handleServerInfo(const std::string& messageId, const json& debugData, System& system);
};