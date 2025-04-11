#include "ImGuiLogger.h"

void ImGuiLogger::AddLog(const char* fmt, ...)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    m_Items.push_back(std::string(buf));

    // Automatically scroll to bottom on new log
    m_ScrollToBottom = true;
    m_NeedsCollapseRecompute = true;
}

void ImGuiLogger::Draw(const char* title, bool* p_open)
{

    if (!ImGui::Begin(title, p_open))
    {
        ImGui::End();
        return;
    }

    // Add a button to clear the log
    if (ImGui::Button("Clear"))
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_Items.clear();
        m_CollapsedItems.clear();
    }

    // Add a toggle button for collapse mode
    ImGui::SameLine();
    if (ImGui::Button(m_Collapse ? "Uncollapse" : "Collapse"))
    {
        m_Collapse = !m_Collapse;
    }

    // Begin child region for scrolling logs
    if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_Collapse)
        {
            // Recompute collapse list only when needed.
            if (m_NeedsCollapseRecompute)
            {
                m_CollapsedItems.clear();
                std::string previousLine;
                int count = 0;
                for (const auto& item : m_Items)
                {
                    if (item == previousLine)
                    {
                        count++;
                    }
                    else
                    {
                        if (!previousLine.empty())
                        {
                            if (count > 1)
                            {
                                m_CollapsedItems.push_back(previousLine + " (" + std::to_string(count) + ")");
                            }
                            else
                            {
                                m_CollapsedItems.push_back(previousLine);
                            }
                        }
                        previousLine = item;
                        count = 1;
                    }
                }
                if (!previousLine.empty())
                {
                    if (count > 1)
                    {
                        m_CollapsedItems.push_back(previousLine + " (" + std::to_string(count) + ")");
                    }
                    else
                    {
                        m_CollapsedItems.push_back(previousLine);
                    }
                }
                m_NeedsCollapseRecompute = false;
            }

            // Then output from the cached collapsed log.
            for (const auto& collapsedLine : m_CollapsedItems)
            {
                ImGui::TextUnformatted(collapsedLine.c_str());
            }
        }
        else
        {
            // Output normally when not collapsed.
            for (const auto& item : m_Items)
            {
                ImGui::TextUnformatted(item.c_str());
            }
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