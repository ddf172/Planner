// MessageAssembler.cpp
#include "MessageAssembler.hpp"

bool MessageAssembler::addFrame(const MessageFrame& frame) {
    if (messageId.empty()) {
        messageId = frame.header.messageId;
    }

    if (frame.header.messageId != messageId) return false;

    chunks[frame.header.sequenceNumber] = frame.payload;

    if (frame.header.isLast) {
        expectedChunks = frame.header.sequenceNumber + 1;
    }

    if (expectedChunks != -1 && static_cast<int>(chunks.size()) == expectedChunks) {
        isComplete = true;
    }

    return isComplete;
}

bool MessageAssembler::complete() const {
    return isComplete;
}

std::optional<nlohmann::json> MessageAssembler::assemble() const {
    if (!isComplete) return std::nullopt;

    std::string fullPayload;
    for (int i = 0; i < expectedChunks; ++i) {
        auto it = chunks.find(i);
        if (it == chunks.end()) return std::nullopt;
        fullPayload += it->second;
    }

    try {
        return nlohmann::json::parse(fullPayload);
    } catch (...) {
        return std::nullopt;
    }
}
