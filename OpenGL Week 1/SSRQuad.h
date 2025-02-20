/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : SSRQuad.h
Description : SSRQuad class represents a quad used for screen space reflections,
			   inheriting from QuadEntity and implementing specific rendering techniques.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once
#include "QuadEntity.h"

/**
 * @class SSRQuad
 * @brief A class representing a quad used for screen space reflections.
 * Inherits from QuadEntity and implements specific rendering techniques for reflections.
 */
class SSRQuad : public QuadEntity
{
public:
    /**
     * @brief Updates graphics-related properties for the SSRQuad.
     * @param data Uniform data containing information needed for graphics updates.
     */
    void onGraphicsUpdate(UniformData data) override;

    /**
     * @brief Executes the lighting pass for the SSRQuad.
     * @param data Uniform data containing information needed for lighting calculations.
     */
    void onLightingPass(UniformData data) override;

    /**
     * @brief Executes the shadow pass for the SSRQuad.
     * @param index The index of the shadow being processed.
     */
    void onShadowPass(uint index) override;
};

