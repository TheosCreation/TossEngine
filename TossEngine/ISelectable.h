#pragma once
#include "Utils.h"
#include "ResourceManager.h"

class GameObject;

class TOSSENGINE_API ISelectable
{
public:
    virtual void OnInspectorGUI() {}
    virtual void OnSelect() {}
    virtual void OnDeSelect() {}
    virtual bool Delete(bool deleteSelf = true) { return false;  }

    static void GameObjectAssignableField(GameObjectPtr& _gameObject, const string& fieldName);

    template<typename T>
    bool ResourceAssignableField(std::shared_ptr<T>& resourcePtr, const std::string& fieldName)
    {
        static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");
        ResourceManager& rm = ResourceManager::GetInstance();

        bool changed = false;
        std::string label = resourcePtr
            ? fieldName + ": " + resourcePtr->getUniqueID()
            : fieldName + ": None";

        if (ImGui::BeginTable((fieldName + "Table").c_str(), 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (ImGui::Selectable(label.c_str()))
            {
                rm.SetSelectedResource(resourcePtr);
            }

            ImGui::TableSetColumnIndex(1);
            if (ImGui::SmallButton("+"))
                ImGui::OpenPopup((fieldName + "Dropdown").c_str());

            if (ImGui::BeginPopup((fieldName + "Dropdown").c_str()))
            {
                // “None” option
                if (ImGui::Selectable("None", resourcePtr == nullptr))
                {
                    if (resourcePtr) {
                        resourcePtr.reset();
                        rm.SetSelectedResource(nullptr);
                        changed = true;
                    }
                }

                // all resources of type T
                auto& all = rm.GetAllResources();
                for (auto& [id, res] : all)
                {
                    if (auto casted = std::dynamic_pointer_cast<T>(res))
                    {
                        bool isSel = (resourcePtr && resourcePtr->getUniqueID() == id);
                        if (ImGui::Selectable(id.c_str(), isSel))
                        {
                            if (!isSel)
                            {
                                resourcePtr = casted;
                                rm.SetSelectedResource(resourcePtr);
                                changed = true;
                            }
                        }
                    }
                }
                ImGui::EndPopup();
            }

            ImGui::EndTable();
        }

        return changed;
    }

    template<typename T>
    bool ResourceDropdownField(std::shared_ptr<T>& resourcePtr, const std::string& fieldName)
    {
        static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

        ResourceManager& resourceManager = ResourceManager::GetInstance();
        
        std::string previewLabel = (resourcePtr) ? resourcePtr->getUniqueID() : "None";
        
        if (ImGui::BeginCombo(fieldName.c_str(), previewLabel.c_str()))
        {
            // Option: None
            bool noneSelected = (resourcePtr == nullptr);
            if (ImGui::Selectable("None", noneSelected))
            {
                resourcePtr = nullptr;
                resourceManager.SetSelectedResource(nullptr);
            }
        
            // List all resources of the correct type
            auto& resources = resourceManager.GetAllResources();
            for (const auto& [id, resource] : resources)
            {
                // Only include resources that match the requested type
                auto casted = std::dynamic_pointer_cast<T>(resource);
                if (casted)
                {
                    bool isSelected = (resourcePtr && resourcePtr->getUniqueID() == id);
                    if (ImGui::Selectable(id.c_str(), isSelected))
                    {
                        resourcePtr = casted;
                        resourceManager.SetSelectedResource(resourcePtr);
                    }
        
                    // Highlight the selected item
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
            }
        
            ImGui::EndCombo();
            return true;
        }
        return false;
    }
    static void FileSelectionTableField(const char* tableID, const std::vector<std::string>& rowLabels, std::vector<std::string>& filePaths, const char* filter);
    static bool FloatSliderField(const string& name, float& value, float speed = 1.0f, float min = 0.0f, float max = 0.0f);
    static bool IntSliderField(const string& name, int& value, int speed = 1, int min = 0, int max = 0);

    static bool LayerDropdownField(const string& name, vector<string>& value);

    static bool BoolCheckboxField(const string& name, bool& value);
};