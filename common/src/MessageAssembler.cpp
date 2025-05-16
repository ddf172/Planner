// MessageAssembler.cpp
#include "MessageAssembler.hpp"

bool MessageAssembler::addFrame(const MessageFrame& frame) {
    if (messageId.empty()) {
        messageId = frame.header.messageId;
    }

    if (frame.header.messageId != messageId) return false;

    chunks[frame.header.sequenceNumber] = frame.payload;

    if (frame.header.isLast) {
        isComplete = true;
    }

    return true;
}

bool MessageAssembler::complete() const {
    return isComplete;
}

std::optional<nlohmann::json> MessageAssembler::assemble() const {
    if (!isComplete) return std::nullopt;

    std::string fullPayload;
    for (const auto& [_, chunk] : chunks) {
        fullPayload += chunk;
    }

    try {
        return nlohmann::json::parse(fullPayload);
    } catch (...) {
        return std::nullopt;
    }
}
