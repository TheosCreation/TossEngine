/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : LightManager.h
Description : Handles lighting and passing into shaders
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once

#include "Utils.h"
#include "Math.h"
#include <vector>
#include "ShadowMap.h"

/**
 * @class LightManager
 * @brief Handles lighting and passing into shaders.
 */
class TOSSENGINE_API LightManager
{
public:
    LightManager();
    ~LightManager() = default;

    /**
    * @brief Creates a point light and adds it to the manager.
    * @param newPointLight The point light to be added.
    */
    uint createPointLight(const PointLightData& newPointLight);
    void updatePointLightIntencity(uint lightId, const float& newIntencity);
    void updatePointLightPosition(uint lightId, const Vector3& position);
    void updatePointLightColor(uint lightId, const Vector3& newColor);
    void updatePointLightRadius(uint lightId, float newRadius);

    /**
     * @brief Creates a directional light and adds it to the manager.
     * @param newDirectionalLight The directional light to be added.
     */
    uint createDirectionalLight(const DirectionalLightData& newDirectionalLight);
    void updateDirectionalLightDirection(uint lightId, const Vector3& direction);
    void updateDirectionalLightIntencity(uint lightId, const float& newIntencity);
    void updateDirectionalLightColor(uint lightId, const Vector3& newColor);

    /**
     * @brief Creates a spotlight and adds it to the manager.
     * @param newSpotLight The spotlight to be added.
     */
    uint createSpotLight(const SpotLightData& newSpotLight);
    void updateSpotLightIntencity(uint lightId, const float& newIntencity);
    void updateSpotLightPosition(uint lightId, const Vector3& position);
    void updateSpotLightDirection(uint lightId, const Vector3& direction);
    void updateSpotLightColor(uint lightId, const Vector3& newColor);
    void updateSpotLightCutOff(uint lightId, const float& newCutoff);
    void updateSpotLightOuterCutOff(uint lightId, const float& newCutoff);

    /**
    * @brief Applies lighting information to the specified shader.
    * @param shader A shared pointer to the shader that will receive the lighting data.
    */
    void applyLighting(ShaderPtr shader) const;

    /**
     * @brief Applies shadow information to the specified shader.
     * @param shader A shared pointer to the shader that will receive the shadow data.
     */
    void applyShadows(ShaderPtr shader) const;

    /**
     * @brief Binds the shadow map texture for rendering.
     * @param index The index of the shadow map to bind.
     */
    void BindShadowMap(int index);

    /**
     * @brief Unbinds the shadow map texture.
     * @param index The index of the shadow map to unbind.
     */
    void UnBindShadowMap(int index);

    /**
     * @brief Gets the count of directional lights managed.
     * @return The current count of directional lights.
     */
    uint getDirectionalLightCount() const;

    /**
     * @brief Gets the light space matrix for a specific directional light.
     * @param index The index of the directional light.
     * @return The light space matrix for the specified directional light.
     */
    Mat4 getLightSpaceMatrix(uint index) const;

    /**
     * @brief Sets the shadow map texture for a specific directional light.
     * @param shadowMap The shadow map texture to set.
     * @param index The index of the directional light.
     */
    void setShadowMapTexture(ShadowMapPtr shadowMap, int index);

    /**
    * @brief Sets the status of the directional light.
    * @param status The new status of the directional light.
    */
    void setDirectionalLightStatus(bool status);

    /**
     * @brief Gets the status of the spotlight.
     * @return True if the spotlight is enabled, false otherwise.
     */
    bool getSpotlightStatus() const;

    /**
     * @brief Sets the status of the spotlight.
     * @param status The new status of the spotlight.
     */
    void setSpotlightStatus(bool status);

    /**
     * @brief Sets the position of the spotlight.
     * @param position The new position of the spotlight.
     */
    void setSpotlightPosition(Vector3 position);

    /**
     * @brief Sets the direction of the spotlight.
     * @param direction The new direction of the spotlight.
     */
    void setSpotlightDirection(Vector3 direction);

    /**
     * @brief Resets the light manager to its initial state.
     */
    void reset();

    /**
     * @brief Clears all lights from the manager.
     */
    void clearLights();

protected:

private:

    float AmbientStrength = 0.2f; //The strength of the ambient light
    Vector3 AmbientColor = Vector3(1.0f, 1.0f, 1.0f); //The color of the ambient light
    static const int MAX_POINT_LIGHTS = 25; //The maximum number of point lights
    PointLightData m_pointLights[MAX_POINT_LIGHTS] = {}; // Array of point lights
    static const int MAX_SPOT_LIGHTS = 10; //The maximum number of spot lights
    SpotLightData m_spotLights[MAX_SPOT_LIGHTS] = {}; // Array of point lights
    uint m_pointLightCount = 0; // The current count of point lights
    uint m_spotLightCount = 0; // The current count of spot lights

    static const int MAX_DIRECTIONAL_LIGHTS = 2; // The maximum number of directional lights
    DirectionalLightData m_directionalLights[MAX_DIRECTIONAL_LIGHTS] = {}; // Array of directional lights
    uint m_directionalLightCount = 0; // The current count of directional lights

    SpotLightData m_spotLight; // The spotlight

    ShadowMapPtr m_shadowMapTexture[MAX_DIRECTIONAL_LIGHTS] = {}; // Array to store shadow map textures.

    bool PointLightsStatus = true; //The status of point lights
    bool DirectionalLightStatus = true; //The status of the directional light
    bool SpotlightStatus = true; //The status of the spotlight
};