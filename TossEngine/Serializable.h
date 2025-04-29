/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : Serializable.h
Description : Abstract base class for serializable objects.
              Provides virtual methods for JSON serialization/deserialization
              and includes runtime type identification support.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "SerializationMacros.h"

/**
 * @class Serializable
 * @brief Abstract base class for all objects that support JSON serialization and deserialization.
 */
class TOSSENGINE_API Serializable
{
public:
    /**
     * @brief Default constructor.
     */
    Serializable() = default;

    /**
     * @brief Serializes this object to a JSON representation.
     *        Derived classes should override this to add custom fields.
     * @return JSON object containing at minimum a "type" field.
     */
    virtual json serialize() const
    {
        return {
            { "type", getClassName(typeid(*this)) } // Used to store the class type name
        };
    }

    /**
     * @brief Deserializes this object from a JSON representation.
     *        Derived classes should override this to load custom fields.
     * @param data The JSON object to read from.
     */
    virtual void deserialize(const json& data) {}
};