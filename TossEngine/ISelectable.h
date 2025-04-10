#pragma once
#include "Utils.h"
#include "ResourceManager.h"

class Resource;

class TOSSENGINE_API ISelectable
{
public:
    virtual void OnInspectorGUI() {}
    virtual void OnSelect() {}
    virtual void OnDeSelect() {}
    virtual bool Delete(bool deleteSelf = true) { return false;  }

    //helper function to keep ui looking the same and easy to change
    template<typename Resource>
    void ResourceAssignableField(std::shared_ptr<Resource>& resourcePtr, std::string fieldName)
    {
        ResourceManager& resourceManager = ResourceManager::GetInstance();
        std::string label = (resourcePtr)
            ? (fieldName + ": " + resourcePtr->getUniqueID())
            : fieldName + ": None";

        if (ImGui::BeginTable((fieldName + "Table").c_str(), 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            if (ImGui::Selectable(label.c_str()))
            {
                resourceManager.SetSelectedResource(resourcePtr);
            }

            ImGui::TableSetColumnIndex(1);
            if (ImGui::SmallButton("+"))
            {
                ImGui::OpenPopup((fieldName + "Dropdown").c_str());
            }

            if (ImGui::BeginPopup((fieldName + "Dropdown").c_str()))
            {
                bool noneSelected = (resourcePtr == nullptr);
                if (ImGui::Selectable("None", noneSelected))
                {
                    resourcePtr = nullptr;
                    resourceManager.SetSelectedResource(nullptr);
                }

                auto& resources = resourceManager.GetAllResources();
                for (const auto& [id, resource] : resources)
                {
                    if (std::dynamic_pointer_cast<Resource>(resource))
                    {
                        bool isSelected = (resourcePtr && resourcePtr->getUniqueID() == id);
                        if (ImGui::Selectable(id.c_str(), isSelected))
                        {
                            resourcePtr = std::dynamic_pointer_cast<Resource>(resource);
                            resourceManager.SetSelectedResource(resourcePtr);
                        }
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::EndTable();
        }
    }

    template<typename Resource>
    bool ResourceDropdownField(std::shared_ptr<Resource>& resourcePtr, const std::string& fieldName)
    {
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
                auto casted = std::dynamic_pointer_cast<Resource>(resource);
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
    
    static bool FloatSliderField(const string& name, float& value, float speed = 1.0f, float min = 0.0f, float max = 0.0f);

    static bool LayerDropdownField(const string& name, vector<string>& value);

    static bool BoolCheckboxField(const string& name, bool& value);
};

