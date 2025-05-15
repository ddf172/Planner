#include "../../include/network/ClientSocket.hpp"

ClientSocket::ClientSocket(QObject* parent) : QObject(parent) {
    socket = new QTcpSocket(this);
    connected = false;
    running = true;
    
    // Połącz sygnały socketu z callbackami
    connect(socket, &QTcpSocket::connected, this, [this]() {
        connected = true;
        if (onConnectedCallback) {
            onConnectedCallback();
        }
    });
    
    connect(socket, &QTcpSocket::disconnected, this, [this]() {
        connected = false;
        if (onDisconnectedCallback) {
            onDisconnectedCallback();
        }
    });
    
    connect(socket, &QTcpSocket::readyRead, this, [this]() {
        QByteArray data = socket->readAll();
        if (data.size() > 0) {
            ISocket::Message message;
            message.type = ISocket::MessageType::Text;
            message.content = QString(data).toStdString();
            
            QMutexLocker locker(&receiveMutex);
            receiveQueue.enqueue(message);
            
            if (onMessageReceivedCallback) {
                onMessageReceivedCallback(message);
            }
        }
    });
    
    // Utworzenie i start wątku wysyłającego w sposób zgodny z Qt
    sendThread = new QThread();
    
    // Utwórz timer do wysyłania wiadomości zamiast korzystania z moveToThread
    QTimer* sendTimer = new QTimer();
    sendTimer->setInterval(10); // sprawdzaj co 10ms
    sendTimer->moveToThread(sendThread);
    
    connect(sendThread, &QThread::started, sendTimer, 
            static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(sendThread, &QThread::finished, sendTimer, &QTimer::stop);
    connect(sendTimer, &QTimer::timeout, this, &ClientSocket::processSendQueue);
    
    sendThread->start();
}

ClientSocket::~ClientSocket() {
    if (isConnected()) {
        disconnect();
    }
    
    running = false;
    sendCondition.wakeAll();
    
    if (sendThread && sendThread->isRunning()) {
        sendThread->quit();
        sendThread->wait();
        delete sendThread;
    }

    delete socket;
}

int ClientSocket::getClientFd() const {
    return socket ? socket->socketDescriptor() : -1;
}

int ClientSocket::getServerFd() const {
    return -1; // Klienty nie mają serwerowego gniazda
}

bool ClientSocket::connectToHost(const QString& host, quint16 port) {
    if (connected) {
        return false;
    }
    
    socket->connectToHost(host, port);
    return socket->waitForConnected(5000); // 5 sekund timeout
}

bool ClientSocket::sendMessage(const ISocket::Message& message) {
    if (!connected) {
        return false;
    }
    
    QMutexLocker locker(&sendMutex);
    sendQueue.enqueue(message);
    sendCondition.wakeOne();
    return true;
}

bool ClientSocket::disconnect() {
    QMutexLocker locker(&connectionMutex);
    if (!connected) {
        return false;
    }
    
    socket->disconnectFromHost();
    
    if (socket->state() != QAbstractSocket::UnconnectedState) {
        socket->waitForDisconnected(1000);
    }
    
    return true;
}

bool ClientSocket::isConnected() {
    return connected && socket && (socket->state() == QAbstractSocket::ConnectedState);
}

void ClientSocket::setOnConnectedCallback(std::function<void()> callback) {
    onConnectedCallback = callback;
}

void ClientSocket::setOnDisconnectedCallback(std::function<void()> callback) {
    onDisconnectedCallback = callback;
}

void ClientSocket::setOnMessageReceivedCallback(std::function<void(const ISocket::Message&)> callback) {
    onMessageReceivedCallback = callback;
}

void ClientSocket::processSendQueue() {
    QMutexLocker locker(&sendMutex);
    
    if (sendQueue.isEmpty() || !connected) {
        return;
    }
    
    while (!sendQueue.isEmpty() && connected) {
        ISocket::Message message = sendQueue.dequeue();
        locker.unlock(); // Odblokuj mutex podczas wysyłania
        
        QMetaObject::invokeMethod(socket, [this, message]() {
            QByteArray data = QByteArray::fromStdString(message.content);
            socket->write(data);
            socket->flush();
        }, Qt::QueuedConnection);
        
        locker.relock(); // Zablokuj mutex ponownie
    }
}