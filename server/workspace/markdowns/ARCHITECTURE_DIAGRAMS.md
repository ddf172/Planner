# Server Architecture - Complete Flow Diagrams

## 🏗️ **Scenario 1: System Startup & Initialization**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           SYSTEM STARTUP FLOW                               │
└─────────────────────────────────────────────────────────────────────────────┘

main() 
  │
  ├─ System system(8080)
  │   │
  │   ├─ ServerSocket serverSocket(8080)                    // 1️⃣ FIRST
  │   │   ├─ socket() + bind() + listen()
  │   │   ├─ receiveThread = thread(&receiveMessages)       // Background thread starts
  │   │   └─ sendThread = thread(&sendMessages)             // Background thread starts
  │   │
  │   ├─ MessageProcessor messageProcessor(this)            // 2️⃣ SECOND  
  │   │   ├─ system = this (pointer stored)
  │   │   └─ serverSocket = nullptr (will be set later)
  │   │
  │   └─ Setup direct connection:
  │       ├─ serverSocket.setOnConnectedCallback(...)       // Only for notifications
  │       ├─ serverSocket.setOnDisconnectedCallback(...)    // Only for notifications
  │       └─ messageProcessor.setServerSocket(&serverSocket) // DIRECT ACCESS!
  │
  ├─ system.registerHandler(DataHandler)
  │   └─ messageProcessor.registerHandler(DATA, handler)
  │
  ├─ system.registerHandler(DebugHandler) 
  │   └─ messageProcessor.registerHandler(DEBUG, handler)
  │
  ├─ system.registerHandler(CommandHandler)
  │   └─ messageProcessor.registerHandler(COMMAND, handler)
  │
  ├─ system.start()
  │   └─ messageProcessor.start()
  │       └─ processingThread = thread(&processLoop)        // Main processing thread
  │
  └─ while(!connected) { system.acceptConnection() }
      └─ serverSocket.accept() [BLOCKING until client connects]

Final State:
┌─────────────────┐                    ┌─────────────────┐
│   ServerSocket  │◀──DIRECT ACCESS───▶│ MessageProcessor│
│                 │                    │                 │
│ 🧵 receiveThread│                    │ 🧵 processThread│
│ 🧵 sendThread   │                    │                 │
│                 │                    │                 │
│ ⏳ waiting for  │                    │ 🔄 waiting on   │
│   client data   │                    │   condition var │
│ receiveQueue    │◀──getters for─────│ (event-driven)  │
│ receiveCondition│◀──direct access───│                 │
└─────────────────┘                    └─────────────────┘
```

---

## 🔌 **Scenario 2: Client Connection**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          CLIENT CONNECTION FLOW                             │
└─────────────────────────────────────────────────────────────────────────────┘

Client connects to port 8080
  │
  ▼
main() loop: system.acceptConnection()
  │
  ▼
ServerSocket::accept()
  │
  ├─ select() with 500ms timeout
  ├─ ::accept() - get clientSocket fd
  ├─ connected = true
  └─ onConnectedCallback() ──────────────▶ System::onClientConnected()
                                              │
                                              ▼
                                         "Client connected" printed

Final State - Client Connected:
┌─────────────────┐                    ┌─────────────────┐
│   ServerSocket  │◀──DIRECT ACCESS───▶│ MessageProcessor│
│                 │                    │                 │
│ 🧵 receiveThread│                    │ 🧵 processThread│
│ 🧵 sendThread   │                    │                 │
│                 │                    │                 │
│ ✅ clientSocket │                    │ 🔄 waiting on   │
│   established   │                    │   condition var │
│ receiveQueue    │◀──getters for─────│ (event-driven)  │
│ receiveCondition│◀──direct access───│                 │
└─────────────────┘                    └─────────────────┘
```

---

## 📥 **Scenario 3: Receiving Message (Complete Flow)**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         MESSAGE RECEIVING FLOW                              │
└─────────────────────────────────────────────────────────────────────────────┘

Client sends JSON message
  │
  ▼
ServerSocket::receiveMessages() [Background Thread]
  │
  ├─ recv() from clientSocket
  ├─ Parse JSON to MessageFrame
  ├─ lock(receiveMutex)
  ├─ receiveQueue.push(frame)
  ├─ unlock(receiveMutex)
  └─ receiveCondition.notify_one()              // Notify waiting threads!
  
  
