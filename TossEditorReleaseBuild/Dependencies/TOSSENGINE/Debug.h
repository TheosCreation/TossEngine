#pragma once
#include "ImGuiLogger.h"

#ifdef TOSSENGINE_EXPORTS
#define TOSSENGINE_API __declspec(dllexport)  // When compiling the DLL
#else
#define TOSSENGINE_API __declspec(dllimport)  // When using the DLL
#endif

class TOSSENGINE_API Debug
{
public:
    // Log a regular message
    static void Log(const std::string& message) {
        PrintMessage(message, "Log");
    }
    // Log an error message and throw an exception
    static void LogError(const std::string& message) {
        PrintMessage(message, "Error");
        throw std::runtime_error("");
    }
    // Log a warning message
    static void LogWarning(const std::string& message) {
        PrintMessage(message, "Warning");
    }
    // Draw the ImGui console window
    static void DrawConsole(bool* p_open = nullptr) {
        s_imguiLogger.Draw("Console Log", p_open);
    }
private:
    // Helper function to format and print messages
    static void PrintMessage(const std::string& message, const std::string& type) {
        std::string fullMessage = "[" + type + "] " + message;
        // Output to standard console
        printf("%s\n", fullMessage.c_str());
        // Also add to the ImGui logger
        s_imguiLogger.AddLog("%s", fullMessage.c_str());
    }
private:
    // Static instance of the ImGui logger
    static ImGuiLogger s_imguiLogger;
};