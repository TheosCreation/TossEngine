/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : ImGuiLogger.h
Description : Provides an ImGui-based console logger for displaying messages inside the TossEngine editor.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <cstdarg>
#include <cstdio>
#include "imgui.h"
#include "TossEngineAPI.h"

/**
 * @class ImGuiLogger
 * @brief Thread-safe logger that stores messages and renders them using ImGui.
 *        Supports message collapsing and auto-scrolling.
 */
class TOSSENGINE_API ImGuiLogger {
public:
    /**
     * @brief Appends a log message with printf-style formatting.
     * @param fmt The format string.
     * @param ... Additional arguments for formatting.
     */
    void AddLog(const char* fmt, ...);

    /**
     * @brief Draws the log window in an ImGui context.
     * @param title The window title.
     * @param p_open Optional pointer to control window open/close state.
     */
    void Draw(const char* title, bool* p_open = nullptr);

private:
    /**
     * @brief Stores all log messages.
     */
    std::vector<std::string> m_Items;

    /**
     * @brief Mutex for ensuring thread-safe access to the log items.
     */
    std::mutex m_mutex;

    /**
     * @brief If true, scrolls the view to the bottom after new messages are added.
     */
    bool m_ScrollToBottom = false;

    /**
     * @brief If true, collapses duplicate messages when rendering.
     */
    bool m_Collapse = false;

    /**
     * @brief Flag to recompute collapsed items if necessary.
     */
    bool m_NeedsCollapseRecompute = true;

    /**
     * @brief Stores the collapsed view of log items when collapsing is enabled.
     */
    std::vector<std::string> m_CollapsedItems;
};