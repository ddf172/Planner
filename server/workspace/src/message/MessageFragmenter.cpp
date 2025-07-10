#include "message/MessageFragmenter.hpp"
#include <random>
#include <sstream>
#include <iomanip>

std::string MessageFragmenter::generateMessageId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << std::hex;
    
    // Generate UUID-like string: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    for (int i = 0; i < 32; ++i) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            ss << '-';
        }
        ss << dis(gen);
    }
    
    return ss.str();
}

std::vector<MessageFrame> MessageFragmenter::fragment(const std::string& payload, MessageType type) {
    std::vector<MessageFrame> fragments;
    
    // If payload is small enough, send as single fragment
    if (payload.size() <= MAX_FRAGMENT_SIZE) {
        MessageFrame frame;
        frame.header.messageId = generateMessageId();
        frame.header.sequenceNumber = 0;
        frame.header.isLast = true;
        frame.header.payloadSize = static_cast<int>(payload.size());
        frame.header.type = type;
        frame.payload = payload;
        
        fragments.push_back(frame);
        return fragments;
    }
    
    // Fragment large payload
    std::string messageId = generateMessageId();
    size_t offset = 0;
    int sequenceNumber = 0;
    
    while (offset < payload.size()) {
        size_t fragmentSize = std::min(MAX_FRAGMENT_SIZE, payload.size() - offset);
        bool isLast = (offset + fragmentSize >= payload.size());
        
        MessageFrame frame;
        frame.header.messageId = messageId;
        frame.header.sequenceNumber = sequenceNumber++;
        frame.header.isLast = isLast;
        frame.header.payloadSize = static_cast<int>(fragmentSize);
        frame.header.type = type;
        frame.payload = payload.substr(offset, fragmentSize);
        
        fragments.push_back(frame);
        offset += fragmentSize;
    }
    
    return fragments;
}