#pragma once
#include "Utils.h"

class JsonUtility {
public:
    static json OpenJsonFile(const std::string& filePath)
    {
        json data;
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            Debug::LogWarning("Failed to load json file at: " + filePath + ". Please verify if the file exists and is valid");
            return data;
        }
        try
        {
            file >> data;
        }
        catch (const std::exception& e)
        {
            Debug::LogWarning("Failed to parse JSON file: " + (string)e.what());
            return data;
        }

        return data;
    }

    static void SaveJsonFile(const std::string& filePath, json data)
    {
        std::ofstream file(filePath);
        if (!file.is_open())
        {
            Debug::LogWarning("Failed to open file for writing: " + filePath);
            return;
        }

        file << data.dump(4); // Pretty-print JSON with indentation
        file.close();
    }
};