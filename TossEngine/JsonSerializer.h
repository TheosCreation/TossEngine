#pragma once
#include "Utils.h"
#include <Refureku/Refureku.h>

class JsonSerializer {
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

private:
    // Helper: Retrieve a field's value as JSON using getPtr.
    template <typename T>
    static json getFieldValueAsJson(const rfk::Field& field, const T* obj)
    {
        // Check for int type
        if (field.getType() == rfk::getType<int>())
        {
            const int* fieldPtr = static_cast<const int*>(field.getPtr(obj));
            return *fieldPtr;
        }
        else if (field.getType() == rfk::getType<float>())
        {
            const float* fieldPtr = static_cast<const float*>(field.getPtr(obj));
            return *fieldPtr;
        }
        else if (field.getType() == rfk::getType<double>())
        {
            const double* fieldPtr = static_cast<const double*>(field.getPtr(obj));
            return *fieldPtr;
        }
        else if (field.getType() == rfk::getType<std::string>())
        {
            const std::string* fieldPtr = static_cast<const std::string*>(field.getPtr(obj));
            return *fieldPtr;
        }
        // Extend with additional type checks as needed.
        return nullptr; // Unsupported type
    }

    // Helper: Set a field's value from JSON using getPtr.
    template <typename T>
    static void setFieldValueFromJson(rfk::Field& field, T* obj, const json& jValue)
    {
        if (field.getType() == rfk::getType<int>())
        {
            int* fieldPtr = static_cast<int*>(field.getPtr(obj));
            *fieldPtr = jValue.get<int>();
        }
        else if (field.getType() == rfk::getType<float>())
        {
            float* fieldPtr = static_cast<float*>(field.getPtr(obj));
            *fieldPtr = jValue.get<float>();
        }
        else if (field.getType() == rfk::getType<double>())
        {
            double* fieldPtr = static_cast<double*>(field.getPtr(obj));
            *fieldPtr = jValue.get<double>();
        }
        else if (field.getType() == rfk::getType<std::string>())
        {
            std::string* fieldPtr = static_cast<std::string*>(field.getPtr(obj));
            *fieldPtr = jValue.get<std::string>();
        }
        // Extend with additional type handling as needed.
    }
};