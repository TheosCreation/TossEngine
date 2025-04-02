#pragma once
#include "JsonUtility.h"
#include "ResourceManager.h"

class Resource;
class Shader;
class Mesh;
class Texture;
class Texture2D;
class TextureCubeMap;
class Sound;

class TOSSENGINE_API Serializable
{
public:
    // Serialize to JSON
    virtual json serialize() const
    {
        return {
            {"type", getClassName(typeid(*this))} // used to identify class type
        };
    }

    // Deserialize from JSON
    virtual void deserialize(const json& data) { }

    template<typename Resource>
    void ResourceSerializedField(std::shared_ptr<Resource>& resourcePtr, std::string fieldName)
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

    void FloatSlider(const std::string& name, float& value, float speed = 1.0f, float min = 0.0f, float max = 0.0f);
};