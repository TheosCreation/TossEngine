#pragma once
#include <vector>
#include <string>
#include <mutex>
#include <cstdarg>
#include <cstdio>
#include "imgui.h"

class ImGuiLogger {
public:
    // Append a log message with formatting support
    void AddLog(const char* fmt, ...) {
        std::lock_guard<std::mutex> lock(m_mutex);
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        m_Items.push_back(std::string(buf));
        // Automatically scroll to bottom on new log
        m_ScrollToBottom = true;
    }

    // Draw the log window using ImGui
    void Draw(const char* title, bool* p_open = nullptr) {
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (const auto& item : m_Items)
            {
                ImGui::TextUnformatted(item.c_str());
            }
            if (m_ScrollToBottom)
            {
                ImGui::SetScrollHereY(1.0f);
                m_ScrollToBottom = false;
            }
        }
        ImGui::EndChild();
        ImGui::End();
    }

private:
    std::vector<std::string> m_Items;
    std::mutex m_mutex;
    bool m_ScrollToBottom = false;
};