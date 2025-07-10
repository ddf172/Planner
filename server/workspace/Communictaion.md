# Server Communication Protocol Documentation

## Overview

This document describes the standardized communication protocol used by the server system. The server uses a message-based architecture with three main message types: Command, Data, and Debug.

## Message Structure

### MessageFrame

All communication is wrapped in a `MessageFrame` structure:

```json
{
  "header": {
    "messageId": "unique-message-id",
    "sequenceNumber": 0,
    "isLast": true,
    "payloadSize": 123,
    "type": "Command|Data|Debug"
  },
  "payload": "JSON string containing the actual message data"
}
```

### Header Fields

- **messageId**: Unique identifier for the message (UUID or random string)
- **sequenceNumber**: Fragment number for large messages (0 for single fragment)
- **isLast**: Boolean indicating if this is the last fragment
- **payloadSize**: Size of the payload in bytes
- **type**: Message type enum ("Command", "Data", or "Debug")

## Message Types

### 1. Command Messages

Commands are used to trigger specific actions on the server.

#### Request Format

```json
{
  "command": "command_name",
  "parameters": {
    "param1": "value1",
    "param2": "value2"
  }
}
```

#### Response Format - Success

```json
{
  "status": "success",
  "command": "command_name",
  "message": "Human readable success message",
  "data": {
    // Command-specific response data
  },
  "timestamp": 1641234567
}
```

#### Response Format - Error

```json
{
  "status": "error",
  "command": "command_name",
  "message": "Error description",
  "error_code": "ERROR_CODE_OPTIONAL",
  "timestamp": 1641234567
}
```

#### Available Commands

| Command | Description | Parameters | Response Data |
|---------|-------------|------------|---------------|
| `ping` | Ping-pong test | none | `message`: "pong" |
| `status` | Get server status | none | `server_running`, `client_connected`, `uptime` |
| `stop` | Shutdown server | none | `message`: shutdown confirmation |

#### Command Examples

**Ping Command:**
```json
{
  "command": "ping"
}
```

**Status Command:**
```json
{
  "command": "status"
}
```

**Stop Command:**
```json
{
  "command": "stop"
}
```

### 2. Data Messages

Data messages are used to send application-specific data to the server.

#### Request Format

```json
{
  "type": "data_type",
  "data": {
    // Application-specific data structure
  },
  "metadata": {
    "source": "client_name",
    "timestamp": "2025-01-10T12:00:00Z"
  }
}
```

#### Response Format

Data messages now send acknowledgment responses:

```json
{
  "status": "success",
  "message": "Data received and processed",
  "message_id": "original-message-id",
  "timestamp": 1641234567
}
```

#### Data Examples

**User Data:**
```json
{
  "type": "user_info",
  "data": {
    "name": "John Doe",
    "age": 30,
    "city": "Warsaw"
  },
  "metadata": {
    "source": "web_client",
    "timestamp": "2025-01-10T12:00:00Z"
  }
}
```

**Sensor Data:**
```json
{
  "type": "sensor_reading",
  "data": {
    "sensor_id": "temp_01",
    "value": 23.5,
    "unit": "celsius"
  },
  "metadata": {
    "source": "iot_device",
    "timestamp": "2025-01-10T12:00:00Z"
  }
}
```

### 3. Debug Messages

Debug messages are used for server diagnostics and testing purposes.

#### Request Format

```json
{
  "debug": "debug_command",
  "parameters": {
    "param1": "value1"
  }
}
```

#### Response Format

Debug messages send acknowledgment responses and perform diagnostic actions:

```json
{
  "status": "success",
  "debug": "debug_command",
  "message": "Debug action completed", 
  "data": {
    // Command-specific debug data
  },
  "timestamp": 1641234567
}
```

#### Available Debug Commands

| Command | Description | Console Output | Response Data |
|---------|-------------|----------------|---------------|
| `print_payload` | Print full payload to server console | Full JSON payload with formatting | confirmation message |
| `uptime` | Show server uptime | Uptime info and timestamps | `current_timestamp`, `uptime_seconds` |
| `server_info` | Display server status | Running status, connections | `server_running`, `client_connected` |

