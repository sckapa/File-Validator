#include "AnimationValidator.h"
#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <filesystem>

void GenerateReport(const AnimationValidator::ValidationResult& result, const std::string& reportPath)
{
    std::ofstream reportFile(reportPath);

    if (!reportFile.is_open())
    {
        std::cerr << "Failed to create report file: " << reportPath << std::endl;
        return;
    }

    reportFile << "=== Animation Validation Report ===\n\n";

    if (!result.allCharacters.empty())
    {
        reportFile << "Characters Found: ";
        for (size_t i = 0; i < result.allCharacters.size(); ++i)
        {
            if (i > 0)
            {
                reportFile << ", ";
            }
            reportFile << result.allCharacters[i];
        }
        reportFile << "\n\n";

        for (const auto& character : result.allCharacters)
        {
            reportFile << "=== " << character << " ===\n";
            reportFile << "Required Animations:\n";

            const auto& animStatus = result.characterAnimations.at(character);
            for (const auto& [anim, present] : animStatus)
            {
                reportFile << " - " << anim << ": "
                    << (present.first ? "PRESENT" : "MISSING")
                    << (present.second ? " (metadata present)" : " (metadata absent)") << "\n";
            }
            reportFile << "\n";
        }
    }

    if (!result.incorrectlyNamedFiles.empty())
    {
        reportFile << "Incorrectly Named Files:\n";
        for (const auto& file : result.incorrectlyNamedFiles)
        {
            reportFile << " - " << file << "\n";
        }
        reportFile << "\n";
    }

    if (!result.frameRangeIssues.empty())
    {
        reportFile << "Frame Range Issues:\n";
        for (const auto& [file, issue] : result.frameRangeIssues)
        {
            reportFile << " - " << file << ": " << issue << "\n";
        }
        reportFile << "\n";
    }

    if (!result.missingAnimationFiles.empty())
    {
        reportFile << "Warning: JSON files without animation files:\n";
        for (const auto& file : result.missingAnimationFiles)
        {
            reportFile << " - " << file << "\n";
        }
        reportFile << "\n";
    }

    if (!result.unsupportedFiles.empty())
    {
        reportFile << "Unsupported Files:\n";
        for (const auto& file : result.unsupportedFiles)
        {
            reportFile << " - " << file << "\n";
        }
    }

    reportFile << "\n=== End of Report ===\n";
    reportFile.close();

    std::cout << "Report generated at: " << reportPath << std::endl;
}

int main()
{
    AnimationValidator validator;
    std::string directoryPath;
    std::string reportPath = "validation_report.txt";

    std::cout << "=== Animation File Validator ===\n\n";

    std::cout << "Enter path to animation files directory: ";
    std::getline(std::cin, directoryPath);

    // Normalize the path and resolve any relative paths
    try
    {
        directoryPath = std::filesystem::canonical(directoryPath).string();
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "Error resolving path: " << e.what() << std::endl;
        return 1;
    }

    while (!std::filesystem::exists(directoryPath))
    {
        std::cout << "\nError: Directory does not exist.\n";
        std::cout << "Enter a valid path (or 'q' to quit): ";
        std::getline(std::cin, directoryPath);

        if (directoryPath == "q" || directoryPath == "Q")
        {
            return 0;
        }

        try
        {
            directoryPath = std::filesystem::canonical(directoryPath).string();
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            std::cerr << "Error resolving path: " << e.what() << std::endl;
            continue;
        }
    }

    std::cout << "\nValidating animation files in: " << directoryPath << "\n";
    std::cout << "This may take a moment...\n\n";

    auto result = validator.ValidateDirectory(directoryPath);
    GenerateReport(result, reportPath);

    return 0;
}
