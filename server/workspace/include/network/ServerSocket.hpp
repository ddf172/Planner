#pragma once

#include "/common/include/network/ISocket.hpp"
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

class ServerSocket : public ISocket {
private:
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
    std::queue<ISocket::Message> sendQueue;
    
    std::queue<ISocket::Message> receiveQueue;
    std::mutex receiveMutex;
    std::condition_variable receiveCondition;


    std::function<void()> onConnectedCallback;
    std::function<void()> onDisconnectedCallback;
    std::function<void(const ISocket::Message&)> onMessageReceivedCallback;

    void receiveMessages();
    void sendMessages();

public:
    ServerSocket(int port);
    ~ServerSocket();

    int getServerFd() const override;
    int getClientFd() const override;

    bool sendMessage(const ISocket::Message& message) override;

    /**
    * @brief Accepts a new client connection.
    * @return true if the connection was successful, false if other client is already connected or connection failed.
    */
    bool accept();
    /**
    * @brief Disconnects the current client.
    * @return true if the disconnection was successful, false if no client is connected.
    */
    bool disconnect() override;
    bool isConnected() override;

    void setOnConnectedCallback(std::function<void()> callback) override;
    void setOnDisconnectedCallback(std::function<void()> callback) override;
    void setOnMessageReceivedCallback(std::function<void(const ISocket::Message&)> callback) override;
};