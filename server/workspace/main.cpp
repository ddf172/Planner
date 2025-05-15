#include "include/network/ServerSocket.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>

volatile sig_atomic_t running = 1;

void signal_handler(int signal) {
    running = 0;
}

int main() {
    // Handling (Ctrl+C)
    signal(SIGINT, signal_handler);
    
    try {
        std::cout << "Starting server on port 8080..." << std::endl;
        ServerSocket server(8080);
        
        server.setOnConnectedCallback([]() {
            std::cout << "Client connected!" << std::endl;
        });
        
        server.setOnDisconnectedCallback([]() {
            std::cout << "Client disconnected!" << std::endl;
        });
        
        server.setOnMessageReceivedCallback([&server](const ISocket::Message& message) {
            std::cout << "Recieved: " << message.content << std::endl;
            
            // Resending the message back to the client
            ISocket::Message response;
            response.type = ISocket::MessageType::Text;
            response.content = "Server recieved: " + message.content;
            server.sendMessage(response);
        });
        
        std::cout << "Waiting for client connection..." << std::endl;
        
        while (running) {
            if (!server.isConnected()) {
                server.accept();
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << "Closing server..." << std::endl;
        if (server.isConnected()) {
            server.disconnect();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}