MessageProcessor::processLoop() [Background Thread]
  │
  ├─ lock(inputMutex)                           // 🔒 Always first!
  │
  ├─ if (inputQueue.empty())                    // Need to get messages from ServerSocket
  │   │
  │   ├─ Check if serverSocket exists           // Safely handle nullptr
  │   │   └─ if (!serverSocket) wait_for(100ms) // Short timeout if no socket
  │   │
  │   └─ lock(serverSocket->getReceiveMutex())  // 🔒 Always second!
  │      │
  │      └─ serverSocket->getReceiveCondition().wait_for(lock, 100ms, [predicate]{ 
  │           // Wait until queue has messages OR processor stopped OR socket disconnected
  │           return !serverSocket || !running || !serverSocket->getReceiveQueue().empty(); 
  │         })                                  // Wait with predicate to avoid lost wakeups!
  │
  │      │
  │      └─ if (!running || !serverSocket) return // Exit if shutting down or no socket
  │
  │      │
  │      └─ Transfer messages from ServerSocket to local queue (with try-catch):
  │         while (!serverSocket->getReceiveQueue().empty() && running && serverSocket)
  │         {
  │             inputQueue.push(serverSocket->getReceiveQueue().front());
  │             serverSocket->getReceiveQueue().pop();
  │         }
  │
  ├─ Copy messages to local vector (outside locks)
  │   └─ localQueue.push_back(inputQueue.front())
  │
  ├─ unlock(inputMutex)                         // 🔓 Release during processing
  │
  └─ Process messages outside of locks:
      └─ for(frame : localQueue)
          ├─ messageIdOpt = assembler.addFragment(frame)
          └─ if(messageIdOpt)                   // Message complete
              ├─ payload = assembler.getAssembledMessage()
              └─ handleCompleteMessage()
                  │
                  └─ dispatcher.dispatch(messageId, payload, type, *system)
                      │
                      └─ handler->handle(messageId, payload, system)
                          │
                          └─ system.sendResponse(messageId, response, type)

DEADLOCK PREVENTION:
🔒 Lock Order: inputMutex → receiveMutex → sendMutex (ALWAYS!)
🔓 Release locks during message processing
⏰ Use condition variables with predicates to prevent lost wakeups
🛡️ All operations use try-catch for robustness
```

---

## 📤 **Scenario 4: Sending Message (Complete Flow)**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         MESSAGE SENDING FLOW                                │
└─────────────────────────────────────────────────────────────────────────────┘

Handler calls: system.sendResponse(messageId, payload, type)
  │
  ▼
System::sendResponse()
  │
  └─ messageProcessor.sendMessage(messageId, payload, type)
      │
      ├─ fragmenter.fragment(payload, type)         // Split large messages
      ├─ for(fragment : fragments)
      │   └─ fragment.header.messageId = messageId
      │
      └─ for(fragment : fragments)
          └─ serverSocket->sendMessage(fragment)    // DIRECT CALL!
              │
              ▼
          ServerSocket::sendMessage(fragment)
              │
              ├─ lock(sendMutex)
              ├─ sendQueue.push(fragment)
              ├─ unlock(sendMutex)
              └─ sendCondition.notify_all()         // Wake up sendThread


ServerSocket::sendMessages() [Background Thread]
  │
  ├─ lock(sendMutex)
  ├─ sendCondition.wait_for(...)                   // Wait for messages
  ├─ while(!sendQueue.empty())
  │   ├─ frame = sendQueue.front()
  │   ├─ sendQueue.pop()
  │   ├─ json = frame.toJson()
  │   └─ send(clientSocket, json.c_str(), ...)     // Send to client
  │
  └─ unlock(sendMutex)

Flow Summary:
Handler → System → MessageProcessor → ServerSocket (DIRECT!) → Client
```

---

## ⚡ **Scenario 5: Complete Request-Response Cycle**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    COMPLETE REQUEST-RESPONSE CYCLE                          │
└─────────────────────────────────────────────────────────────────────────────┘

📱 CLIENT                    🖥️  SERVER COMPONENTS                          
    │                          
    │ {"messageId":"cmd-ping-001",
    │  "type":1, "payload":"ping"}
    │                          
    ▼                          
┌─────────────────┐            ┌─────────────────┐
│   ServerSocket  │            │ MessageProcessor│
│                 │            │                 │
│ 🧵 receiveThread│            │ 🧵 processThread│
│ receiveQueue    │            │ inputQueue      │
│ receiveMutex    │            │ inputMutex      │
│ receiveCondition│            │                 │
│                 │            │ ├─ wait(predicate)
│                 │            │ ├─ transfer queue
│                 │            │ ├─ assemble     │
│                 │            │ ├─ dispatch     │
│                 │            │ └─ CommandHandler│
│                 │  sendMessage() │◀────────────│
│ 🧵 sendThread   │◀─────────────────│ (DIRECT!) │
│ sendQueue       │            │                 │
└─────────────────┘            └─────────────────┘
    ▲                          
    │ {"messageId":"cmd-ping-001",
    │  "type":1, "payload":"pong"}
    │                          
📱 CLIENT                    

Timeline:
1. Client sends "ping" → ServerSocket receives → adds to receiveQueue → notifies
2. MessageProcessor waits on receiveCondition with predicate → wakes up → transfers messages
3. MessageProcessor assembles → dispatches to CommandHandler
4. CommandHandler processes → calls system.sendResponse("pong")
5. System → MessageProcessor → serverSocket->sendMessage() → ServerSocket adds to sendQueue  
6. ServerSocket sends "pong" → Client receives response

