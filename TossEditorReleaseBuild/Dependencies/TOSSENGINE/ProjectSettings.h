#pragma once
#include "Utils.h"

// Structure representing the project settings that the user can configue and save
struct TOSSENGINE_API ProjectSettings
{
    RenderingPath renderingPath = RenderingPath::Deferred;

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
    }

    // Save settings to a JSON file
    void SaveToFile(const std::string& filename)
    {
        json j;
        j["RenderingPath"] = ToString(renderingPath);

        std::ofstream file(filename);
        if (!file) {
            std::cerr << "Failed to save " << filename << std::endl;
            return;
        }

        Debug::Log("Saved Project Settings to filepath: " + filename);

        file << j.dump(4); // Pretty-print JSON with indentation
    }
};