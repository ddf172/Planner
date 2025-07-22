#pragma once

#include <map>
#include <memory>
#include "control/IMessageHandler.hpp"
#include "message/MessageFrame.hpp"

// Forward declaration
class System;

class HandlerDispatcher {
private:
    std::map<MessageType, IMessageHandler*> handlers;
    
public:
    /**
     * @brief Registers a handler for a specific message type
     * @param type The message type to handle
     * @param handler Pointer to the handler (not owned by dispatcher)
     */
    void registerHandler(MessageType type, IMessageHandler* handler);
    
    /**
     * @brief Dispatches a message to the appropriate handler
     * @param messageId The message ID
     * @param payload The message payload
     * @param type The message type
     * @param system Reference to the system
     * @return true if handler was found and executed, false otherwise
     */
    bool dispatch(const std::string& messageId, const std::string& payload, MessageType type, System& system);
    
    /**
     * @brief Checks if a handler is registered for a message type
     * @param type The message type
     * @return true if handler exists
     */
    bool hasHandler(MessageType type) const;
    
    /**
     * @brief Gets the number of registered handlers
     * @return Number of handlers
     */
    size_t getHandlerCount() const;
};