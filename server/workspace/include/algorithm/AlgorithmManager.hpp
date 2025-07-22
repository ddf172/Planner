#pragma once

#include "algorithm/AlgorithmInfo.hpp"
#include "extern/nlohmann/json.hpp"
#include <vector>
#include <string>
#include <map>

using json = nlohmann::json;

class AlgorithmManager {
public:
    AlgorithmManager();
    explicit AlgorithmManager(const std::string& algorithmDirectory);
    
    // Load algorithms from directory
    bool scanAlgorithms(const std::string& algorithmDirectory = "");
    
    // Get list of available algorithms
    std::vector<AlgorithmInfo> getAlgorithms() const;
    std::vector<std::string> getAlgorithmNames() const;
    
    // Get specific algorithm info
    AlgorithmInfo getAlgorithm(const std::string& name) const;
    bool hasAlgorithm(const std::string& name) const;
    
    // Validate algorithm configuration
    std::vector<std::string> validateConfiguration(const std::string& algorithmName, const json& config) const;
    
    // Get algorithm executable path
    std::string getAlgorithmPath(const std::string& name) const;
    
private:
    std::string algorithmsDirectory;
    std::map<std::string, AlgorithmInfo> algorithms;
    
    void loadAlgorithmFromDirectory(const std::string& algorithmDir);
    bool isValidAlgorithmDirectory(const std::string& path) const;
};
