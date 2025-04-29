/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : LightManager.h
Description : Manages dynamic lights (point, spot, directional) and shadow maps for real-time rendering.
              Handles updating light properties and passing them into shaders.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Utils.h"
#include "Math.h"
#include "ShadowMap.h"
#include <vector>

/**
 * @class LightManager
 * @brief Manages all light types (point, spot, directional) and their interaction with shaders and shadow maps.
 */
class TOSSENGINE_API LightManager
{
public:
    LightManager();
    ~LightManager() = default;

    // --- Point Lights ---

    uint createPointLight(const PointLightData& newPointLight);
    void updatePointLightIntencity(uint lightId, const float& newIntencity);
    void updatePointLightPosition(uint lightId, const Vector3& position);
    void updatePointLightColor(uint lightId, const Vector3& newColor);
    void updatePointLightRadius(uint lightId, float newRadius);

    // --- Directional Lights ---

    uint createDirectionalLight(const DirectionalLightData& newDirectionalLight);
    void updateDirectionalLightDirection(uint lightId, const Vector3& direction);
    void updateDirectionalLightIntencity(uint lightId, const float& newIntencity);
    void updateDirectionalLightColor(uint lightId, const Vector3& newColor);

    // --- Spotlights ---

    uint createSpotLight(const SpotLightData& newSpotLight);
    void updateSpotLightIntencity(uint lightId, const float& newIntencity);
    void updateSpotLightPosition(uint lightId, const Vector3& position);
    void updateSpotLightDirection(uint lightId, const Vector3& direction);
    void updateSpotLightColor(uint lightId, const Vector3& newColor);
    void updateSpotLightCutOff(uint lightId, const float& newCutoff);
    void updateSpotLightOuterCutOff(uint lightId, const float& newCutoff);

    // --- Shader Integration ---

    void applyLighting(ShaderPtr shader) const;
    void applyShadows(ShaderPtr shader);

    // --- Shadow Map Controls ---

    void BindShadowMap(int index);
    void UnBindShadowMap(int index);
    void setShadowMapTexture(ShadowMapPtr shadowMap, int index);
    Mat4 getLightSpaceMatrix(uint index) const;
    uint getDirectionalLightCount() const;

    // --- Spotlight Control ---

    void setSpotlightStatus(bool status);
    bool getSpotlightStatus() const;
    void setSpotlightPosition(Vector3 position);
    void setSpotlightDirection(Vector3 direction);

    // --- Directional Light Control ---

    void setDirectionalLightStatus(bool status);

    // --- Global Controls ---

    void clearLights();
    void reset();

private:
    // --- Ambient Lighting ---
    float AmbientStrength = 0.2f;
    Vector3 AmbientColor = Vector3(1.0f, 1.0f, 1.0f);

    // --- Point Lights ---
    static const int MAX_POINT_LIGHTS = 25;
    PointLightData m_pointLights[MAX_POINT_LIGHTS] = {};
    uint m_pointLightCount = 0;

    // --- Spot Lights ---
    static const int MAX_SPOT_LIGHTS = 10;
    SpotLightData m_spotLights[MAX_SPOT_LIGHTS] = {};
    SpotLightData m_spotLight;
    uint m_spotLightCount = 0;

    // --- Directional Lights ---
    static const int MAX_DIRECTIONAL_LIGHTS = 2;
    DirectionalLightData m_directionalLights[MAX_DIRECTIONAL_LIGHTS] = {};
    uint m_directionalLightCount = 0;

    // --- Shadow Maps ---
    ShadowMapPtr m_shadowMapTexture[MAX_DIRECTIONAL_LIGHTS] = {};

    // --- Status Flags ---
    bool PointLightsStatus = true;
    bool DirectionalLightStatus = true;
    bool SpotlightStatus = true;
};
