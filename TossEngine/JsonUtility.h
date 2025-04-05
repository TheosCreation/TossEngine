#pragma once
#include "Utils.h"

class JsonUtility {
public:
    static json OpenJsonFile(const std::string& filePath, bool ignoreWarnings = false)
    {
        json data;
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            if(!ignoreWarnings) Debug::LogWarning("Failed to load json file at: " + filePath + ". Please verify if the file exists and is valid");
            return data;
        }
        try
        {
            file >> data;
        }
        catch (const std::exception& e)
        {
            if (!ignoreWarnings) Debug::LogWarning("Failed to parse JSON file: " + (string)e.what());
            return data;
        }

        return data;
    }

    static bool SaveJsonFile(const std::string& filePath, json data, bool ignoreWarnings = false)
    {
        std::ofstream file(filePath);
        if (!file.is_open())
        {
            if (!ignoreWarnings) Debug::LogWarning("Failed to open file for writing: " + filePath);
            return false;
        }

        file << data.dump(4); // Pretty-print JSON with indentation
        file.close();
        return true;
    }
};