#### Debug Examples

**Print Payload:**
```json
{
  "debug": "print_payload",
  "test_data": {"key": "value"}
}
```

**Server Uptime:**
```json
{
  "debug": "uptime"
}
```

**Server Info:**
```json
{
  "debug": "server_info"
}
```

#### Debug Examples

**Print Payload Example:**
```json
{
  "debug": "print_payload",
  "test_data": {
    "user_id": "12345",
    "action": "login"
  }
}
```

**Server Info Example:**
```json
{
  "debug": "server_info"
}
```

## Error Handling

### Common Error Responses

#### Invalid JSON Format
```json
{
  "status": "error",
  "message": "Invalid JSON format",
  "error_code": "INVALID_JSON",
  "timestamp": 1641234567
}
```

#### Missing Required Field
```json
{
  "status": "error",
  "message": "No 'command' field found in payload",
  "error_code": "MISSING_COMMAND_FIELD",
  "timestamp": 1641234567
}
```

#### Unknown Command
```json
{
  "status": "error",
  "message": "Unknown command: invalid_command",
  "error_code": "UNKNOWN_COMMAND",
  "available_commands": ["test", "stop", "status", "ping"],
  "timestamp": 1641234567
}
```

#### Server Error
```json
{
  "status": "error",
  "message": "Internal server error",
  "error_code": "INTERNAL_ERROR",
  "timestamp": 1641234567
}
```

#### Invalid Debug Format
```json
{
  "status": "error",
  "message": "No 'debug' field found in payload",
  "error_code": "MISSING_DEBUG_FIELD",
  "timestamp": 1641234567
}
```

#### Unknown Debug Command
```json
{
  "status": "error",
  "message": "Unknown debug command: invalid_cmd",
  "error_code": "UNKNOWN_DEBUG_COMMAND",
  "available_commands": ["print_payload", "uptime", "server_info"],
  "timestamp": 1641234567
}
```

## Message Fragmentation

For large messages, the system supports automatic fragmentation:

1. Large messages are split into multiple fragments
2. Each fragment has a sequential `sequenceNumber` starting from 0
3. The last fragment has `isLast: true`
4. The server reassembles fragments before processing

### Fragment Example

**Fragment 1:**
```json
{
  "header": {
    "messageId": "large-msg-001",
    "sequenceNumber": 0,
    "isLast": false,
    "payloadSize": 1024,
    "type": "Data"
  },
  "payload": "first part of large message..."
}
```

**Fragment 2 (Final):**
```json
{
  "header": {
    "messageId": "large-msg-001",
    "sequenceNumber": 1,
    "isLast": true,
    "payloadSize": 512,
    "type": "Data"
  },
  "payload": "final part of large message..."
}
```

## Client Implementation Guidelines

### Connection
- Connect to server on TCP port 8080
- Use JSON over TCP for all communication
- Handle connection errors gracefully

### Message Sending
1. Create the payload JSON
2. Wrap in MessageFrame structure
3. Serialize to JSON string
4. Send over TCP socket

### Error Handling
- Always check response `status` field
- Handle network timeouts
- Implement retry logic for critical commands
- Log errors for debugging

### Example Client Code Structure

```cpp
// Create command
json command = {
    {"command", "test"}
};

// Create message frame
MessageFrame frame;
frame.header.messageId = generateUniqueId();
frame.header.sequenceNumber = 0;
frame.header.isLast = true;
frame.header.type = MessageType::Command;
frame.payload = command.dump();
frame.header.payloadSize = frame.payload.size();

// Send message
sendMessage(frame);
```

## Server Implementation Guidelines

### Adding New Commands
1. Add command name to `handleCommandX` method in `CommandHandler`
2. Implement command-specific logic
3. Return standardized response format
4. Update available_commands list
5. Document in this README

### Adding New Message Types
1. Add new enum value to `MessageType`
2. Create new handler class inheriting from `IMessageHandler`
3. Register handler in `main.cpp`
4. Document message format in this README

## Version History

- **v1.0.0**: Initial protocol implementation
  - Basic Command, Data, and Debug message types
  - Message fragmentation support
  - Error handling standardization
