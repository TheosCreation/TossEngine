/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : Debug.h
Description : Provides static methods for logging messages, errors, and warnings
              to both the standard console and the in-editor ImGui console.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "ImGuiLogger.h"

/**
 * @class Debug
 * @brief Provides utility methods for logging debug messages, warnings, and errors
 *        to both the console and an ImGui-based console window.
 */
class TOSSENGINE_API Debug
{
public:
    /**
     * @brief Logs a regular message to the console and ImGui logger.
     * @param message The message to log.
     */
    static void Log(const std::string& message) {
        PrintMessage(message, "Log");
    }

    /**
     * @brief Logs a formatted regular message to the console and ImGui logger.
     *        Accepts printf-style formatting.
     * @param format The format string.
     * @param ... Additional arguments for formatting.
     */
    static void Log(const char* format, ...) {
        // Create a buffer to hold the formatted string
        char buffer[1024];

        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        // Pass the formatted string to PrintMessage
        PrintMessage(std::string(buffer), "Log");
    }

    /**
     * @brief Logs an error message to the console and ImGui logger.
     *        Optionally throws a runtime exception.
     * @param message The error message to log.
     * @param shouldThrow If true, throws a std::runtime_error after logging.
     */
    static void LogError(const std::string& message, bool shouldThrow = true) {
        PrintMessage(message, "Error");
        if (shouldThrow) {
            throw std::runtime_error(message);
        }
    }

    /**
     * @brief Logs a warning message to the console and ImGui logger.
     * @param message The warning message to log.
     */
    static void LogWarning(const std::string& message) {
        PrintMessage(message, "Warning");
    }

    /**
     * @brief Draws the in-editor ImGui console window.
     * @param p_open Optional pointer to control window open/close status.
     */
    static void DrawConsole(bool* p_open = nullptr) {
        s_imguiLogger.Draw("Console Log", p_open);
    }

private:
    /**
     * @brief Helper function to format and print messages with a specific type prefix.
     * @param message The message content.
     * @param type The type of the message ("Log", "Warning", "Error", etc.).
     */
    static void PrintMessage(const std::string& message, const std::string& type) {
        std::string fullMessage = "[" + type + "] " + message;
        // Output to the standard console
        printf("%s\n", fullMessage.c_str());
        // Add the message to the ImGui logger
        s_imguiLogger.AddLog("%s", fullMessage.c_str());
    }

private:
    /**
     * @brief Static instance of the ImGui logger used for displaying messages
     *        inside the editor's console window.
     */
    static ImGuiLogger s_imguiLogger;
};