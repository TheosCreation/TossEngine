#include "ImGuiLogger.h"

void ImGuiLogger::Draw(const char* title, bool* p_open)
{
    if (!ImGui::Begin(title, p_open))
    {
        ImGui::End();
        return;
    }

    // Button to clear the log
    if (ImGui::Button("Clear"))
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_Items.clear();
        m_CollapsedItems.clear();
    }

    // Toggle button for collapse mode
    ImGui::SameLine();
    if (ImGui::Button(m_Collapse ? "Uncollapse" : "Collapse"))
    {
        m_Collapse = !m_Collapse;
    }

    // Begin child region for scrolling logs
    if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto renderItem = [&](const std::string& msg, int idx)
            {
                // Determine color: default, warning=yellow, error=red
                ImVec4 col = ImGui::GetStyleColorVec4(ImGuiCol_Text);
                if (msg.rfind("[Error]", 0) == 0)
                    col = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                else if (msg.rfind("[Warning]", 0) == 0)
                    col = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

                ImGui::PushStyleColor(ImGuiCol_Text, col);
                std::string label = msg + "##" + std::to_string(idx);
                if (ImGui::Selectable(label.c_str()))
                    ImGui::SetClipboardText(msg.c_str());
                ImGui::PopStyleColor();
            };

        if (m_Collapse)
        {
            // Recompute collapsed log items when needed.
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

            // Output from collapsed log items

            for (size_t i = 0; i < m_CollapsedItems.size(); i++)
                renderItem(m_CollapsedItems[i], (int)i);
        }
        else
        {
            for (size_t i = 0; i < m_Items.size(); i++)
                renderItem(m_Items[i], (int)i);
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
