#pragma once

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

    virtual int getFd() const = 0;

    /**
    * @brief Send data to the socket without serialization.
    *
    * @param buffer Pointer to the data to be sent.
    * @param length Length of the data to be sent.
    * @return Number of bytes sent, or -1 on error.
    */
    virtual int send(const void* buffer, size_t length) = 0;
    
    /**
    * @brief Receive data from the socket without deserialization.
    *
    * @param buffer Pointer to the buffer where received data will be stored.
    * @param length Maximum length of the buffer.
    * @return Number of bytes received, or -1 on error.
    */
    virtual int recv(void* buffer, size_t length) = 0;

    virtual bool disconnect() = 0;
    virtual bool isConnected() = 0;

    virtual ~ISocket() = default;


    virtual void setOnConnectedCallback(std::function<void()> callback) = 0;
    virtual void setOnDisconnectedCallback(std::function<void()> callback) = 0;
    virtual void setOnMessageReceivedCallback(std::function<void(const Message&)> callback) = 0;
}