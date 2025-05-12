#pragma once
#include "MessageFrame.hpp"
#include <map>
#include <optional>

class MessageAssembler {
    std::string messageId;
    std::map<int, std::string> chunks;
    bool isComplete = false;

public:
    bool addFrame(const MessageFrame& frame);
    bool complete() const;
    std::optional<nlohmann::json> assemble() const;
};
