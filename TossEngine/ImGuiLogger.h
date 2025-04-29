#pragma once
#include <vector>
#include <string>
#include <mutex>
#include <format>
#include <cstdarg>
#include <cstdio>
#include "imgui.h"
#include "TossEngineAPI.h"

class TOSSENGINE_API ImGuiLogger {
public:
    void AddLog(std::string& message)
    {
        m_Items.emplace_back(message);
        m_ScrollToBottom = true;
        m_NeedsCollapseRecompute = true;
    }

    template<typename... Args>
    void AddLogF(const char* format, Args&&... args)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        char buf[1024];
        snprintf(buf, sizeof(buf), format, std::forward<Args>(args)...);
        m_Items.emplace_back(std::string(buf));
        m_ScrollToBottom = true;
        m_NeedsCollapseRecompute = true;
    }

    // Draw the log window using ImGui
    void Draw(const char* title, bool* p_open = nullptr);

private:
    std::vector<std::string> m_Items;
    std::mutex m_mutex;
    bool m_ScrollToBottom = false;
    bool m_Collapse = false;
    bool m_NeedsCollapseRecompute = true;
    std::vector<std::string> m_CollapsedItems;
};