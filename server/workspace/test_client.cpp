#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <chrono>
#include "include/message/MessageFrame.hpp"

class TestClient {
private:
    int clientSocket;
    bool connected;

public:
    TestClient() : clientSocket(-1), connected(false) {}
    
    ~TestClient() {
        disconnect();
    }
    
    bool connect(const std::string& host, int port) {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }
        
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        
        if (inet_pton(AF_INET, host.c_str(), &serverAddress.sin_addr) <= 0) {
            std::cerr << "Invalid address" << std::endl;
            close(clientSocket);
            return false;
        }
        
        if (::connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
            std::cerr << "Connection failed" << std::endl;
            close(clientSocket);
            return false;
        }
        
        connected = true;
        std::cout << "✓ Connected to server at " << host << ":" << port << std::endl;
        return true;
    }
    
    void disconnect() {
        if (connected && clientSocket >= 0) {
            close(clientSocket);
            connected = false;
            std::cout << "✓ Disconnected from server" << std::endl;
        }
    }
    
    bool sendMessage(const MessageFrame& frame) {
        if (!connected) {
            std::cerr << "✗ Not connected to server" << std::endl;
            return false;
        }
        
        try {
            json j = frame;
            std::string jsonStr = j.dump();
            
            const char* data = jsonStr.c_str();
            size_t totalSent = 0;
            size_t toSend = jsonStr.size();
            
            while (totalSent < toSend) {
                ssize_t bytesSent = send(clientSocket, data + totalSent, toSend - totalSent, 0);
                if (bytesSent < 0) {
                    std::cerr << "✗ Failed to send message" << std::endl;
                    return false;
                }
                totalSent += bytesSent;
            }
            
            std::cout << "→ Sent: " << frame.header.messageId << " (Type: " << static_cast<int>(frame.header.type) << ")" << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "✗ Error serializing message: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool receiveResponse(int timeoutMs = 5000) {
        if (!connected) {
            std::cerr << "✗ Not connected to server" << std::endl;
            return false;
        }
        
        // Set timeout for receiving
        struct timeval tv;
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        
        char buffer[4096] = {0};
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        
        if (bytesReceived <= 0) {
            std::cerr << "⚠ No response received (timeout or connection closed)" << std::endl;
            return false;
        }
        
        try {
            std::string jsonStr(buffer, bytesReceived);
            json j = json::parse(jsonStr);
            MessageFrame responseFrame = j.get<MessageFrame>();
            
            std::cout << "← Received: " << responseFrame.header.messageId << " (Type: " << static_cast<int>(responseFrame.header.type) << ")" << std::endl;
            
            // Parse and display response payload
            json responsePayload = json::parse(responseFrame.payload);
            std::cout << "  Response Data: " << responsePayload.dump(2) << std::endl;
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "✗ Error parsing response: " << e.what() << std::endl;
            std::cout << "Raw response: " << std::string(buffer, bytesReceived) << std::endl;
            return false;
        }
    }
    
    bool sendAndWaitForResponse(const MessageFrame& frame, int timeoutMs = 5000) {
        if (!sendMessage(frame)) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small delay
        return receiveResponse(timeoutMs);
    }
    
    bool isConnected() const {
        return connected;
    }
};

// Helper function to create a test message
MessageFrame createTestMessage(const std::string& payload, MessageType type, const std::string& messageId) {
    MessageFrame frame;
    
    frame.header.messageId = messageId;
    frame.header.sequenceNumber = 0;  // Single fragment message
    frame.header.isLast = true;       // This is the last (and only) fragment
    frame.header.payloadSize = payload.size();
    frame.header.type = type;
    frame.payload = payload;
    
    return frame;
}

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << " " << title << std::endl;
    std::cout << std::string(50, '=') << std::endl;
}

void printTestHeader(int testNum, const std::string& description) {
    std::cout << "\n[Test " << testNum << "] " << description << std::endl;
    std::cout << std::string(40, '-') << std::endl;
}

