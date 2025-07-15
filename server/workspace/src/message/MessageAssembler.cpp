#include "message/MessageAssembler.hpp"
#include <algorithm>
#include <numeric>

std::optional<std::string> MessageAssembler::addFragment(const MessageFrame& frame)
{   
    // Add fragment to the incomplete messages map, if entry does not exist, create it
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
    
    // Find the last sequence number and check if we have all fragments
    int lastSequenceNumber = -1;
    bool hasLastFragment = false;
    
    for (int i = fragments.size() - 1; i >= 0; --i) {
        if (fragments[i].header.isLast) {
            lastSequenceNumber = fragments[i].header.sequenceNumber;
            hasLastFragment = true;
            break;
        }
    }
    
    // If we didn't find a last fragment, the message is incomplete
    if (!hasLastFragment) {
        return false;
    }
    
    // Check if we have exactly the expected number of fragments
    int expectedFragmentCount = lastSequenceNumber + 1;
    if (static_cast<int>(fragments.size()) != expectedFragmentCount) {
        return false;
    }

    // Check for missing fragments
    std::vector<bool> sequenceNumbersSeen(expectedFragmentCount, false);
    for (const auto& fragment : fragments) {
        int seqNum = fragment.header.sequenceNumber;
        if (seqNum >= 0 && seqNum <= lastSequenceNumber) {
            sequenceNumbersSeen[seqNum] = true;
        }
    }

    // Check if all sequence numbers from 0 to lastSequenceNumber are present
    for (bool seen : sequenceNumbersSeen) {
        if (!seen) {
            return false; // Missing fragment
        }
    }
    
    return true;
}

std::optional<std::string> MessageAssembler::getAssembledMessage(const std::string& messageId)
{
    if (!isMessageComplete(messageId))
    {
        return std::nullopt;
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

std::optional<MessageType> MessageAssembler::getMessageType(const std::string& messageId) const {
    auto it = incompleteMessages.find(messageId);
    if (it == incompleteMessages.end() || it->second.empty()) {
        return std::nullopt;
    }
    
    return it->second[0].header.type;
}

void MessageAssembler::cleanup(const std::string& messageId) {
    incompleteMessages.erase(messageId);
}

size_t MessageAssembler::getIncompleteMessageCount() const {
    return incompleteMessages.size();
}