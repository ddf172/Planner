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
    std::cout << "ServerSocket: destructor called" << std::endl;

    if (isConnected()) {
        std::cout << "ServerSocket: disconnecting client in destructor" << std::endl;
        disconnect();
    }

    std::cout << "ServerSocket: stopping threads" << std::endl;
    running = false;
    
    connectionCondition.notify_all();
    sendCondition.notify_all();
    receiveCondition.notify_all();

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
    {
        std::lock_guard<std::mutex> lock(connectionMutex);
        if (!connected) return false;
        connected = false;

        if (clientSocket >= 0) {
            std::cout << "ServerSocket: disconnecting client (fd=" << clientSocket << ")" << std::endl;
            try {
                shutdown(clientSocket, SHUT_RDWR);
                close(clientSocket);
                clientSocket = -1;
            } catch (const std::exception& e) {
                std::cerr << "Error during socket shutdown: " << e.what() << std::endl;
            }
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