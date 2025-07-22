#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include "../extern/nlohmann/json.hpp"

using json = nlohmann::json;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " input.json output.json [config.json] [progress.json]" << std::endl;
        return 2;
    }
    
    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    std::string configFile = (argc > 3) ? argv[3] : "";
    std::string progressFile = (argc > 4) ? argv[4] : "";
    
    try {
        // Read input data
        std::ifstream input(inputFile);
        json inputData;
        input >> inputData;
        input.close();
        
        // Read config
        json config;
        int delay = 5; // default
        if (!configFile.empty()) {
            std::ifstream configStream(configFile);
            configStream >> config;
            configStream.close();
            
            if (config.contains("delay")) {
                delay = config["delay"];
            }
        }
        
        std::cout << "Simple test algorithm starting with " << delay << " seconds delay" << std::endl;
        
        // Simulate algorithm execution with progress updates
        for (int i = 0; i <= delay; ++i) {
            float progress = (float)i / delay;
            
            // Update progress file if provided
            if (!progressFile.empty()) {
                json progressData = {
                    {"progress", progress},
                    {"status", (i < delay) ? "optimizing" : "finalizing"},
                    {"currentBest", {
                        {"qualityScore", 0.5 + progress * 0.3},
                        {"constraintViolations", (int)(10 * (1.0 - progress))}
                    }},
                    {"algorithmSpecific", {
                        {"iteration", i},
                        {"totalIterations", delay}
                    }}
                };
                
                std::ofstream progressStream(progressFile);
                progressStream << progressData.dump(2);
                progressStream.close();
            }
            
            if (i < delay) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        
        // Generate simple result
        json result = {
            {"status", "success"},
            {"schedule", {
                {"events", json::array()}
            }},
            {"metadata", {
                {"algorithmName", "simple_test"},
                {"version", "1.0"},
                {"executionTimeMs", delay * 1000},
                {"qualityScore", 0.8},
                {"constraintViolations", 0}
            }}
        };
        
        // Create a simple test event if we have data
        if (inputData.contains("timeBlocks") && !inputData["timeBlocks"].empty() &&
            inputData.contains("subjects") && !inputData["subjects"].empty() &&
            inputData.contains("groups") && !inputData["groups"].empty() &&
            inputData.contains("rooms") && !inputData["rooms"].empty() &&
            inputData.contains("teachers") && !inputData["teachers"].empty()) {
            
            json event = {
                {"id", 1},
                {"subjectId", inputData["subjects"][0]["id"]},
                {"groupId", inputData["groups"][0]["id"]},
                {"roomId", inputData["rooms"][0]["id"]},
                {"teacherId", inputData["teachers"][0]["id"]},
                {"timeBlockId", inputData["timeBlocks"][0]["id"]}
            };
            
            result["schedule"]["events"].push_back(event);
        }
        
        // Write output
        std::ofstream output(outputFile);
        output << result.dump(2);
        output.close();
        
        std::cout << "Simple test algorithm completed successfully" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        
        // Write error result
        json errorResult = {
            {"status", "error"},
            {"errorMessage", e.what()}
        };
        
        std::ofstream output(outputFile);
        output << errorResult.dump(2);
        output.close();
        
        return 3;
    }
}
