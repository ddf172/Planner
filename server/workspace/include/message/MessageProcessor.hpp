#pragma once

#include "message/MessageFrame.hpp"
#include "message/MessageAssembler.hpp"
#include "message/MessageFragmenter.hpp"
#include "commands/HandlerDispatcher.hpp"
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
    
    // Input queue (from ServerSocket)
    std::queue<MessageFrame> inputQueue;
    mutable std::mutex inputMutex;
    std::condition_variable inputCondition;
    
    // Message processing components
    MessageAssembler assembler;
    MessageFragmenter fragmenter;
    HandlerDispatcher dispatcher;
    
    // Reference to system for handlers
    System* system;
    
    // Reference to ServerSocket for direct queue access
    ServerSocket* serverSocket;
    
    // Main processing loop
    void processLoop();
    
    // Internal handlers
    void handleInputMessage(const MessageFrame& frame);
    void handleCompleteMessage(const std::string& messageId, const std::string& payload, MessageType type);

public:
    MessageProcessor(System* sys);
    ~MessageProcessor();
    
    // Control methods
    void start();
    void stop();
    bool isRunning() const;
    
    // Configuration methods
    void setServerSocket(ServerSocket* socket);
    void registerHandler(MessageType type, IMessageHandler* handler);
    
    // Message handling
    void sendMessage(const std::string& messageId, const std::string& payload, MessageType type);
};
