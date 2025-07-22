#include "algorithm/AlgorithmInfo.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

AlgorithmInfo AlgorithmInfo::fromInfoFile(const std::string& infoPath) {
    AlgorithmInfo info;
    
    try {
        std::ifstream file(infoPath);
        if (!file.is_open()) {
            std::cerr << "Cannot open info file: " << infoPath << std::endl;
            return info;
        }
        
        json j;
        file >> j;
        
        info.name = j.value("name", "");
        info.displayName = j.value("displayName", "");
        info.version = j.value("version", "");
        info.description = j.value("description", "");
        info.author = j.value("author", "");
        info.type = j.value("type", "");
        info.supportsProgress = j.value("supportsProgress", false);
        
        if (j.contains("parameters")) {
            info.parameters = j["parameters"];
        }
        
        // Set path to algorithm directory
        info.path = std::filesystem::path(infoPath).parent_path();
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing info file " << infoPath << ": " << e.what() << std::endl;
    }
    
    return info;
}

bool AlgorithmInfo::isValid() const {
    return !name.empty() && 
           !displayName.empty() && 
           !path.empty() &&
           std::filesystem::exists(path + "/algorithm");
}

std::vector<std::string> AlgorithmInfo::validateParameters(const json& config) const {
    std::vector<std::string> errors;
    
    if (!parameters.is_object()) {
        return errors; // No parameters defined, all valid
    }
    
    for (const auto& [paramName, paramConfig] : parameters.items()) {
        if (!config.contains(paramName)) {
            continue; // Parameter not provided, will use default
        }
        
        if (!paramConfig.contains("type")) {
            continue; // No type validation possible
        }
        
        std::string type = paramConfig["type"];
        const auto& value = config[paramName];
        
        if (type == "int" && !value.is_number_integer()) {
            errors.push_back("Parameter '" + paramName + "' must be an integer");
        } else if (type == "float" && !value.is_number()) {
            errors.push_back("Parameter '" + paramName + "' must be a number");
        } else if (type == "string" && !value.is_string()) {
            errors.push_back("Parameter '" + paramName + "' must be a string");
        }
        
        // Check min/max bounds for numeric values
        if (value.is_number() && paramConfig.contains("min")) {
            if (value.get<double>() < paramConfig["min"].get<double>()) {
                errors.push_back("Parameter '" + paramName + "' is below minimum value");
            }
        }
        
        if (value.is_number() && paramConfig.contains("max")) {
            if (value.get<double>() > paramConfig["max"].get<double>()) {
                errors.push_back("Parameter '" + paramName + "' is above maximum value");
            }
        }
    }
    
    return errors;
}
