#pragma once

#include <string>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

struct MessageHeader {
    std::string messageId;     // UUID albo losowy identyfikator wiadomości
    int sequenceNumber;        // Numer fragmentu
    bool isLast;               // Czy to ostatni fragment
    int payloadSize;           // Rozmiar payloadu

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(MessageHeader, messageId, sequenceNumber, isLast, payloadSize)
};

struct MessageFrame {
    MessageHeader header;
    std::string payload;       // Fragment zserializowanego JSON-a

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(MessageFrame, header, payload)
};
