#pragma once
#include "Utils.h"

class Serializable
{
public:
    // Serialize the entity to JSON
    virtual json serialize() const
    {
        return {
            {"type", getClassName(typeid(*this))} // used to idetify class type
        };
    }

    // Deserialize the entity from JSON
    virtual void deserialize(const json& data) { }
};