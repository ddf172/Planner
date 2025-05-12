// MessageFragmenter.cpp
#include "MessageFragmenter.hpp"

std::vector<MessageFrame> MessageFragmenter::fragment(const nlohmann::json& json, const std::string& messageId, int maxPayloadSize) {
    std::string serialized = json.dump();
    std::vector<MessageFrame> frames;

    int totalSize = serialized.size();
    int offset = 0;
    int sequence = 0;

    while (offset < totalSize) {
        int chunkSize = std::min(maxPayloadSize, totalSize - offset);
        std::string chunk = serialized.substr(offset, chunkSize);

        MessageHeader header {
            .messageId = messageId,
            .sequenceNumber = sequence,
            .isLast = (offset + chunkSize >= totalSize),
            .payloadSize = chunkSize
        };

        frames.push_back({header, chunk});

        offset += chunkSize;
        sequence++;
    }

    return frames;
}
