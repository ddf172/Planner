#include "include/network/ClientSocket.hpp"
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "Creating client...";
    ClientSocket client;
    
    client.setOnConnectedCallback([]() {
        qDebug() << "Connected to server!";
    });
    
    client.setOnDisconnectedCallback([]() {
        qDebug() << "Disconnected from server!";
        QCoreApplication::quit();
    });
    
    client.setOnMessageReceivedCallback([](const ISocket::Message& message) {
        qDebug() << "Message received:" << QString::fromStdString(message.content);
    });
    
    qDebug() << "Connecting to server...";
    if (!client.connectToHost("localhost", 8080)) {
        qDebug() << "Failed to connect to server!";
        return 1;
    }
    
    // Send messages to server every 2 seconds
    QTimer *timer = new QTimer(&app);
    QObject::connect(timer, &QTimer::timeout, [&client]() {
        if (client.isConnected()) {
            // Get message from user
            std::cout << "Enter message (or type 'exit' to quit): ";
            std::string input;
            std::getline(std::cin, input);
            
            // Check if user wants to quit
            if (input == "exit") {
                client.disconnect();
                QCoreApplication::quit();
                return;
            }
            
            // Send message
            ISocket::Message message;
            message.type = ISocket::MessageType::Text;
            message.content = input;
            
            if (client.sendMessage(message)) {
                qDebug() << "Message sent:" << QString::fromStdString(input);
            } else {
                qDebug() << "Failed to send message!";
            }
        }
    });
    timer->start(2000); // Every 2 seconds
    
    // Run event loop
    return app.exec();
}