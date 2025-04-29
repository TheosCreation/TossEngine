/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : ProjectSettings.h
Description : Defines the project settings configuration structure for TossEngine.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once
#include "Utils.h"

/**
 * @struct ProjectSettings
 * @brief Holds project-wide configurable settings such as rendering mode and physics parameters.
 */
struct TOSSENGINE_API ProjectSettings
{
    RenderingPath renderingPath = RenderingPath::Deferred;  //!< Default rendering path.
    Vector3 gravity = Vector3(0.0f, -9.81f, 0.0f);           //!< Default world gravity.

    /**
     * @brief Loads project settings from a JSON file.
     * @param filename The path to the settings file.
     */
    void LoadFromFile(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file)
        {
            Debug::LogWarning("Could not find project settings at: " + filename + " — using default settings.");
            return;
        }

        json j;
        try
        {
            file >> j;
        }
        catch (const std::exception& e)
        {
            Debug::LogWarning("Failed to parse project settings JSON: " + std::string(e.what()));
            return;
        }

        if (j.contains("RenderingPath") && j["RenderingPath"].is_string())
        {
            renderingPath = FromString<RenderingPath>(j["RenderingPath"].get<std::string>());
        }

        if (j.contains("Gravity") && j["Gravity"].is_array() && j["Gravity"].size() == 3)
        {
            gravity = Vector3(
                j["Gravity"][0].get<float>(),
                j["Gravity"][1].get<float>(),
                j["Gravity"][2].get<float>()
            );
        }
    }

    /**
     * @brief Saves project settings to a JSON file.
     * @param filename The path where the settings file will be saved.
     */
    void SaveToFile(const std::string& filename)
    {
        json j;
        j["RenderingPath"] = ToString(renderingPath);
        j["Gravity"] = { gravity.x, gravity.y, gravity.z };

        std::ofstream file(filename);
        if (!file)
        {
            Debug::LogError("Failed to save project settings to: " + filename);
            return;
        }

        Debug::Log("Saved Project Settings to: " + filename);
        file << j.dump(4); // Pretty-printed JSON
    }
};