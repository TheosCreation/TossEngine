#pragma once
#include "SerializationMacros.h"

class TOSSENGINE_API Serializable
{
public:
    Serializable() = default;

    // Serialize to JSON
    virtual json serialize() const
    {
        return {
            {"type", getClassName(typeid(*this))} // used to identify class type
        };
    }

    // Deserialize from JSON
    virtual void deserialize(const json& data) { }
};