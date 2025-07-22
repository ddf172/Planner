#pragma once

#include "control/IMessageHandler.hpp"

class DataHandler : public IMessageHandler {
public:
    void handle(const std::string& messageId, const std::string& payload, System& system) override;
    MessageType getHandledType() const override;
};