#include "AnimationValidator.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <filesystem>
#include <thread>
#include <cstring>

static AnimationValidator validator;
static AnimationValidator::ValidationResult validationResult;
static bool isRunningValidation = false;
static bool showValidationResults = false;
static char directoryPath[256] = "";
static std::string statusMessage;

void RunValidation()
{
    isRunningValidation = true;
    statusMessage = "Validating directory: " + std::string(directoryPath);

    validationResult = validator.ValidateDirectory(directoryPath);

    isRunningValidation = false;
    showValidationResults = true;
    statusMessage = "Validation complete!";
}

int main()
{
    if (!glfwInit())
    {
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(1000, 800, "Animation Validator", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Animation File Validator", nullptr, ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Directory to validate:");
        ImGui::InputText("##directory", directoryPath, IM_ARRAYSIZE(directoryPath));

        if (ImGui::Button("Browse..."))
        {
            // Button does nothing - can be implemented later
        }
        ImGui::SameLine();

        if (ImGui::Button("Run Validation") && !isRunningValidation)
        {
            if (std::filesystem::exists(directoryPath))
            {
                std::thread validationThread(RunValidation);
                validationThread.detach();
            }
            else
            {
                statusMessage = "Error: Directory does not exist!";
            }
        }

        if (isRunningValidation)
        {
            ImGui::SameLine();
            ImGui::Text("Validating...");
        }

        ImGui::Spacing();
        ImGui::Separator();

        if (!statusMessage.empty())
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", statusMessage.c_str());
        }

        if (showValidationResults)
        {
            ImGui::Spacing();

            // Character info
            if (!validationResult.allCharacters.empty())
            {
                ImGui::Text("Characters Found: ");
                ImGui::SameLine();
                for (size_t i = 0; i < validationResult.allCharacters.size(); ++i)
                {
                    if (i > 0) ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0, 1, 1, 1), "%s%s",
                        (i > 0 ? ", " : ""),
                        validationResult.allCharacters[i].c_str());
                }
                ImGui::Spacing();

                for (const auto& character : validationResult.allCharacters)
                {
                    if (ImGui::CollapsingHeader(character.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::Indent();

                        ImGui::Text("Required Animations:");
                        const auto& animStatus = validationResult.characterAnimations.at(character);
                        for (const auto& [anim, present] : animStatus)
                        {
                            ImGui::Bullet();
                            ImGui::TextColored(
                                present.first ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1),
                                "%s: %s%s",
                                anim.c_str(),
                                present.first ? "PRESENT" : "MISSING",
                                present.second ? " (metadata present)" : " (metadata absent)"
                            );
                        }
                        ImGui::Unindent();
                    }
                }
            }

            // Global issues
            if (!validationResult.incorrectlyNamedFiles.empty())
            {
                if (ImGui::CollapsingHeader("Incorrectly Named Files"))
                {
                    for (const auto& file : validationResult.incorrectlyNamedFiles)
                    {
                        ImGui::Bullet();
                        ImGui::Text("%s", file.c_str());
                    }
                }
            }

            if (!validationResult.frameRangeIssues.empty())
            {
                if (ImGui::CollapsingHeader("Frame Range Issues"))
                {
                    for (const auto& [file, issue] : validationResult.frameRangeIssues)
                    {
                        ImGui::Bullet();
                        ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "%s: %s", file.c_str(), issue.c_str());
                    }
                }
            }

            if (!validationResult.missingAnimationFiles.empty())
            {
                if (ImGui::CollapsingHeader("JSON Without Animation Files"))
                {
                    for (const auto& file : validationResult.missingAnimationFiles)
                    {
                        ImGui::Bullet();
                        ImGui::Text("%s", file.c_str());
                    }
                }
            }

            if (!validationResult.unsupportedFiles.empty())
            {
                if (ImGui::CollapsingHeader("Unsupported Files"))
                {
                    for (const auto& file : validationResult.unsupportedFiles)
                    {
                        ImGui::Bullet();
                        ImGui::TextDisabled("%s", file.c_str());
                    }
                }
            }
        }

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
