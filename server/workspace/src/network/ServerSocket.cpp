#include "network/ServerSocket.hpp"

ServerSocket::ServerSocket(int port){
    // Initialize server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    // Enable address / port reuse
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(serverSocket);
        throw std::runtime_error("Failed to set socket options");
    }

    // Bind socket to specified port
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        close(serverSocket);
        throw std::runtime_error("Failed to bind socket");
    }

    // Start listening for incoming connections
    if (listen(serverSocket, 5) < 0) {
        close(serverSocket);
        throw std::runtime_error("Failed to listen on socket");
    }

    // Initialize member variables
    connected = false;
    running = true;

    // Start threads for receiving and sending messages
    receiveThread = std::thread(&ServerSocket::receiveMessages, this);
    sendThread = std::thread(&ServerSocket::sendMessages, this);
}

ServerSocket::~ServerSocket(){
    std::cout << "ServerSocket: destructor called" << std::endl;

    // Disconnect before destroying the object
    if (isConnected()) {
        std::cout << "ServerSocket: disconnecting client in destructor" << std::endl;
        disconnect();
    }

    std::cout << "ServerSocket: stopping threads" << std::endl;
    running = false;
    
    // Notify threads to wake up and stop
    connectionCondition.notify_all();
    sendCondition.notify_all();
    receiveCondition.notify_all();

    // Join threads to ensure they finish before destruction
    if (receiveThread.joinable()) {
        try {
            std::cout << "ServerSocket: joining receiveThread" << std::endl;
            receiveThread.join();
        } catch (const std::exception& e) {
            std::cerr << "Error joining receiveThread: " << e.what() << std::endl;
        }
    }
    
    if (sendThread.joinable()) {
        try {
            std::cout << "ServerSocket: joining sendThread" << std::endl;
            sendThread.join();
        } catch (const std::exception& e) {
            std::cerr << "Error joining sendThread: " << e.what() << std::endl;
        }
    }
    
    std::cout << "ServerSocket: threads stopped" << std::endl;
    
    // Close server socket
    if (serverSocket >= 0) {
        close(serverSocket);
        serverSocket = -1;
    }
}

bool ServerSocket::sendMessage(const MessageFrame& message){
    if (!running || !connected) {
        return false;
    }

    // Take lock on sendMutex and push message to sendQueue and notify sendThread
    std::lock_guard<std::mutex> lock(sendMutex);
    sendQueue.push(message);
    sendCondition.notify_one();
    return true;
}

bool ServerSocket::accept(){
    if (!running) return false;
    if (connected) return false;

    // Set server socket to non-blocking mode
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(serverSocket, &readfds);
    
    // Set a timeout for select to avoid blocking indefinitely
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000; // 500ms timeout
    
    // Wait for incoming connections
    if (select(serverSocket + 1, &readfds, NULL, NULL, &tv)<= 0) {
        return false; // Timeout or error
    }

    // Accept the incoming connection
    clientSocket = ::accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0) {
        return false;
    }

    // Set client socket to non-blocking mode
    struct timeval recv_tv;
    recv_tv.tv_sec = 0;
    recv_tv.tv_usec = 100000; // 100ms
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &recv_tv, sizeof(recv_tv));

    // Set connected state
    std::lock_guard<std::mutex> lock(connectionMutex);
    connected = true;
    connectionCondition.notify_all();

    if (onConnectedCallback) {
        onConnectedCallback();
    }
    return true;
}

bool ServerSocket::disconnect(){
    // Parentheses to ensure the lock is released before calling the callback
    {
        std::lock_guard<std::mutex> lock(connectionMutex);
        if (!connected) return false;
        connected = false;

        // Shutdown and close the client socket if clientSocket is valid
        if (clientSocket >= 0) {
            std::cout << "ServerSocket: disconnecting client (fd=" << clientSocket << ")" << std::endl;
            if (shutdown(clientSocket, SHUT_RDWR) < 0) {
                std::cerr << "Error shutting down client socket: " << strerror(errno) << std::endl;
            }
            if (close(clientSocket) < 0) {
                std::cerr << "Error closing client socket: " << strerror(errno) << std::endl;
            }
            clientSocket = -1;
        }

        connectionCondition.notify_all();
    }

    if (onDisconnectedCallback) {
        try {
            onDisconnectedCallback();
        } catch (const std::exception& e) {
            std::cerr << "Exception in disconnect callback: " << e.what() << std::endl;
        }
    }

    return true;
}

