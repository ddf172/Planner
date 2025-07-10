#pragma once

#include <map>
#include <vector>
#include <string>
#include "message/MessageFrame.hpp"

class MessageAssembler {
private:
    std::map<std::string, std::vector<MessageFrame>> incompleteMessages;
    
public:
    /**
     * @brief Adds a fragment to the assembler
     * @param frame The message frame fragment
     * @return true if message is now complete, false if more fragments needed
     */
    bool addFragment(const MessageFrame& frame);
    
    /**
     * @brief Checks if message is complete
     * @param messageId The message ID to check
     * @return true if message is complete
     */
    bool isMessageComplete(const std::string& messageId) const;
    
    /**
     * @brief Gets the complete assembled message
     * @param messageId The message ID
     * @return The complete assembled payload
     */
    std::string getCompleteMessage(const std::string& messageId);
    
    /**
     * @brief Gets the message type of completed message
     * @param messageId The message ID
     * @return The message type
     */
    MessageType getMessageType(const std::string& messageId) const;
    
    /**
     * @brief Cleans up message fragments after processing
     * @param messageId The message ID to cleanup
     */
    void cleanup(const std::string& messageId);
    
    /**
     * @brief Gets number of incomplete messages
     * @return Number of incomplete messages
     */
    size_t getIncompleteMessageCount() const;
};