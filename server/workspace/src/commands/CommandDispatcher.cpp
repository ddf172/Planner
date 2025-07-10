#include "commands/CommandDispatcher.hpp"
#include "core/System.hpp"
#include <iostream>

void CommandDispatcher::registerHandler(MessageType type, IMessageHandler* handler) {
    if (handler == nullptr) {
        std::cerr << "Cannot register null handler" << std::endl;
        return;
    }
    
    handlers[type] = handler;
}

bool CommandDispatcher::dispatch(const std::string& messageId, const std::string& payload, MessageType type, System& system) {
    auto it = handlers.find(type);
    if (it == handlers.end()) {
        std::cerr << "No handler registered for message type: " << static_cast<int>(type) << std::endl;
        return false;
    }
    
    try {
        it->second->handle(messageId, payload, system);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error handling message " << messageId << ": " << e.what() << std::endl;
        return false;
    }
}

bool CommandDispatcher::hasHandler(MessageType type) const {
    return handlers.find(type) != handlers.end();
}

size_t CommandDispatcher::getHandlerCount() const {
    return handlers.size();
}