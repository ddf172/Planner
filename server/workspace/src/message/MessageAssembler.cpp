#include "message/MessageAssembler.hpp"
#include <algorithm>
#include <iostream>

bool MessageAssembler::addFragment(const MessageFrame& frame) {
    const std::string& messageId = frame.header.messageId;
    
    // Add fragment to the collection
    incompleteMessages[messageId].push_back(frame);
    
    // Check if this was the last fragment
    if (frame.header.isLast) {
        // Verify we have all fragments
        auto& fragments = incompleteMessages[messageId];
        
        // Sort fragments by sequence number
        std::sort(fragments.begin(), fragments.end(), 
            [](const MessageFrame& a, const MessageFrame& b) {
                return a.header.sequenceNumber < b.header.sequenceNumber;
            });
        
        // Check for missing fragments
        for (size_t i = 0; i < fragments.size(); ++i) {
            if (fragments[i].header.sequenceNumber != static_cast<int>(i)) {
                std::cerr << "Missing fragment " << i << " for message " << messageId << std::endl;
                return false;
            }
        }
        
        return true; // Message is complete
    }
    
    return false; // More fragments needed
}

bool MessageAssembler::isMessageComplete(const std::string& messageId) const {
    auto it = incompleteMessages.find(messageId);
    if (it == incompleteMessages.end()) {
        return false;
    }
    
    const auto& fragments = it->second;
    if (fragments.empty()) {
        return false;
    }
    
    // Check if we have the last fragment
    for (const auto& fragment : fragments) {
        if (fragment.header.isLast) {
            return true;
        }
    }
    
    return false;
}

std::string MessageAssembler::getCompleteMessage(const std::string& messageId) {
    auto it = incompleteMessages.find(messageId);
    if (it == incompleteMessages.end()) {
        return "";
    }
    
    auto& fragments = it->second;
    if (fragments.empty()) {
        return "";
    }
    
    // Sort fragments by sequence number
    std::sort(fragments.begin(), fragments.end(), 
        [](const MessageFrame& a, const MessageFrame& b) {
            return a.header.sequenceNumber < b.header.sequenceNumber;
        });
    
    // Assemble the complete message
    std::string completeMessage;
    for (const auto& fragment : fragments) {
        completeMessage += fragment.payload;
    }
    
    return completeMessage;
}

MessageType MessageAssembler::getMessageType(const std::string& messageId) const {
    auto it = incompleteMessages.find(messageId);
    if (it == incompleteMessages.end() || it->second.empty()) {
        return MessageType::Data; // Default
    }
    
    return it->second[0].header.type;
}

void MessageAssembler::cleanup(const std::string& messageId) {
    incompleteMessages.erase(messageId);
}

size_t MessageAssembler::getIncompleteMessageCount() const {
    return incompleteMessages.size();
}