#include "AnimationValidator.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <algorithm>

using json = nlohmann::json;

AnimationValidator::AnimationValidator() 
{
    namingPattern = std::regex(R"(^([A-Za-z]+)_([A-Za-z]+)(_v\d{3})?\.(fbx|gltf|glb|json|anim|txt)$)");
}

std::string GetBaseName(const std::string& filename) 
{
    size_t versionPos = filename.find("_v");
    size_t dotPos = filename.find_last_of('.');

    if (versionPos != std::string::npos && dotPos != std::string::npos && versionPos < dotPos) 
    {
        return filename.substr(0, versionPos) + filename.substr(dotPos);
    }
    return filename;
}

AnimationValidator::ValidationResult AnimationValidator::ValidateDirectory(const std::string& directoryPath) 
{
    ValidationResult result;
    std::map<std::string, std::set<std::string>> characterAnimations;
    std::map<std::string, std::set<std::string>> jsonFiles;
    std::set<std::string> allCharacters;

    if (!std::filesystem::exists(directoryPath)) 
    {
        std::cerr << "Directory does not exist: " << directoryPath << std::endl;
        return result;
    }

    // First pass: Scan all files
    for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath)) 
    {
        if (!entry.is_regular_file()) continue;

        const auto& path = entry.path();
        std::string extension = path.extension().string();
        std::string filename = path.filename().string();

        if (!IsSupportedExtension(extension)) 
        {
            result.unsupportedFiles.push_back(filename);
            continue;
        }

        if (!ValidateNamingConvention(filename)) 
        {
            result.incorrectlyNamedFiles.push_back(filename);
            continue;
        }

        std::string character = ExtractCharacterName(filename);
        std::string action = ExtractActionName(filename);
        std::string baseName = GetBaseName(filename);

        allCharacters.insert(character);

        if (extension == ".json") 
        {
            jsonFiles[character].insert(action);
            continue;
        }

        characterAnimations[character].insert(action);

        // Get frame count from either the file itself or matching JSON
        auto [frameCount, duration] = GetFrameCount(path);
        if (frameCount > 0) 
        {
            if (frameCount < 10) 
            {
                result.frameRangeIssues.emplace_back(filename, "Too Short (" + std::to_string(frameCount) + " frames)");
            }
            else if (frameCount > 500) 
            {
                result.frameRangeIssues.emplace_back(filename, "Too Long (" + std::to_string(frameCount) + " frames)");
            }
        }
    }

    result.allCharacters = std::vector<std::string>(allCharacters.begin(), allCharacters.end());

    // Second pass: Check JSON files have corresponding animation files
    for (const auto& [character, jsonActions] : jsonFiles) 
    {
        for (const auto& action : jsonActions) 
        {
            bool hasAnimationFile = false;
            std::string searchPattern = character + "_" + action;

            // Search through all previously scanned files
            for (const auto& charAnim : characterAnimations[character]) 
            {
                if (charAnim == action) 
                {
                    hasAnimationFile = true;
                    break;
                }
            }

            if (!hasAnimationFile) 
            {
                result.missingAnimationFiles.push_back(character + "_" + action + ".json");
            }
        }
    }

    // Check for missing required animations per character
    for (const auto& character : allCharacters) 
    {
        std::map<std::string, std::pair<bool, bool>> charAnimations;
        for (const auto& reqAnim : requiredAnimations) 
        {
            bool hasFile = (characterAnimations[character].find(reqAnim) != characterAnimations[character].end());
            bool hasMetadata = (jsonFiles[character].find(reqAnim) != jsonFiles[character].end());
            charAnimations[reqAnim] = { hasFile, hasMetadata };
        }
        result.characterAnimations[character] = charAnimations;
    }

    return result;
}

std::pair<int, int> AnimationValidator::GetFrameCount(const std::filesystem::path& filePath) 
{
    if (filePath.extension() == ".json") 
    {
        try 
        {
            std::ifstream file(filePath);
            json data = json::parse(file);

            if (data.contains("frame_count")) 
            {
                int frames = data["frame_count"];
                float duration = data.value("duration_sec", 0.0f);
                return { frames, duration };
            }
        }
        catch (const std::exception& e) 
        {
            std::cerr << "Error parsing JSON file: " << filePath << " - " << e.what() << std::endl;
        }
    }
    else 
    {
        // Look for matching JSON file in the same directory
        std::string baseName = GetBaseName(filePath.stem().string());
        std::filesystem::path jsonPath = filePath.parent_path() / (baseName + ".json");

        if (std::filesystem::exists(jsonPath)) 
        {
            return GetFrameCount(jsonPath);
        }
    }
    return { -1, -1 };
}

bool AnimationValidator::IsSupportedExtension(const std::string& extension) 
{
    static const std::vector<std::string> supported = 
    {
        ".fbx", ".json", ".anim", ".txt", ".gltf", ".glb", ".usd", ".obj"
    };
    return std::find(supported.begin(), supported.end(), extension) != supported.end();
}

bool AnimationValidator::ValidateNamingConvention(const std::string& filename) 
{
    return std::regex_match(filename, namingPattern);
}

std::string AnimationValidator::ExtractCharacterName(const std::string& filename) 
{
    size_t underscorePos = filename.find('_');
    if (underscorePos != std::string::npos) 
    {
        return filename.substr(0, underscorePos);
    }
    return "";
}

std::string AnimationValidator::ExtractActionName(const std::string& filename) 
{
    size_t firstUnderscore = filename.find('_');
    if (firstUnderscore == std::string::npos) return "";

    size_t dotPos = filename.find('.');
    if (dotPos == std::string::npos) return "";

    size_t versionUnderscore = filename.find("_v", firstUnderscore + 1);
    if (versionUnderscore != std::string::npos && versionUnderscore < dotPos) 
    {
        return filename.substr(firstUnderscore + 1, versionUnderscore - firstUnderscore - 1);
    }

    return filename.substr(firstUnderscore + 1, dotPos - firstUnderscore - 1);
}