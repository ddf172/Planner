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
    
    inputCondition.notify_all();
    
    if (serverSocket) {
        serverSocket->getReceiveCondition().notify_all();
    }
    
    if (processingThread.joinable()) {
        if (std::this_thread::get_id() == processingThread.get_id()) {
            std::cout << "WARNING: Trying to join processing thread from itself - skipping join" << std::endl;
            processingThread.detach();
        } else {
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

void MessageProcessor::registerHandler(MessageType type, IMessageHandler* handler) {
    dispatcher.registerHandler(type, handler);
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
        {
            // Lock the local queue to prepare for message processing.
            std::unique_lock<std::mutex> lock(inputMutex);

            if (inputQueue.empty())
            {
                if (!serverSocket) {

                    inputCondition.wait_for(lock, std::chrono::milliseconds(100), 
                        [this] { return !inputQueue.empty() || !running; });
                    
                    if (!running) {
                        std::cout << "MessageProcessor: processLoop stopping (no serverSocket)" << std::endl;
                        return;
                    }
                    continue;
                }
                
                try {
                    std::unique_lock<std::mutex> serverLock(serverSocket->getReceiveMutex());
                    
                    if (!running) {
                        std::cout << "MessageProcessor: processLoop stopping (before wait)" << std::endl;
                        return;
                    }

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
                } catch (const std::exception& e) {
                    std::cerr << "Exception in processLoop wait: " << e.what() << std::endl;
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    if (!running) {
                        return;
                    }
                    continue;
                }

                // At this point, we hold the lock and the server queue is not empty.
                // Transfer all messages from the server's queue to our local queue.
                try {
                    if (!running) {
                        std::cout << "MessageProcessor: processLoop stopping (before transfer)" << std::endl;
                        return;
                    }
                    
                    if (serverSocket) {
                        while (!serverSocket->getReceiveQueue().empty() && running)
                        {
                            inputQueue.push(serverSocket->getReceiveQueue().front());
                            serverSocket->getReceiveQueue().pop();
                        }
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Exception transferring messages: " << e.what() << std::endl;
                }
            }

            // Move all messages to a local queue to minimize lock time.
            while (!inputQueue.empty())
            {
                localQueue.push_back(inputQueue.front());
                inputQueue.pop();
            }
        } // Release inputMutex

        // Process messages outside of the lock.
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
                    std::string payload = assembler.getAssembledMessage(messageId);
                    MessageType type = assembler.getMessageType(messageId);
                    
                    // Dispatch the complete message
                    handleCompleteMessage(messageId, payload, type);
                    
                    // Cleanup assembled fragments
                    assembler.cleanup(messageId);
                }
            }
        }
    }
}

void MessageProcessor::handleInputMessage(const MessageFrame& frame) {
    std::cout << "MessageProcessor: Processing fragment " << frame.header.messageId 
              << " [" << frame.header.sequenceNumber << "]" << std::endl;
    
    auto messageIdOpt = assembler.addFragment(frame);
    
    if (messageIdOpt) {
        std::string messageId = messageIdOpt.value();
        std::string payload = assembler.getAssembledMessage(messageId);
        MessageType type = assembler.getMessageType(messageId);
        
        handleCompleteMessage(messageId, payload, type);
        assembler.cleanup(messageId);
    }
}

void MessageProcessor::handleCompleteMessage(const std::string& messageId, const std::string& payload, MessageType type) {
    if (system) {
        dispatcher.dispatch(messageId, payload, type, *system);
    } else {
        std::cerr << "MessageProcessor: No system reference available" << std::endl;
    }
}
