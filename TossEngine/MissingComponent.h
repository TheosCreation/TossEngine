/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : MissingComponent.h
Description : Represents a placeholder for a component that could not be loaded properly
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Component.h"

/**
 * @class MissingComponent
 * @brief Represents a placeholder for a component that could not be loaded properly (e.g., missing DLL/class).
 */
class TOSSENGINE_API MissingComponent : public Component
{
public:
    std::string missingType;   //!< Name of the missing component type.
    json originalData;         //!< Original serialized data for recovery purposes.

    /**
     * @brief Constructor initializing the missing type and data.
     * @param type The name of the missing component type.
     * @param data The original JSON data for the component.
     */
    MissingComponent(const std::string& type, const json& data)
        : missingType(type), originalData(data) {
    }

    /**
     * @brief Draws the missing component information in the Inspector.
     */
    void OnInspectorGUI() override
    {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "This component could not be loaded.");
        ImGui::Text("Type: %s", missingType.c_str());
        ImGui::Text("Original JSON:");
        ImGui::TextWrapped("%s", originalData.dump(2).c_str());
    }

    /**
     * @brief Serializes the MissingComponent back to JSON.
     * @return A JSON object representing the missing component.
     */
    json serialize() const override
    {
        json j = originalData;
        j["type"] = missingType;
        return j;
    }
};
