#include <iostream>
#include <memory>
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
    
    std::cout << "Server is running. Press Enter to stop..." << std::endl;
    std::cin.get(); // Czeka na naciśnięcie Enter
    
    system.stop();
    return 0;
}
