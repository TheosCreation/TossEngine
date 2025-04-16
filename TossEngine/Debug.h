#pragma once
#include "ImGuiLogger.h"

class TOSSENGINE_API Debug
{
public:
    // Log a regular message
    static void Log(const std::string& message) {
        PrintMessage(message, "Log");
    }
    static void Log(const char* format, ...) {
        // Create a buffer to hold the formatted string.
        char buffer[1024];

        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        // Pass the formatted string to PrintMessage.
        PrintMessage(std::string(buffer), "Log");
    }

    // Log an error message and throw an exception
    static void LogError(const std::string& message, bool shouldThrow = true) {
        PrintMessage(message, "Error");
        if (shouldThrow)
        {
            throw std::runtime_error(message);
        }
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