workspace "AnimationValidator"
    configurations { "Debug", "Release" }
    platforms { "x64" }
    startproject "AnimationValidatorCLI"
    
    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "On"

    filter { "configurations:Release" }
        defines { "NDEBUG" }
        optimize "On"

    filter { "action:vs*" }
        defines { "_CRT_SECURE_NO_WARNINGS" }

    filter {}

project "AnimationValidatorCLI"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "bin-int/%{cfg.buildcfg}/%{prj.name}"

    files 
    {
        "src/AnimationValidator.h",
        "src/AnimationValidator.cpp",
        "src/main.cpp"
    }

    includedirs 
    {
        "src",
        "external/nlohmann_json/include"
    }

    filter { "system:windows" }
        systemversion "latest"
        staticruntime "On"

    filter {}

project "AnimationValidatorGUI"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "bin-int/%{cfg.buildcfg}/%{prj.name}"

    files 
    {
        "src/AnimationValidator.h",
        "src/AnimationValidator.cpp",
        "src/gui_main.cpp",
        "external/imgui/*.h",
        "external/imgui/*.cpp",
        "external/imgui/backends/imgui_impl_glfw.h",
        "external/imgui/backends/imgui_impl_glfw.cpp",
        "external/imgui/backends/imgui_impl_opengl3.h",
        "external/imgui/backends/imgui_impl_opengl3.cpp"
    }

    includedirs 
    {
        "src",
        "external/nlohmann_json/include",
        "external/imgui",
        "external/imgui/backends",
        "external/glfw/include"
    }

    libdirs
    {
        "lib"
    }

    links 
    {
        "glfw3_mt",
        "opengl32"
    }

    filter { "system:windows" }
        systemversion "latest"
        staticruntime "On"
        defines { "IMGUI_IMPL_OPENGL_LOADER_GL3W" }

    filter {}