bool ServerSocket::isConnected() const {
    return connected;
}

std::mutex& ServerSocket::getReceiveMutex() {
    return receiveMutex;
}

std::condition_variable& ServerSocket::getReceiveCondition() {
    return receiveCondition;
}

std::queue<MessageFrame>& ServerSocket::getReceiveQueue() {
    return receiveQueue;
}

void ServerSocket::setOnConnectedCallback(std::function<void()> callback) {
    onConnectedCallback = callback;
}

void ServerSocket::setOnDisconnectedCallback(std::function<void()> callback){
    onDisconnectedCallback = callback;
}

void ServerSocket::receiveMessages(){
    while (running) {
        
        // Wait for connection to be established
        if (!connected) {
            std::unique_lock<std::mutex> lock(connectionMutex);
            connectionCondition.wait_for(lock, std::chrono::milliseconds(500),
                [this] { return connected || !running; });
            if (!running) break;
            if (!connected) continue;
        }

        char buffer[4096] = {0};
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        
        // If not recieved data, and errno indicates EAGAIN or EWOULDBLOCK, continue to next iteration
        if (bytesReceived < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue; 
        }
        
        // If bytesReceived is 0 or negative, disconnect
        if (bytesReceived <= 0) {
            disconnect();
            continue;
        }

        // Parse and enqueue received data
        try {
            std::string jsonStr(buffer, bytesReceived);
            json j = json::parse(jsonStr);
            MessageFrame message = j.get<MessageFrame>();
            {
                std::lock_guard<std::mutex> lock(receiveMutex);
                receiveQueue.push(message);
            }
            receiveCondition.notify_one();
        } catch (const std::exception& e) {
            std::cerr << "Error parsing received message: " << e.what() << std::endl;
        }
    }
}

void ServerSocket::sendMessages(){
    while (running) {
        // Wait for connection to be established
        if (!connected) {
            std::unique_lock<std::mutex> lock(connectionMutex);
            connectionCondition.wait_for(lock, std::chrono::milliseconds(500),
                [this] { return connected || !running; });
            if (!running) break;
            if (!connected) continue;
        }
        
        // Wait for messages to send
        std::unique_lock<std::mutex> lock(sendMutex);
        sendCondition.wait_for(lock, std::chrono::milliseconds(500),
            [this] { return !sendQueue.empty() || !running || !connected; });

        if (!running) break;
        if (!connected) continue;
        
        // Add all messages from sendQueue to a local queue
        std::queue<MessageFrame> localQueue;
        while (!sendQueue.empty()) {
            localQueue.push(sendQueue.front());
            sendQueue.pop();
        }
        lock.unlock(); // Release lock before processing and sending
        
        // Process and send messages outside of lock
        while (!localQueue.empty()) {
            MessageFrame message = localQueue.front();
            localQueue.pop();

            try {
                // Serialize message to JSON
                json j = message;
                std::string jsonStr = j.dump();
                
                const char* data = jsonStr.c_str();
                size_t totalSent = 0;
                size_t toSend = jsonStr.size();

                // Send data in a loop to handle partial sends
                while (totalSent < toSend && running && connected) {
                    ssize_t bytesSent = send(clientSocket, data + totalSent, toSend - totalSent, 0);
                    if (bytesSent < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                            continue;
                        } else {
                            disconnect();
                            break;
                        }
                    } else if (bytesSent == 0) {
                        disconnect();
                        break;
                    }
                    totalSent += bytesSent;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error serializing message: " << e.what() << std::endl;
            }
        }
    }
}