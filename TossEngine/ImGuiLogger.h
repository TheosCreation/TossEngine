#pragma once
#include <vector>
#include <string>
#include <mutex>
#include <cstdarg>
#include <cstdio>
#include "imgui.h"
#include "TossEngineAPI.h"

class TOSSENGINE_API ImGuiLogger {
public:
    // Append a log message with formatting support
    void AddLog(const char* fmt, ...);

    // Draw the log window using ImGui
    void Draw(const char* title, bool* p_open = nullptr);

private:
    std::vector<std::string> m_Items;
    std::mutex m_mutex;
    bool m_ScrollToBottom = false;
};