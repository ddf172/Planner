#pragma once

#include "message/MessageFrame.hpp"
#include "message/MessageAssembler.hpp"
#include "message/MessageFragmenter.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <functional>
#include <optional>

// Forward declarations
class System;
class ServerSocket;

class MessageProcessor {
private:
    // Processing thread
    std::thread processingThread;
    std::atomic<bool> running;
    
    // Message processing components
    MessageAssembler assembler;
    MessageFragmenter fragmenter;

    // Callbacks
    void onClientConnected();
    void onClientDisconnected();
    
    // Reference to system for handlers
    System* system;
    
    // Owned ServerSocket for network communication
    std::unique_ptr<ServerSocket> serverSocket;
    
    // Main processing loop
    void processLoop();
    
    // Internal handlers
    void handleInputMessage(const MessageFrame& frame);
    void handleCompleteMessage(const std::string& messageId, const std::string& payload, MessageType type);

public:
    MessageProcessor(System* sys, int port);
    ~MessageProcessor();
    
    // Control methods
    void start();
    void stop();
    bool isRunning() const;
    
    // Network methods
    bool acceptConnection();
    bool isClientConnected() const;
    void setOnConnectedCallback(std::function<void()> callback);
    void setOnDisconnectedCallback(std::function<void()> callback);
    
    // Configuration methods - REMOVED setServerSocket
    
    // Message handling
    void sendMessage(const std::string& messageId, const std::string& payload, MessageType type);
};
