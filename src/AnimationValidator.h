#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <regex>
#include <map>
#include <fstream>
#include <iostream>
#include <set>

class AnimationValidator 
{
public:
    struct ValidationResult 
    {
        std::vector<std::string> allCharacters;
        std::map<std::string, std::map<std::string, std::pair<bool, bool>>> characterAnimations;
        std::vector<std::string> incorrectlyNamedFiles;
        std::vector<std::pair<std::string, std::string>> frameRangeIssues;
        std::vector<std::string> missingAnimationFiles;
        std::vector<std::string> unsupportedFiles;
    };

    AnimationValidator();
    ValidationResult ValidateDirectory(const std::string& directoryPath);

private:
    std::vector<std::string> requiredAnimations = 
    {
        "Idle", "Run", "LightAttack", "HeavyAttack", "HeroPose"
    };

    std::regex namingPattern;

    bool IsSupportedExtension(const std::string& extension);
    bool ValidateNamingConvention(const std::string& filename);
    std::pair<int, int> GetFrameCount(const std::filesystem::path& filePath);
    std::string ExtractCharacterName(const std::string& filename);
    std::string ExtractActionName(const std::string& filename);
};