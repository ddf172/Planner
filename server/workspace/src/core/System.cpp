#include "core/System.hpp"
#include <iostream>

System::System(int port) : serverSocket(port), running(false) {
    // Set up ServerSocket callbacks
    serverSocket.setOnMessageReceivedCallback([this](const MessageFrame& frame) {
        this->onMessageReceived(frame);
    });
    
    serverSocket.setOnConnectedCallback([this]() {
        this->onClientConnected();
    });
    
    serverSocket.setOnDisconnectedCallback([this]() {
        this->onClientDisconnected();
    });
    
    std::cout << "System initialized on port " << port << std::endl;
}

System::~System() {
    stop();
}

void System::start() {
    if (running.load()) {
        std::cout << "System is already running" << std::endl;
        return;
    }
    
    running.store(true);
    mainThread = std::thread(&System::processLoop, this);
    
    std::cout << "System started" << std::endl;
}

void System::stop() {
    if (!running.load()) {
        return;
    }
    
    running.store(false);
    
    if (mainThread.joinable()) {
        mainThread.join();
    }
    
    std::cout << "System stopped" << std::endl;
}

void System::registerHandler(std::unique_ptr<IMessageHandler> handler) {
    if (!handler) {
        std::cerr << "Cannot register null handler" << std::endl;
        return;
    }
    
    MessageType type = handler->getHandledType();
    dispatcher.registerHandler(type, handler.get());
    handlers.push_back(std::move(handler));
    
    std::cout << "Handler registered for message type: " << static_cast<int>(type) << std::endl;
}

bool System::sendMessage(const std::string& payload, MessageType type) {
    if (!isClientConnected()) {
        std::cerr << "Cannot send message: no client connected" << std::endl;
        return false;
    }
    
    auto fragments = fragmenter.fragment(payload, type);
    
    for (const auto& fragment : fragments) {
        if (!serverSocket.sendMessage(fragment)) {
            std::cerr << "Failed to send message fragment" << std::endl;
            return false;
        }
    }
    
    return true;
}

bool System::sendResponse(const std::string& messageId, const std::string& payload, MessageType type) {
    // For now, just send as regular message
    // In future, could include messageId in response for correlation
    return sendMessage(payload, type);
}

bool System::isRunning() const {
    return running.load();
}

bool System::isClientConnected() const {
    return serverSocket.isConnected();
}

void System::printStats() const {
    std::cout << "=== System Statistics ===" << std::endl;
    std::cout << "Running: " << (running.load() ? "Yes" : "No") << std::endl;
    std::cout << "Client connected: " << (isClientConnected() ? "Yes" : "No") << std::endl;
    std::cout << "Registered handlers: " << dispatcher.getHandlerCount() << std::endl;
    std::cout << "Incomplete messages: " << assembler.getIncompleteMessageCount() << std::endl;
    std::cout << "=========================" << std::endl;
}

void System::onMessageReceived(const MessageFrame& frame) {
    std::cout << "Received fragment: " << frame.header.messageId 
              << " [" << frame.header.sequenceNumber << "]" << std::endl;
    
    bool isComplete = assembler.addFragment(frame);
    
    if (isComplete) {
        std::string messageId = frame.header.messageId;
        std::string payload = assembler.getCompleteMessage(messageId);
        MessageType type = assembler.getMessageType(messageId);
        
        onMessageAssembled(messageId, payload, type);
        assembler.cleanup(messageId);
    }
}

void System::onMessageAssembled(const std::string& messageId, const std::string& payload, MessageType type) {
    std::cout << "Message assembled: " << messageId << " (type: " << static_cast<int>(type) << ")" << std::endl;
    
    if (!dispatcher.dispatch(messageId, payload, type, *this)) {
        std::cerr << "Failed to dispatch message: " << messageId << std::endl;
    }
}

void System::onClientConnected() {
    std::cout << "Client connected" << std::endl;
}

void System::onClientDisconnected() {
    std::cout << "Client disconnected" << std::endl;
}

void System::processLoop() {
    std::cout << "Starting main processing loop" << std::endl;
    
    while (running.load()) {
        // Try to accept new connection if none exists
        if (!isClientConnected()) {
            serverSocket.accept();
        }
        
        // Sleep to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Main processing loop ended" << std::endl;
}