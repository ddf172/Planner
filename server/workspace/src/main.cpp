#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "core/System.hpp"
#include "commands/handlers/DataHandler.hpp"
#include "commands/handlers/DebugHandler.hpp"
#include "commands/handlers/CommandHandler.hpp"

int main() {
    System system(8080);  // Podajemy port
    
    // Register message handlers
    std::cout << "Registering message handlers..." << std::endl;
    system.registerHandler(std::make_unique<DataHandler>());
    system.registerHandler(std::make_unique<DebugHandler>());
    system.registerHandler(std::make_unique<CommandHandler>());
    
    system.start();
    
    std::cout << "Server started. Waiting for client connection..." << std::endl;
    
    // Wait for client connection in a loop
    while (true) {
        if (system.acceptConnection()) {
            std::cout << "Client connected successfully!" << std::endl;
            break;
        }
        // Small delay to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get(); // Czeka na naciśnięcie Enter
    return 0;
}
