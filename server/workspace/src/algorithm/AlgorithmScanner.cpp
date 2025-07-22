#include "algorithm/AlgorithmScanner.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>

AlgorithmScanner::AlgorithmScanner() {
    // Default algorithms directory relative to workspace
    algorithmsDirectory = "../algorithms";
    scanAlgorithms();
}

AlgorithmScanner::AlgorithmScanner(const std::string& algorithmDirectory) 
    : algorithmsDirectory(algorithmDirectory) {
    scanAlgorithms();
}

bool AlgorithmScanner::scanAlgorithms(const std::string& algorithmDirectory) {
    if (!algorithmDirectory.empty()) {
        algorithmsDirectory = algorithmDirectory;
    }
    
    algorithms.clear();
    
    if (!std::filesystem::exists(algorithmsDirectory)) {
        std::cerr << "Algorithm directory does not exist: " << algorithmsDirectory << std::endl;
        return false;
    }
    
    std::cout << "Scanning algorithms in: " << algorithmsDirectory << std::endl;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(algorithmsDirectory)) {
            if (entry.is_directory()) {
                loadAlgorithmFromDirectory(entry.path().string());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning algorithms directory: " << e.what() << std::endl;
        return false;
    }
    
    std::cout << "Found " << algorithms.size() << " algorithms" << std::endl;
    return true;
}

std::vector<AlgorithmInfo> AlgorithmScanner::getAlgorithms() const {
    std::vector<AlgorithmInfo> result;
    result.reserve(algorithms.size());
    
    for (const auto& [name, info] : algorithms) {
        result.push_back(info);
    }
    
    return result;
}

std::vector<std::string> AlgorithmScanner::getAlgorithmNames() const {
    std::vector<std::string> result;
    result.reserve(algorithms.size());
    
    for (const auto& [name, info] : algorithms) {
        result.push_back(name);
    }
    
    return result;
}

AlgorithmInfo AlgorithmScanner::getAlgorithm(const std::string& name) const {
    auto it = algorithms.find(name);
    if (it != algorithms.end()) {
        return it->second;
    }
    return AlgorithmInfo(); // Return empty/invalid info
}

bool AlgorithmScanner::hasAlgorithm(const std::string& name) const {
    return algorithms.find(name) != algorithms.end();
}

std::vector<std::string> AlgorithmScanner::validateConfiguration(const std::string& algorithmName, const json& config) const {
    auto it = algorithms.find(algorithmName);
    if (it == algorithms.end()) {
        return {"Algorithm '" + algorithmName + "' not found"};
    }
    
    return it->second.validateParameters(config);
}

std::string AlgorithmScanner::getAlgorithmPath(const std::string& name) const {
    auto it = algorithms.find(name);
    if (it != algorithms.end()) {
        return it->second.path;
    }
    return "";
}

void AlgorithmScanner::loadAlgorithmFromDirectory(const std::string& algorithmDir) {
    if (!isValidAlgorithmDirectory(algorithmDir)) {
        return;
    }
    
    std::string infoFile = algorithmDir + "/info.json";
    
    // Try to load info.json
    if (std::filesystem::exists(infoFile)) {
        AlgorithmInfo info = AlgorithmInfo::fromInfoFile(infoFile);
        if (info.isValid()) {
            algorithms[info.name] = info;
            std::cout << "Loaded algorithm: " << info.name << " (" << info.displayName << ")" << std::endl;
        } else {
            std::cerr << "Invalid algorithm info in: " << infoFile << std::endl;
        }
    } else {
        // Create minimal algorithm info from directory name
        std::string dirName = std::filesystem::path(algorithmDir).filename().string();
        AlgorithmInfo info;
        info.name = dirName;
        info.displayName = dirName;
        info.description = "Algorithm: " + dirName;
        info.path = algorithmDir;
        info.version = "1.0.0";
        info.supportsProgress = false;
        
        if (info.isValid()) {
            algorithms[info.name] = info;
            std::cout << "Created minimal info for algorithm: " << info.name << std::endl;
        }
    }
}

bool AlgorithmScanner::isValidAlgorithmDirectory(const std::string& path) const {
    // Check if directory contains executable named "algorithm"
    std::string executablePath = path + "/algorithm";
    return std::filesystem::exists(executablePath) && 
           std::filesystem::is_regular_file(executablePath);
}
