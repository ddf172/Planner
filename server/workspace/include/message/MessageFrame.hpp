#pragma once

#include <string>
#include <vector>
#include "extern/nlohmann/json.hpp"

using json = nlohmann::json;

enum class MessageType {
    Data,
    Command,
    Debug,
    // Add more types as needed
};

NLOHMANN_JSON_SERIALIZE_ENUM(MessageType, {
    {MessageType::Data, "Data"},
    {MessageType::Command, "Command"},
    {MessageType::Debug, "Debug"},
})

struct MessageHeader {
    std::string messageId;     // UUID or random message identifier
    int sequenceNumber;        // Fragment number
    bool isLast;               // Is this the last fragment
    int payloadSize;           // Payload size
    MessageType type;          // Type of the message

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(MessageHeader, messageId, sequenceNumber, isLast, payloadSize, type)
};

struct MessageFrame {
    MessageHeader header;
    std::string payload;       // Fragment of serialized JSON

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(MessageFrame, header, payload)
};
