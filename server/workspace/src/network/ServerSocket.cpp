#include "/workspace/include/network/ServerSocket.hpp"

ServerSocket::ServerSocket(int port){
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(serverSocket, 5) < 0) {
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
    close(serverSocket);
    serverSocket = -1;
}

int ServerSocket::getClientFd() const {
    return clientSocket;
}

int ServerSocket::getServerFd() const {
    return serverSocket;
}

bool ServerSocket::sendMessage(const ISocket::Message& message){
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

    clientSocket = ::accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0) {
        return false;
    }

    // Set the socket to non-blocking mode
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000; // 100ms
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    connected = true;
    if (onConnectedCallback) {
        onConnectedCallback();
    }
    return true;
}

bool ServerSocket::disconnect(){
    std::lock_guard<std::mutex> lock(connectionMutex);
    if (!connected) return false;
    connected = false;

    shutdown(clientSocket, SHUT_RDWR);
    close(clientSocket);
    clientSocket = -1;
    if (onDisconnectedCallback) {
        onDisconnectedCallback();
    }

    connectionCondition.notify_all();
    return true;
}

bool ServerSocket::isConnected(){
    return connected;
}

void ServerSocket::setOnConnectedCallback(std::function<void()> callback){
    onConnectedCallback = callback;
}

void ServerSocket::setOnDisconnectedCallback(std::function<void()> callback){
    onDisconnectedCallback = callback;
}

void ServerSocket::setOnMessageReceivedCallback(std::function<void(const ISocket::Message&)> callback){
    onMessageReceivedCallback = callback;
}

void ServerSocket::receiveMessages(){
    while (running) {
        if (!connected) {
            std::unique_lock<std::mutex> lock(connectionMutex);
            connectionCondition.wait(lock, [this] { return connected || !running; });
            continue;

        }

        char buffer[1024] = {0};
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        
        // Check for EAGAIN or EWOULDBLOCK errors (timeout errors)
        if (bytesReceived < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            continue; 
        }
        
        // Check for other errors or connection closure from the client
        if (bytesReceived <= 0) {
            disconnect();
            continue;
        }

        ISocket::Message message;
        message.type = ISocket::MessageType::Text;
        message.content = std::string(buffer, bytesReceived);

        std::lock_guard<std::mutex> lock(receiveMutex);
        receiveQueue.push(message);
        if (onMessageReceivedCallback) {
            onMessageReceivedCallback(message);
        }
    }
}

void ServerSocket::sendMessages(){
    while (running) {
        if (!connected) {
            std::unique_lock<std::mutex> lock(connectionMutex);
            connectionCondition.wait(lock, [this] { return connected || !running; });
            continue;
        }
        std::unique_lock<std::mutex> lock(sendMutex);
        sendCondition.wait(lock, [this] { return !sendQueue.empty() || !running; });

        if (!running) {
            break;
        }
        if (!connected) {
            continue;
        }
        while (!sendQueue.empty()) {
            ISocket::Message message = sendQueue.front();
            sendQueue.pop();

            const char* data = message.content.c_str();
            size_t totalSent = 0;
            size_t toSend = message.content.size();

            while (totalSent < toSend) {
                ssize_t bytesSent = send(clientSocket, data + totalSent, toSend - totalSent, 0);
                if (bytesSent <= 0) {
                    disconnect();
                    break;
                }
                totalSent += bytesSent;
            }
        }
    }
}