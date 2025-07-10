#pragma once

#include "commands/IMessageHandler.hpp"
#include "extern/nlohmann/json.hpp"

using json = nlohmann::json;

class CommandHandler : public IMessageHandler {
public:
    void handle(const std::string& messageId, const std::string& payload, System& system) override;
    MessageType getHandledType() const override;

private:
    void handleStopCommand(const std::string& messageId, const json& commandData, System& system);
    void handleStatusCommand(const std::string& messageId, const json& commandData, System& system);
    void handlePingCommand(const std::string& messageId, const json& commandData, System& system);
};
