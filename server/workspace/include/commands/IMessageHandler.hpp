#pragma once

#include <string>
#include "message/MessageFrame.hpp"

// Forward declaration
class System;

class IMessageHandler {
public:
    virtual ~IMessageHandler() = default;
    virtual void handle(const std::string& messageId, const std::string& payload, System& system) = 0;
    virtual MessageType getHandledType() const = 0;
};