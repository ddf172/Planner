#pragma once
#include "json.hpp"
#include <map>
#include <optional>
#include <string>
#include "MessageFrame.hpp"

class MessageAssembler {
    std::string messageId;
    std::map<int, std::string> chunks;
    bool isComplete = false;
    int expectedChunks = -1;

public:
    // Adds a frame and returns true if the message is now complete
    bool addFrame(const MessageFrame& frame);

    // Returns true if the message is complete
    bool complete() const;

    // Assembles and returns the full JSON message if complete
    std::optional<nlohmann::json> assemble() const;
};
