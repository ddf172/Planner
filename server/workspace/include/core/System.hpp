#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include "network/ServerSocket.hpp"
#include "message/MessageAssembler.hpp"
#include "message/MessageFragmenter.hpp"
#include "commands/CommandDispatcher.hpp"
#include "commands/IMessageHandler.hpp"

class System {
private:
    ServerSocket serverSocket;
    MessageAssembler assembler;
    MessageFragmenter fragmenter;
    CommandDispatcher dispatcher;
    
    std::vector<std::unique_ptr<IMessageHandler>> handlers;
    std::atomic<bool> running;
    std::thread mainThread;
    
    // Internal callback methods
    void onMessageReceived(const MessageFrame& frame);
    void onMessageAssembled(const std::string& messageId, const std::string& payload, MessageType type);
    void onClientConnected();
    void onClientDisconnected();
    
    // Main processing loop
    void processLoop();
    
public:
    System(int port);
    ~System();
    
    /**
     * @brief Starts the system
     */
    void start();
    
    /**
     * @brief Stops the system
     */
    void stop();
    
    /**
     * @brief Registers a message handler
     * @param handler Unique pointer to handler
     */
    void registerHandler(std::unique_ptr<IMessageHandler> handler);
    
    /**
     * @brief Sends a message to the client
     * @param payload Message payload
     * @param type Message type
     * @return true if message was queued for sending
     */
    bool sendMessage(const std::string& payload, MessageType type);
    
    /**
     * @brief Sends a response message to the client
     * @param messageId Original message ID (for correlation)
     * @param payload Response payload
     * @param type Message type
     * @return true if message was queued for sending
     */
    bool sendResponse(const std::string& messageId, const std::string& payload, MessageType type);
    
    /**
     * @brief Checks if system is running
     * @return true if running
     */
    bool isRunning() const;
    
    /**
     * @brief Checks if client is connected
     * @return true if client is connected
     */
    bool isClientConnected() const;
    
    /**
     * @brief Gets system statistics
     */
    void printStats() const;
};