🔄 The cycle can repeat infinitely with different message types and handlers

🎯 KEY: Event-driven architecture with direct communication!
```

---

## 🛑 **Scenario 6: System Shutdown**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           SYSTEM SHUTDOWN FLOW                              │
└─────────────────────────────────────────────────────────────────────────────┘

User presses Enter in main() or Client sends STOP command
  │
  ▼
CommandHandler::handleStopCommand       // For client-initiated shutdown
  │
  ├─ Send success response to client
  ├─ Sleep 500ms to ensure response delivery
  └─ Start detached thread for shutdown   // CRITICAL: Never stop from handler thread!
      │
      ▼ 

system.stop()                          // Called from main thread or detached thread
  │
  ├─ running = false
  ├─ serverSocket.disconnect()          // First disconnect client
  │   ├─ connected = false
  │   ├─ Close socket (shutdown + close)
  │   └─ Call onDisconnectedCallback()
  │
  ├─ messageProcessor.setServerSocket(nullptr) // Clear socket reference
  │
  └─ messageProcessor.stop()
      │
      ├─ running = false
      ├─ inputCondition.notify_all()          // Wake up processLoop
      ├─ serverSocket->getReceiveCondition().notify_all() // Wake up waits
      │
      ├─ Check if called from processThread    // CRITICAL: Prevent deadlock
      │   ├─ if (current_thread == processingThread)
      │   │   └─ processingThread.detach()     // NEVER join self - detach instead
      │   └─ else
      │       └─ processingThread.join()       // Safe to join from another thread
      │
      └─ serverSocket = nullptr               // Clear socket reference


~System() destructor
  │
  ├─ ~MessageProcessor()
  │   └─ stop() [already called]
  │
  └─ ~ServerSocket()
      │
      ├─ disconnect() if still connected
      ├─ running = false
      ├─ Notify all condition variables
      │
      ├─ receiveThread.join() with try-catch   // Safely join threads
      ├─ sendThread.join() with try-catch
      │
      └─ close(serverSocket)                   // Close network socket

Final State: All threads terminated, all resources cleaned up, no deadlocks
```

---

## 🎯 **Key Architecture Principles**

### **🔒 Mutex Lock Order (Deadlock Prevention)**
```
ALWAYS: inputMutex → receiveMutex → sendMutex
NEVER:  receiveMutex → inputMutex (DEADLOCK!)
NEVER:  sendMutex → inputMutex (DEADLOCK!)
NEVER:  sendMutex → receiveMutex (DEADLOCK!)
```

### **🔄 Thread Responsibilities**
- **Main Thread**: User interaction, system control
- **ServerSocket::receiveThread**: Network I/O (receiving)
- **ServerSocket::sendThread**: Network I/O (sending)  
- **MessageProcessor::processThread**: Message processing, handler dispatch
- **CommandHandler Detached Thread**: Safe system shutdown when triggered by client

### **📦 Message Flow Queues**
```
Network → receiveQueue → inputQueue → Handler → sendQueue → Network
         (ServerSocket)  (MessageProcessor)    (ServerSocket)
```

### **🎭 Component Roles**
- **System**: Coordinator, configuration, API interface (NOT a courier!)
- **ServerSocket**: Network I/O, connection management, exposes queues and synchronization primitives
- **MessageProcessor**: Message assembly, handler dispatch, DIRECT communication with ServerSocket
- **Handlers**: Business logic, response generation
- **CommandHandler**: Special role in system shutdown (using detached threads)

### **🚀 Direct Communication (NO Couriers!)**
```
┌─────────────────┐                    ┌─────────────────┐
│   ServerSocket  │◀──getReceiveQueue()─│ MessageProcessor│
│                 │◀──getReceiveMutex()─│                 │
│ receiveQueue    │◀──getReceiveCondition()│              │
│ sendQueue       │◀──sendMessage()────│                 │
└─────────────────┘                    └─────────────────┘

✅ MessageProcessor directly accesses ServerSocket queues and synchronization
✅ Event-driven with condition variables and predicates
✅ No busy waiting, no polling, efficient thread wakeup
❌ NO System courier, NO callback-based message transfer
```

### **🛡️ Robust Shutdown Sequence**
```
✅ Never call system.stop() from MessageProcessor thread (use detached thread)
✅ Check thread identity in stop() methods to avoid self-joining
✅ Clear null references before stopping components
✅ Use try-catch in all callbacks and critical operations
✅ Proper notification of all condition variables during shutdown
✅ Lock-free shutdown flag checking (atomic variables)
```

### **🔍 Error Handling & Resilience**
```
✅ All network operations are protected with try-catch
✅ Thread join operations are protected with try-catch
✅ Callbacks are wrapped in try-catch to prevent crashes
✅ Condition variables always used with predicates to prevent lost wakeups
✅ Timeouts on waits to prevent indefinite blocking
```

This architecture ensures **thread safety**, **performance**, **simplicity**, **robustness against shutdown races** and **maintainability**! 🚀
