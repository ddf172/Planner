#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include "network/ServerSocket.hpp"
#include "message/MessageProcessor.hpp"
#include "control/IMessageHandler.hpp"
#include "control/HandlerDispatcher.hpp"
#include "algorithm/AlgorithmScanner.hpp"
#include "algorithm/AlgorithmRunner.hpp"

class System {
private:
    ServerSocket serverSocket;
    MessageProcessor messageProcessor;
    HandlerDispatcher dispatcher;
    AlgorithmScanner algorithmScanner;
    AlgorithmRunner algorithmRunner;
    
    std::vector<std::unique_ptr<IMessageHandler>> handlers;
    std::atomic<bool> running;
    
    // Internal callback methods
    void onClientConnected();
    void onClientDisconnected();
    
public:
    System(int port);
    ~System();
    
    /**
     * @brief Starts the system
     */
    void start();
    
    /**
     * @brief Try to accept a client connection
     * @return true if connection accepted
     */
    bool acceptConnection();
    
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
     * @brief Sends a message to the client with correlation ID
     * @param messageId Message ID for correlation (e.g., response to original message)
     * @param payload Message payload
     * @param type Message type
     * @return true if message was queued for sending
     */
    bool sendMessage(const std::string& messageId, const std::string& payload, MessageType type);
    
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
    
    /**
     * @brief Handles a complete message from MessageProcessor
     * @param messageId Message ID
     * @param payload Message payload
     * @param type Message type
     */
    void handleCompleteMessage(const std::string& messageId, const std::string& payload, MessageType type);
    
    // Component access methods
    /**
     * @brief Gets the algorithm scanner
     * @return Reference to algorithm scanner
     */
    AlgorithmScanner& getAlgorithmScanner();
    
    /**
     * @brief Gets the algorithm runner
     * @return Reference to algorithm runner
     */
    AlgorithmRunner& getAlgorithmRunner();
};