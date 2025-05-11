#pragma once

#include <vector>
#include <functional>
#include <string>

class ISocket{
public:
    enum class MessageType {
        Text,
        Command,
        Data
    };
    
    struct Message {
        MessageType type;
        std::string content;
        std::vector<uint8_t> binaryData;
    };

    virtual int getServerFd() const = 0;
    virtual int getClientFd() const = 0;

    /**
    * @brief Sends Message over the socket.
    * @param buffer Pointer to the Message to be sent.
    * @return true if message was added to the buffer, false if socket is not connected.
    */
    virtual bool sendMessage(const Message& message) = 0;

    virtual bool disconnect() = 0;
    virtual bool isConnected() = 0;

    virtual ~ISocket() = default;


    virtual void setOnConnectedCallback(std::function<void()> callback) = 0;
    virtual void setOnDisconnectedCallback(std::function<void()> callback) = 0;
    virtual void setOnMessageReceivedCallback(std::function<void(const Message&)> callback) = 0;
};