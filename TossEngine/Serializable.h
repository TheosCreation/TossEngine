#pragma once
#include "Utils.h"

class TOSSENGINE_API Serializable
{
public:
    // Serialize the GameObject to JSON
    virtual json serialize() const
    {
        return {
            {"type", getClassName(typeid(*this))} // used to identify class type
        };
    }

    // Deserialize the GameObject from JSON
    virtual void deserialize(const json& data) { }
};