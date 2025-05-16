#pragma once
#include "MessageFrame.hpp"
#include <vector>
#include <string>
#include "json.hpp"

class MessageFragmenter {
public:
    static std::vector<MessageFrame> fragment(const nlohmann::json& json, const std::string& messageId, int maxPayloadSize);
};
