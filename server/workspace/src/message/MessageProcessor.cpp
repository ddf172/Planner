#include "message/MessageProcessor.hpp"
#include "core/System.hpp"
#include "network/ServerSocket.hpp"
#include <iostream>
#include <chrono>

MessageProcessor::MessageProcessor(System* sys) 
    : running(false), system(sys), serverSocket(nullptr) {
    std::cout << "MessageProcessor initialized" << std::endl;
}

MessageProcessor::~MessageProcessor() {
    stop();
}

void MessageProcessor::start() {
    if (running.load()) {
        std::cout << "MessageProcessor is already running" << std::endl;
        return;
    }
    
    running.store(true);
    processingThread = std::thread(&MessageProcessor::processLoop, this);
    std::cout << "MessageProcessor started" << std::endl;
}

void MessageProcessor::stop() {
    if (!running.load()) {
        return;
    }
    
    std::cout << "MessageProcessor stopping..." << std::endl;
    
    running.store(false);
    
    // If ServerSocket is set, notify ReceiveCondition to wake up any waiting threads
    // and allow message processing to finish
    if (serverSocket) {
        serverSocket->getReceiveCondition().notify_all();
    }
    
    if (processingThread.joinable()) {
        // Avoid deadlock by checking if the current thread is the processing thread
        // If so, detach it to prevent joining from itself
        if (std::this_thread::get_id() == processingThread.get_id()) {
            std::cout << "WARNING: Trying to join processing thread from itself - skipping join" << std::endl;
            processingThread.detach();
        }
        // Otherwise, join the thread to ensure it finishes
        else {
            std::cout << "Joining processing thread..." << std::endl;
            try {
                processingThread.join();
            } catch (const std::exception& e) {
                std::cerr << "Error joining processing thread: " << e.what() << std::endl;
                if (processingThread.joinable()) {
                    processingThread.detach();
                }
            }
        }
    }
    
    serverSocket = nullptr;
    
    std::cout << "MessageProcessor stopped" << std::endl;
}

bool MessageProcessor::isRunning() const {
    return running.load();
}

void MessageProcessor::sendMessage(const std::string& messageId, const std::string& payload, MessageType type) {
    // Fragment the message if needed
    std::vector<MessageFrame> fragments = fragmenter.fragment(payload, type);
    
    // Update message IDs to match the provided messageId
    for (auto& fragment : fragments) {
        fragment.header.messageId = messageId;
    }
    
    // Send directly to ServerSocket
    if (serverSocket) {
        for (const auto& fragment : fragments) {
            serverSocket->sendMessage(fragment);
        }
    }
}

void MessageProcessor::setServerSocket(ServerSocket* socket) {
    serverSocket = socket;
}

void MessageProcessor::processLoop()
{
    std::cout << "MessageProcessor: processLoop started" << std::endl;
    
    while (running)
    {
        if (!running) {
            std::cout << "MessageProcessor: processLoop stopping (running=false)" << std::endl;
            break;
        }
        
        std::vector<MessageFrame> localQueue;
        
        // Handle case when serverSocket is not set
        if (!serverSocket) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (!running) {
                std::cout << "MessageProcessor: processLoop stopping (no serverSocket)" << std::endl;
                return;
            }
            continue;
        }
        
        // Wait for messages from ServerSocket and transfer them to local queue
        try {
            std::unique_lock<std::mutex> serverLock(serverSocket->getReceiveMutex());
            
            if (!running) {
                std::cout << "MessageProcessor: processLoop stopping (before wait)" << std::endl;
                return;
            }

            // Wait for messages or shutdown signal
            serverSocket->getReceiveCondition().wait_for(serverLock, 
                std::chrono::milliseconds(100), [this] {
                if (!running || !serverSocket) return true;
                
                // Wait until the server's receive queue is not empty OR the processor is stopped.
                return !serverSocket->getReceiveQueue().empty();
            });

            // If the loop was woken up to stop, exit gracefully.
            if (!running || !serverSocket)
            {
                std::cout << "MessageProcessor: processLoop stopping (after wait)" << std::endl;
                return;
            }
            
            while (!serverSocket->getReceiveQueue().empty() && running)
            {
                localQueue.push_back(serverSocket->getReceiveQueue().front());
                serverSocket->getReceiveQueue().pop();
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Exception in processLoop: " << e.what() << std::endl;
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (!running) {
                return;
            }
            continue;
        }

        // Process messages outside of any locks
        if (!localQueue.empty())
        {
            for (const auto& frame : localQueue)
            {
                // Re-assemble and dispatch the message.
                auto messageIdOpt = assembler.addFragment(frame);
                if (messageIdOpt)
                {
                    // Message is complete, process it
                    std::string messageId = messageIdOpt.value();
                    
                    auto payloadOpt = assembler.getAssembledMessage(messageId);
                    auto typeOpt = assembler.getMessageType(messageId);
                    
                    if (payloadOpt && typeOpt) {
                        // Dispatch the complete message
                        handleCompleteMessage(messageId, payloadOpt.value(), typeOpt.value());
                        
                        // Cleanup assembled fragments
                        assembler.cleanup(messageId);
                    } else {
                        std::cerr << "Error: Could not get assembled message or type for " << messageId << std::endl;
                    }
                }
            }
        }
    }
}

void MessageProcessor::handleInputMessage(const MessageFrame& frame) {
    // Debug log for processing
    std::cout << "MessageProcessor: Processing fragment " << frame.header.messageId 
              << " [" << frame.header.sequenceNumber << "]" << std::endl;
    
    auto messageIdOpt = assembler.addFragment(frame);
    
    // If the message is complete addFragment returns a valid messageId, nullopt otherwise
    if (messageIdOpt) {
        std::string messageId = messageIdOpt.value();
        
        auto payloadOpt = assembler.getAssembledMessage(messageId);
        auto typeOpt = assembler.getMessageType(messageId);
        
        if (payloadOpt && typeOpt) {
            handleCompleteMessage(messageId, payloadOpt.value(), typeOpt.value());
            assembler.cleanup(messageId);
        } else {
            std::cerr << "Error: Could not get assembled message or type for " << messageId << std::endl;
        }
    }
}

void MessageProcessor::handleCompleteMessage(const std::string& messageId, const std::string& payload, MessageType type) {
    if (system) {
        system->handleCompleteMessage(messageId, payload, type);
    } else {
        std::cerr << "MessageProcessor: No system reference available" << std::endl;
    }
}