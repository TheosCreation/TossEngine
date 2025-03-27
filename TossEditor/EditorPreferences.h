#pragma once
#include "Utils.h"

struct EditorPreferences
{
    string lastKnownOpenScenePath = "";
    Vector2 windowSize = Vector2(800, 800);
    bool maximized = false;

    void LoadFromFile(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file) {
            Debug::Log("Could not find file at " + filename + " creating fresh editor preferences");
            return;
        }

        json j;
        file >> j;

        if (j.contains("Maximized"))
        {
            maximized = j["Maximized"].get<bool>();
        }
        if (j.contains("WindowSize"))
        {
            // Assuming WindowSize is stored as an array: [width, height]
            windowSize.x = j["WindowSize"][0].get<float>();
            windowSize.y = j["WindowSize"][1].get<float>();
        }
        if (j.contains("LastKnownOpenScenePath") && j["LastKnownOpenScenePath"].is_string()) {
            lastKnownOpenScenePath = j["LastKnownOpenScenePath"];
        }
    }

    // Save settings to a JSON file
    void SaveToFile(const std::string& filename)
    {
        json j;
        j["Maximized"] = maximized;
        j["WindowSize"] = { windowSize.x, windowSize.y };
        j["LastKnownOpenScenePath"] = lastKnownOpenScenePath;

        std::ofstream file(filename);
        if (!file) {
            std::cerr << "Failed to save " << filename << std::endl;
            return;
        }

        Debug::Log("Saved Editor preferences to filepath: " + filename);

        file << j.dump(4); // Pretty-print JSON with indentation
    }
};