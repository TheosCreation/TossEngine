#pragma once
#include "Utils.h"

class JsonUtility {
public:
    // Serialize an object: stores the type name and then the fields.
    template <typename T>
    static json Serialize(const T* obj)
    {
        json j;
        //const rfk::Struct* structType = (rfk::Struct)rfk::getType<T>();
        //if (structType)
        //{
        //    // Store the type name first.
        //    j["type"] = structType->getName();
        //
        //    // Create a sub-object to hold field data.
        //    json data;
        //    for (const rfk::Field& field : structType->getFields())
        //    {
        //        // Process only fields marked with metadata "Variable"
        //        if (field.hasMetadata("Variable"))
        //        {
        //            std::string fieldName = field.getName();
        //            data[fieldName] = getFieldValueAsJson(field, obj);
        //        }
        //    }
        //    j["data"] = data;
        //}
        return j;
    }

    // Deserialize an object: optionally, you can check the "type" field.
    template <typename T>
    static void Deserialize(T* obj, const json& j)
    {
        //const rfk::Struct* structType = (rfk::Struct)rfk::getType<T>();
        //if (structType)
        //{
        //    // Optional: check if the type name in JSON matches.
        //    if (j.contains("type") && j["type"] != structType->getName())
        //    {
        //        throw std::runtime_error("Type mismatch during deserialization.");
        //    }
        //
        //    // Process the "data" sub-object.
        //    if (j.contains("data"))
        //    {
        //        const json& data = j["data"];
        //        for (rfk::Field& field : structType->getFields())
        //        {
        //            if (field.hasMetadata("Variable"))
        //            {
        //                std::string fieldName = field.getName();
        //                if (data.contains(fieldName))
        //                {
        //                    setFieldValueFromJson(field, obj, data[fieldName]);
        //                }
        //            }
        //        }
        //    }
        //}
    }

    static json OpenJsonFile(const std::string & filePath)
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

private:
};