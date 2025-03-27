#pragma once
#include "Utils.h"

// Structure representing the toss player settings that the user can configure and save
struct TOSSENGINE_API TossPlayerSettings
{
    std::string firstSceneToOpen = "";
    std::vector<std::string> selectedSceneFilePaths = std::vector<std::string>();
    Vector2 windowSize = Vector2(800, 800);

    //stuff pulled from the project settings 
    RenderingPath renderingPath = RenderingPath::Deferred;

    bool LoadFromFile(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file)
        {
            Debug::LogWarning("Could not load player settings from " + filename);
            return false;
        }

        json j;
        file >> j;

        if (j.contains("FirstSceneToOpen"))
        {
            firstSceneToOpen = j["FirstSceneToOpen"].get<std::string>();
        }

        if (j.contains("AllSceneFilePaths"))
        {
            // Automatically convert JSON array to std::vector<std::string>
            selectedSceneFilePaths = j["AllSceneFilePaths"].get<std::vector<std::string>>();
        }

        if (j.contains("RenderingPath") && j["RenderingPath"].is_string()) {
            renderingPath = FromString<RenderingPath>(j["RenderingPath"].get<std::string>());
        }

        if (j.contains("WindowSize"))
        {
            // Assuming WindowSize is stored as an array: [width, height]
            windowSize.x = j["WindowSize"][0].get<float>();
            windowSize.y = j["WindowSize"][1].get<float>();
        }

        return true;
    }

    // Save settings to a JSON file
    void SaveToFile(const std::string& filename)
    {
        // Before saving, check if firstSceneToOpen is empty.
        if (firstSceneToOpen.empty())
        {
            if (!selectedSceneFilePaths.empty())
            {
                // Use the first selected scene if available.
                firstSceneToOpen = selectedSceneFilePaths[0];
            }
            else
            {
                // Log a debug message if no scene is selected.
                Debug::LogError("No scene file selected for FirstSceneToOpen.", false);
            }
        }

        json j;
        j["FirstSceneToOpen"] = firstSceneToOpen;
        j["AllSceneFilePaths"] = selectedSceneFilePaths; // Save the scenes list
        j["RenderingPath"] = ToString(renderingPath);
        j["WindowSize"] = { windowSize.x, windowSize.y };

        std::ofstream file(filename);
        if (!file)
        {
            std::cerr << "Failed to save " << filename << std::endl;
            return;
        }

        Debug::Log("Saved Player Settings to filepath: " + filename);

        file << j.dump(4); // Pretty-print JSON with indentation
    }
};