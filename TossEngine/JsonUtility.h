/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : JsonUtility.h
Description : Provides utility functions for reading and writing JSON files using nlohmann::json.
              Handles error logging and warning suppression for engine/editor JSON usage.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Utils.h"

/**
 * @class JsonUtility
 * @brief Provides static functions for opening and saving JSON files with optional warning suppression.
 */
class JsonUtility {
public:
    /**
     * @brief Loads a JSON file from disk and parses its contents.
     * @param filePath The path to the JSON file.
     * @param ignoreWarnings If true, suppresses error/warning logs on failure.
     * @return Parsed JSON object (empty if file is missing or invalid).
     */
    static json OpenJsonFile(const std::string& filePath, bool ignoreWarnings = false)
    {
        json data;
        std::ifstream file(filePath);

        if (!file.is_open())
        {
            if (!ignoreWarnings)
                Debug::LogWarning("Failed to load JSON file at: " + filePath + ". Please verify if the file exists and is valid.");
            return data;
        }

        try
        {
            file >> data;
        }
        catch (const std::exception& e)
        {
            if (!ignoreWarnings)
                Debug::LogWarning("Failed to parse JSON file: " + std::string(e.what()));
            return json{};
        }

        return data;
    }

    /**
     * @brief Saves a JSON object to disk in a pretty-printed format.
     * @param filePath The path to write the JSON file to.
     * @param data The JSON object to write.
     * @param ignoreWarnings If true, suppresses error logs on failure.
     * @return True if the save was successful, false otherwise.
     */
    static bool SaveJsonFile(const std::string& filePath, json data, bool ignoreWarnings = false)
    {
        std::ofstream file(filePath);
        if (!file.is_open())
        {
            if (!ignoreWarnings)
                Debug::LogWarning("Failed to open file for writing: " + filePath);
            return false;
        }

        file << data.dump(4); // Pretty-print JSON with 4-space indentation
        file.close();
        return true;
    }
};
