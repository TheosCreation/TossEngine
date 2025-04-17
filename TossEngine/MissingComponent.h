#pragma once
#include "Component.h"

class MissingComponent : public Component {
public:
    std::string missingType;
    json originalData;

    MissingComponent(const std::string& type, const json& data)
        : missingType(type), originalData(data) {}


    void OnInspectorGUI() override {
        ImGui::Text("This component could not be loaded.");
        ImGui::Text("Type: %s", missingType.c_str());
        ImGui::Text("Original JSON:");
        ImGui::TextWrapped("%s", originalData.dump(2).c_str());
    }

    json serialize() const override {
        // Preserve the original data and type
        json j = originalData;
        j["type"] = missingType;
        return j;
    }
};