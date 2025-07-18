#include "core/System.hpp"
#include <iostream>
#include <chrono>

System::System(int port) : serverSocket(port), messageProcessor(this), running(false) {
    // Set up ServerSocket callbacks for connection events
    serverSocket.setOnConnectedCallback([this]() {
        try {
            this->onClientConnected();
        } catch (const std::exception& e) {
            std::cerr << "Exception in onClientConnected callback: " << e.what() << std::endl;
        }
    });
    
    serverSocket.setOnDisconnectedCallback([this]() {
        try {
            this->onClientDisconnected();
        } catch (const std::exception& e) {
            std::cerr << "Exception in onClientDisconnected callback: " << e.what() << std::endl;
        }
    });
    
    // Give MessageProcessor access to ServerSocket for direct queue access
    messageProcessor.setServerSocket(&serverSocket);
    
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
    
    // Start message processor
    messageProcessor.start();
    
    std::cout << "System started" << std::endl;
}

void System::stop() {
    if (!running.load()) {
        std::cout << "System is already stopped" << std::endl;
        return;
    }
    
    std::cout << "System stopping..." << std::endl;
    
    running.store(false);
    
    try {
        if (serverSocket.isConnected()) {
            std::cout << "Disconnecting client during system shutdown" << std::endl;
            serverSocket.disconnect();
        }
        
        std::cout << "Setting MessageProcessor serverSocket to nullptr" << std::endl;
        messageProcessor.setServerSocket(nullptr);
        
        std::cout << "Stopping MessageProcessor" << std::endl;
        messageProcessor.stop();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during system stop: " << e.what() << std::endl;
    }
    
    std::cout << "System stopped" << std::endl;
}

bool System::acceptConnection() {
    return serverSocket.accept();
}

void System::registerHandler(std::unique_ptr<IMessageHandler> handler) {
    if (!handler) {
        std::cerr << "Cannot register null handler" << std::endl;
        return;
    }
    
    // Register handler with dispatcher, assign type to handler
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
    
    // Generate unique message ID for outgoing message
    static int counter = 0;
    std::string messageId = "sys-msg-" + std::to_string(++counter);
    
    messageProcessor.sendMessage(messageId, payload, type);
    return true;
}

bool System::sendMessage(const std::string& messageId, const std::string& payload, MessageType type) {
    if (!isClientConnected()) {
        std::cerr << "Cannot send message: no client connected" << std::endl;
        return false;
    }
    
    // Use provided messageId for response correlation
    messageProcessor.sendMessage(messageId, payload, type);
    return true;
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
    std::cout << "Message processor running: " << (messageProcessor.isRunning() ? "Yes" : "No") << std::endl;
    std::cout << "Handlers count: " << handlers.size() << std::endl;
    std::cout << "=========================" << std::endl;
}

void System::onClientConnected() {
    std::cout << "Client connected" << std::endl;
}

void System::onClientDisconnected() {
    std::cout << "Client disconnected" << std::endl;
    
    if (!running.load()) {
        std::cout << "System is already shutting down - ignoring disconnect event" << std::endl;
        return;
    }
    
}

void System::handleCompleteMessage(const std::string& messageId, const std::string& payload, MessageType type) {
    dispatcher.dispatch(messageId, payload, type, *this);
}