int main() {
    TestClient client;
    
    printSeparator("SERVER COMMUNICATION PROTOCOL TEST");
    
    // Connect to server
    std::cout << "\nConnecting to server..." << std::endl;
    if (!client.connect("127.0.0.1", 8080)) {
        std::cerr << "✗ Failed to connect to server" << std::endl;
        return 1;
    }
    
    // Wait a moment for connection to stabilize
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    printSeparator("COMMAND TESTS");
    
    // Test 1: Command - Ping
    printTestHeader(1, "PING command");
    {
        json pingCmd = {{"command", "ping"}};
        MessageFrame frame = createTestMessage(pingCmd.dump(), MessageType::Command, "cmd-ping-001");
        if (!client.sendAndWaitForResponse(frame)) {
            std::cerr << "✗ Ping test failed" << std::endl;
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Test 2: Command - Status
    printTestHeader(2, "STATUS command");
    {
        json statusCmd = {{"command", "status"}};
        MessageFrame frame = createTestMessage(statusCmd.dump(), MessageType::Command, "cmd-status-001");
        if (!client.sendAndWaitForResponse(frame)) {
            std::cerr << "✗ Status test failed" << std::endl;
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    printSeparator("DEBUG TESTS");
    
    // Test 3: Debug - Print Payload
    printTestHeader(3, "DEBUG print_payload command");
    {
        json debugCmd = {
            {"debug", "print_payload"},
            {"test_data", {
                {"user", "test_client"},
                {"action", "protocol_verification"},
                {"timestamp", "2025-01-10T12:00:00Z"},
                {"details", {
                    {"test_type", "print_payload"},
                    {"expected_behavior", "server should print this payload to console"}
                }}
            }}
        };
        MessageFrame frame = createTestMessage(debugCmd.dump(), MessageType::Debug, "debug-print-001");
        if (!client.sendAndWaitForResponse(frame)) {
            std::cerr << "✗ Debug print_payload test failed" << std::endl;
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Test 4: Debug - Server Info
    printTestHeader(4, "DEBUG server_info command");
    {
        json debugCmd = {{"debug", "server_info"}};
        MessageFrame frame = createTestMessage(debugCmd.dump(), MessageType::Debug, "debug-info-001");
        if (!client.sendAndWaitForResponse(frame)) {
            std::cerr << "✗ Debug server_info test failed" << std::endl;
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Test 5: Debug - Uptime
    printTestHeader(5, "DEBUG uptime command");
    {
        json debugCmd = {{"debug", "uptime"}};
        MessageFrame frame = createTestMessage(debugCmd.dump(), MessageType::Debug, "debug-uptime-001");
        if (!client.sendAndWaitForResponse(frame)) {
            std::cerr << "✗ Debug uptime test failed" << std::endl;
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    printSeparator("DATA TEST");
    
    // Test 6: Data - Single comprehensive test
    printTestHeader(6, "DATA message");
    {
        json dataMsg = {
            {"type", "protocol_test"},
            {"data", {
                {"message", "Comprehensive protocol test from test client"},
                {"test_id", 12345},
                {"features_tested", {
                    "command_ping", "command_status", "debug_print_payload", 
                    "debug_server_info", "debug_uptime", "data_acknowledgment"
                }},
                {"client_info", {
                    {"version", "1.0"},
                    {"build_date", "2025-01-10"},
                    {"purpose", "Protocol verification and testing"}
                }}
            }},
            {"metadata", {
                {"source", "test_client"},
                {"timestamp", "2025-01-10T12:00:00Z"},
                {"expected_response", "acknowledgment"}
            }}
        };
        MessageFrame frame = createTestMessage(dataMsg.dump(), MessageType::Data, "data-test-001");
        if (!client.sendAndWaitForResponse(frame)) {
            std::cerr << "✗ Data test failed" << std::endl;
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    printSeparator("ERROR HANDLING TESTS");
    
    // Test 7: Error handling - Unknown command
    printTestHeader(7, "ERROR handling - Unknown command");
    {
        json badCmd = {{"command", "invalid_command_test"}};
        MessageFrame frame = createTestMessage(badCmd.dump(), MessageType::Command, "cmd-error-001");
        if (!client.sendAndWaitForResponse(frame)) {
            std::cerr << "✗ Error handling test failed" << std::endl;
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Test 8: Error handling - Unknown debug command
    printTestHeader(8, "ERROR handling - Unknown debug command");
    {
        json badDebug = {{"debug", "invalid_debug_command"}};
        MessageFrame frame = createTestMessage(badDebug.dump(), MessageType::Debug, "debug-error-001");
        if (!client.sendAndWaitForResponse(frame)) {
            std::cerr << "✗ Debug error handling test failed" << std::endl;
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Test 9: Error handling - Malformed command (missing command field)
    printTestHeader(9, "ERROR handling - Malformed command");
    {
        json malformedCmd = {{"invalid_field", "test"}};
        MessageFrame frame = createTestMessage(malformedCmd.dump(), MessageType::Command, "cmd-malformed-001");
        if (!client.sendAndWaitForResponse(frame)) {
            std::cerr << "✗ Malformed command test failed" << std::endl;
        }
    }
    
    printSeparator("ALL TESTS COMPLETED");
    std::cout << "\n✓ All protocol tests have been executed." << std::endl;
    std::cout << "✓ Check server console output for debug print_payload results." << std::endl;
    std::cout << "✓ All responses should show proper acknowledgments and error codes." << std::endl;
    
    std::cout << "\nPress Enter to test STOP command (this will shutdown the server)..." << std::endl;
    std::cin.get();
    
    // Test 10: Stop command (will shutdown server)
    printTestHeader(10, "STOP command - Server Shutdown");
    {
        json stopCmd = {{"command", "stop"}};
        MessageFrame frame = createTestMessage(stopCmd.dump(), MessageType::Command, "cmd-stop-001");
        std::cout << "⚠ Sending server shutdown command..." << std::endl;
        if (!client.sendAndWaitForResponse(frame, 2000)) { // Shorter timeout since server will shutdown
            std::cout << "ℹ Server may have shutdown before response (expected behavior)" << std::endl;
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    client.disconnect();
    
    printSeparator("TEST SESSION COMPLETE");
    std::cout << "✓ Test client finished successfully." << std::endl;
    
    return 0;
}
