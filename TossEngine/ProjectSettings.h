#pragma once
#include "Utils.h"

// Structure representing the project settings that the user can configue and save
struct TOSSENGINE_API ProjectSettings
{
    RenderingPath renderingPath = RenderingPath::Deferred;
    Vector3 gravity = Vector3(0.0f, -9.81f, 0.0f);

    void LoadFromFile(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file) {
            Debug::Log("Could not find file at " + filename + " creating fresh project settings");
            return;
        }

        json j;
        file >> j;

        if (j.contains("RenderingPath") && j["RenderingPath"].is_string()) {
            renderingPath = FromString<RenderingPath>(j["RenderingPath"].get<std::string>()); 
        }

        if (j.contains("Gravity") && j["Gravity"].is_array() && j["Gravity"].size() == 3) {
            gravity = Vector3(
                j["Gravity"][0].get<float>(),
                j["Gravity"][1].get<float>(),
                j["Gravity"][2].get<float>()
            );
        }
    }

    // Save settings to a JSON file
    void SaveToFile(const std::string& filename)
    {
        json j;
        j["RenderingPath"] = ToString(renderingPath);
        j["Gravity"] = { gravity.x, gravity.y, gravity.z };

        std::ofstream file(filename);
        if (!file) {
            std::cerr << "Failed to save " << filename << std::endl;
            return;
        }

        Debug::Log("Saved Project Settings to filepath: " + filename);

        file << j.dump(4); // Pretty-print JSON with indentation
    }
};