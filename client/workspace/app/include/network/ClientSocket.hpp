#pragma once

#include "/common/include/network/ISocket.hpp"
#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>
#include <atomic>
#include <functional>
#include <QTimer>

class ClientSocket : public QObject, public ISocket {
    Q_OBJECT

private:
    QTcpSocket* socket;
    QThread* sendThread;

    std::atomic<bool> connected;
    QMutex connectionMutex;
    QWaitCondition connectionCondition;

    std::atomic<bool> running;

    QMutex sendMutex;
    QWaitCondition sendCondition;
    QQueue<ISocket::Message> sendQueue;
    
    QQueue<ISocket::Message> receiveQueue;
    QMutex receiveMutex;

    std::function<void()> onConnectedCallback;
    std::function<void()> onDisconnectedCallback;
    std::function<void(const ISocket::Message&)> onMessageReceivedCallback;

private slots:
    void processSendQueue();

public:
    explicit ClientSocket(QObject* parent = nullptr);
    ~ClientSocket() override;

    int getServerFd() const override;
    int getClientFd() const override;

    bool connectToHost(const QString& host, quint16 port);

    bool sendMessage(const ISocket::Message& message) override;
    bool disconnect() override;
    bool isConnected() override;

    void setOnConnectedCallback(std::function<void()> callback) override;
    void setOnDisconnectedCallback(std::function<void()> callback) override;
    void setOnMessageReceivedCallback(std::function<void(const ISocket::Message&)> callback) override;

};