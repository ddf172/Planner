#include "network/ServerSocket.hpp"

ServerSocket::ServerSocket(int port){
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(serverSocket);
        throw std::runtime_error("Failed to set socket options");
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        close(serverSocket);
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(serverSocket, 5) < 0) {
        close(serverSocket);
        throw std::runtime_error("Failed to listen on socket");
    }

    connected = false;
    running = true;

    receiveThread = std::thread(&ServerSocket::receiveMessages, this);
    sendThread = std::thread(&ServerSocket::sendMessages, this);
}

ServerSocket::~ServerSocket(){
    if (isConnected()) disconnect();

    running = false;
    connectionCondition.notify_all();

    if (receiveThread.joinable()) {
        receiveThread.join();
    }
    if (sendThread.joinable()) {
        sendThread.join();
    }
    
    if (serverSocket >= 0) {
        close(serverSocket);
        serverSocket = -1;
    }
}

int ServerSocket::getClientFd() const {
    return clientSocket;
}

int ServerSocket::getServerFd() const {
    return serverSocket;
}

bool ServerSocket::sendMessage(const MessageFrame& message){
    if (!connected) {
        return false;
    }
    std::lock_guard<std::mutex> lock(sendMutex);
    sendQueue.push(message);
    sendCondition.notify_one();
    return true;
}

bool ServerSocket::accept(){
    if (connected) return false;
    if (!running) return false;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(serverSocket, &readfds);
    
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000; // 500ms timeout
    
    int activity = select(serverSocket + 1, &readfds, NULL, NULL, &tv);
    if (activity <= 0) {
        return false; // Timeout or error
    }

    clientSocket = ::accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0) {
        return false;
    }

    //non-blocking
    struct timeval recv_tv;
    recv_tv.tv_sec = 0;
    recv_tv.tv_usec = 100000; // 100ms
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &recv_tv, sizeof(recv_tv));

    connected = true;
    std::lock_guard<std::mutex> lock(connectionMutex);
    connectionCondition.notify_all();

    if (onConnectedCallback) {
        onConnectedCallback();
    }
    return true;
}

bool ServerSocket::disconnect(){
    std::lock_guard<std::mutex> lock(connectionMutex);
    if (!connected) return false;
    connected = false;

    if (clientSocket >= 0) {
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        clientSocket = -1;
    }

    if (onDisconnectedCallback) {
        onDisconnectedCallback();
    }

    connectionCondition.notify_all();
    return true;
}

bool ServerSocket::isConnected() const {
    return connected;
}

void ServerSocket::setOnConnectedCallback(std::function<void()> callback){
    onConnectedCallback = callback;
}

void ServerSocket::setOnDisconnectedCallback(std::function<void()> callback){
    onDisconnectedCallback = callback;
}

void ServerSocket::setOnMessageReceivedCallback(std::function<void(const MessageFrame&)> callback){
    onMessageReceivedCallback = callback;
}

void ServerSocket::receiveMessages(){
    while (running) {
        if (!running) break;
        
        if (!connected) {
            std::unique_lock<std::mutex> lock(connectionMutex);
            connectionCondition.wait_for(lock, std::chrono::milliseconds(500),
                [this] { return connected || !running; });
            if (!running) break;
            if (!connected) continue;
        }

        char buffer[4096] = {0};
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        
        if (!running) break;
        
        if (bytesReceived < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue; 
        }
        
        if (bytesReceived <= 0) {
            disconnect();
            continue;
        }

        if (bytesReceived > 0) {
            try {
                std::string jsonStr(buffer, bytesReceived);
                json j = json::parse(jsonStr);
                MessageFrame message = j.get<MessageFrame>();

                std::lock_guard<std::mutex> lock(receiveMutex);
                receiveQueue.push(message);
                if (onMessageReceivedCallback) {
                    onMessageReceivedCallback(message);
                }
                receiveCondition.notify_one();
            } catch (const std::exception& e) {
                std::cerr << "Error parsing received message: " << e.what() << std::endl;
            }
        }
    }
}

void ServerSocket::sendMessages(){
    while (running) {
        if (!connected) {
            std::unique_lock<std::mutex> lock(connectionMutex);
            connectionCondition.wait_for(lock, std::chrono::milliseconds(500),
                [this] { return connected || !running; });
            if (!running) break;
            if (!connected) continue;
        }
        
        std::unique_lock<std::mutex> lock(sendMutex);
        sendCondition.wait_for(lock, std::chrono::milliseconds(500),
            [this] { return !sendQueue.empty() || !running || !connected; });

        if (!running) break;
        if (!connected) continue;
        
        while (!sendQueue.empty()) {
            MessageFrame message = sendQueue.front();
            sendQueue.pop();

            try {
                json j = message;
                std::string jsonStr = j.dump();
                
                const char* data = jsonStr.c_str();
                size_t totalSent = 0;
                size_t toSend = jsonStr.size();

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