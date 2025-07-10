#include "message/MessageAssembler.hpp"
#include <algorithm>
#include <numeric>

std::optional<std::string> MessageAssembler::addFragment(const MessageFrame& frame)
{
    incompleteMessages[frame.header.messageId].push_back(frame);
    if (isMessageComplete(frame.header.messageId))
    {
        return frame.header.messageId;
    }
    return std::nullopt;
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
    
    // Check for missing fragments
    size_t totalFragments = fragments.size();
    size_t expectedFragments = 0;
    for (const auto& fragment : fragments) {
        if (fragment.header.sequenceNumber >= static_cast<int>(expectedFragments)) {
            expectedFragments = fragment.header.sequenceNumber + 1;
        }
    }
    return totalFragments == expectedFragments;
}

std::string MessageAssembler::getAssembledMessage(const std::string& messageId)
{
    if (!isMessageComplete(messageId))
    {
        return "";
    }

    auto& fragments = incompleteMessages[messageId];

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