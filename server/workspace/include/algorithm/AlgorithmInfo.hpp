#pragma once

#include <string>
#include <vector>
#include "extern/nlohmann/json.hpp"

using json = nlohmann::json;

struct AlgorithmInfo {
    std::string name;
    std::string displayName;
    std::string path;
    std::string version;
    std::string description;
    std::string author;
    std::string type;
    bool supportsProgress;
    json parameters;
    
    static AlgorithmInfo fromInfoFile(const std::string& infoPath);
    bool isValid() const;
    std::vector<std::string> validateParameters(const json& config) const;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(AlgorithmInfo, name, displayName, version, description, author, type, supportsProgress, parameters)
};
