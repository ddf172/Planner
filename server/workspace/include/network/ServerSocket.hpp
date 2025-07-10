#pragma once

#include "message/MessageFrame.hpp"
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <iostream>
#include <optional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

class ServerSocket {
public:
    ServerSocket(int port);
    ~ServerSocket();

    bool accept();
    bool disconnect();
    bool isConnected() const;
    bool sendMessage(const MessageFrame& message);

    int getClientFd() const;
    int getServerFd() const;

    std::mutex& getReceiveMutex();
    std::condition_variable& getReceiveCondition();
    std::queue<MessageFrame>& getReceiveQueue();
    
    void setOnConnectedCallback(std::function<void()> callback);
    void setOnDisconnectedCallback(std::function<void()> callback);

private:
    void receiveMessages();
    void sendMessages();

    int serverSocket;
    int clientSocket;

    std::thread receiveThread;
    std::thread sendThread;

    std::atomic<bool> connected;
    std::mutex connectionMutex;
    std::condition_variable connectionCondition;

    std::atomic<bool> running;

    std::mutex sendMutex;
    std::condition_variable sendCondition;

    std::queue<MessageFrame> receiveQueue;
    std::mutex receiveMutex;
    std::condition_variable receiveCondition;

    std::queue<MessageFrame> sendQueue;

    std::function<void()> onConnectedCallback;
    std::function<void()> onDisconnectedCallback;
};