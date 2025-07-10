#pragma once

#include <vector>
#include <string>
#include "message/MessageFrame.hpp"

class MessageFragmenter {
private:
    static constexpr size_t MAX_FRAGMENT_SIZE = 4000; // Leave room for headers
    
    /**
     * @brief Generates a unique message ID
     * @return Unique message ID string
     */
    std::string generateMessageId();
    
public:
    /**
     * @brief Fragments a large message into smaller frames
     * @param payload The message payload to fragment
     * @param type The message type
     * @return Vector of message frames
     */
    std::vector<MessageFrame> fragment(const std::string& payload, MessageType type);
    
    /**
     * @brief Gets the maximum fragment size
     * @return Maximum fragment size in bytes
     */
    static size_t getMaxFragmentSize() { return MAX_FRAGMENT_SIZE; }
};