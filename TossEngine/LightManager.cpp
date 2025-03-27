/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : LightManager.cpp
Description : Handles lighting and passing into shaders
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "LightManager.h"
#include "Shader.h"
#include "ShadowMap.h"

LightManager::LightManager()
{
    for (int i = 0; i < MAX_DIRECTIONAL_LIGHTS; i++)
    {
        m_shadowMapTexture[i] = std::make_shared<ShadowMap>(Vector2(4096.0f));
    }
}

uint LightManager::createPointLight(const PointLightData& newPointLight)
{
    if (m_pointLightCount >= MAX_POINT_LIGHTS)
    {
        Debug::LogWarning("Maximum number of point lights reached!");
        return -1; // Indicate failure
    }

    uint lightId = m_pointLightCount; // Assign ID based on count
    m_pointLights[lightId] = newPointLight;
    m_pointLightCount++;

    return lightId; // Return valid ID
}

void LightManager::updatePointLightIntencity(uint lightId, const float& newIntencity)
{
    if (lightId >= m_pointLightCount)
    {
        Debug::LogWarning("Invalid PointLight ID: " + lightId);
        return;
    }

    m_pointLights[lightId].SpecularStrength = newIntencity;
}

void LightManager::updatePointLightPosition(uint lightId, const Vector3& position)
{
    if (lightId >= m_pointLightCount)
    {
        Debug::LogWarning("Invalid PointLight ID: " + lightId);
        return;
    }

    m_pointLights[lightId].Position = position;
}

void LightManager::updatePointLightColor(uint lightId, const Vector3& newColor)
{
    if (lightId >= m_pointLightCount)
    {
        Debug::LogWarning("Invalid PointLight ID: " + lightId);
        return;
    }

    m_pointLights[lightId].Color = newColor;
}

void LightManager::updatePointLightRadius(uint lightId, float newRadius)
{
    if (lightId >= m_pointLightCount)
    {
        Debug::LogWarning("Invalid PointLight ID: " + lightId);
        return;
    }

    m_pointLights[lightId].Radius = newRadius;
}

uint LightManager::createDirectionalLight(const DirectionalLightData& newDirectionalLight)
{
    if (m_directionalLightCount >= MAX_DIRECTIONAL_LIGHTS)
    {
        Debug::LogWarning("Maximum number of directional lights reached!");
        return -1;
    }

    uint lightId = m_directionalLightCount; // Assign ID based on count
    m_directionalLights[lightId] = newDirectionalLight;
    m_directionalLightCount++;

    return lightId; // Return valid ID
}

void LightManager::updateDirectionalLightDirection(uint lightId, const Vector3& direction)
{
    if (lightId >= m_directionalLightCount)
    {
        Debug::LogWarning("Invalid Directional ID: " + lightId);
        return;
    }

    m_directionalLights[lightId].Direction = direction;
}

void LightManager::updateDirectionalLightIntencity(uint lightId, const float& newIntencity)
{
    if (lightId >= m_directionalLightCount)
    {
        Debug::LogWarning("Invalid Directional ID: " + lightId);
        return;
    }

    m_directionalLights[lightId].SpecularStrength = newIntencity;
}

void LightManager::updateDirectionalLightColor(uint lightId, const Vector3& newColor)
{
    if (lightId >= m_directionalLightCount)
    {
        Debug::LogWarning("Invalid Directional ID: " + lightId);
        return;
    }

    m_directionalLights[lightId].Color = newColor;
}

uint LightManager::createSpotLight(const SpotLightData& newSpotLight)
{
    if (m_spotLightCount >= MAX_SPOT_LIGHTS)
    {
        Debug::LogWarning("Maximum number of spot lights reached!");
        return -1; // Indicate failure
    }

    uint lightId = m_spotLightCount; // Assign ID based on count
    m_spotLights[lightId] = newSpotLight;
    m_spotLightCount++;

    return lightId; // Return valid ID
}

void LightManager::updateSpotLightIntencity(uint lightId, const float& newIntencity)
{
    if (lightId >= m_spotLightCount)
    {
        Debug::LogWarning("Invalid Spot Light ID: " + lightId);
        return;
    }

    m_spotLights[lightId].SpecularStrength = newIntencity;
}

void LightManager::updateSpotLightPosition(uint lightId, const Vector3& position)
{
    if (lightId >= m_spotLightCount)
    {
        Debug::LogWarning("Invalid Spot Light ID: " + lightId);
        return;
    }

    m_spotLights[lightId].Position = position;
}

void LightManager::updateSpotLightDirection(uint lightId, const Vector3& direction)
{
    if (lightId >= m_spotLightCount)
    {
        Debug::LogWarning("Invalid Spot Light ID: " + lightId);
        return;
    }

    m_spotLights[lightId].Direction = direction;
}

void LightManager::updateSpotLightColor(uint lightId, const Vector3& newColor)
{
    if (lightId >= m_spotLightCount)
    {
        Debug::LogWarning("Invalid Spot Light ID: " + lightId);
        return;
    }

    m_spotLights[lightId].Color = newColor;
}

void LightManager::updateSpotLightCutOff(uint lightId, const float& newCutoff)
{
    if (lightId >= m_spotLightCount)
    {
        Debug::LogWarning("Invalid Spot Light ID: " + lightId);
        return;
    }

    m_spotLights[lightId].CutOff = newCutoff;
}

void LightManager::updateSpotLightOuterCutOff(uint lightId, const float& newCutoff)
{
    if (lightId >= m_spotLightCount)
    {
        Debug::LogWarning("Invalid Spot Light ID: " + lightId);
        return;
    }

    m_spotLights[lightId].OuterCutOff = newCutoff;
}

void LightManager::applyLighting(ShaderPtr shader) const
{
    shader->setFloat("AmbientStrength", AmbientStrength);
    shader->setVec3("AmbientColor", AmbientColor);

    if (PointLightsStatus)
    {
        for (unsigned int i = 0; i < m_pointLightCount; i++)
        {
            std::string index = std::to_string(i);
            shader->setVec3("PointLightArray[" + index + "].Base.Color", m_pointLights[i].Color);
            shader->setFloat("PointLightArray[" + index + "].Base.SpecularStrength", m_pointLights[i].SpecularStrength);

            shader->setVec3("PointLightArray[" + index + "].Position", m_pointLights[i].Position);
            shader->setFloat("PointLightArray[" + index + "].AttenuationConstant", m_pointLights[i].AttenuationConstant);
            shader->setFloat("PointLightArray[" + index + "].AttenuationLinear", m_pointLights[i].AttenuationLinear);
            shader->setFloat("PointLightArray[" + index + "].AttenuationExponent", m_pointLights[i].AttenuationExponent);
            shader->setFloat("PointLightArray[" + index + "].Radius", m_pointLights[i].Radius);
        }

        shader->setUint("PointLightCount", m_pointLightCount);
    }
    else
    {
        shader->setUint("PointLightCount", 0);
    }

    if (DirectionalLightStatus)
    {
        for (unsigned int i = 0; i < m_directionalLightCount; i++)
        {
            std::string index = std::to_string(i);
            shader->setVec3("DirLightArray[" + index + "].Base.Color", m_directionalLights[i].Color);
            shader->setFloat("DirLightArray[" + index + "].Base.SpecularStrength", m_directionalLights[i].SpecularStrength);

            shader->setVec3("DirLightArray[" + index + "].Direction", m_directionalLights[i].Direction);

            shader->setFloat("DirLightArray[" + index + "].Base.SpecularStrength", m_directionalLights[i].SpecularStrength);
        }
        //apply directional light to shader
        shader->setUint("DirectionalLightCount", m_directionalLightCount);
    }
    else
    {
        shader->setUint("DirectionalLightCount", 0);
    }

    if (SpotlightStatus)
    {
        for (unsigned int i = 0; i < m_spotLightCount; i++)
        {
            std::string index = std::to_string(i);
            shader->setVec3("SpotLightArray[" + index + "].Base.Color", m_spotLights[i].Color);
            shader->setFloat("SpotLightArray[" + index + "].Base.SpecularStrength", m_spotLights[i].SpecularStrength);

            shader->setVec3("SpotLightArray[" + index + "].Position", m_spotLights[i].Position);
            shader->setVec3("SpotLightArray[" + index + "].Direction", m_spotLights[i].Direction);
            shader->setFloat("SpotLightArray[" + index + "].CutOff", m_spotLights[i].CutOff);
            shader->setFloat("SpotLightArray[" + index + "].CutOff", m_spotLights[i].OuterCutOff);
            shader->setFloat("SpotLightArray[" + index + "].AttenuationConstant", m_spotLights[i].AttenuationConstant);
            shader->setFloat("SpotLightArray[" + index + "].AttenuationLinear", m_spotLights[i].AttenuationLinear);
            shader->setFloat("SpotLightArray[" + index + "].AttenuationExponent", m_spotLights[i].AttenuationExponent);
        }

        shader->setUint("SpotLightCount", m_spotLightCount);
    }
    else
    {
        shader->setUint("SpotLightCount", 0);
    }
}

void LightManager::applyShadows(ShaderPtr shader) const
{
    //bindings 5 - 6
    for (uint i = 0; i < m_directionalLightCount; i++)
    {
        std::string index = std::to_string(i);
        shader->setTexture2D(m_shadowMapTexture[i]->getId(), 5 + i, "Texture_ShadowMap[" + index + "]");
        shader->setMat4("VPLight[" + index + "]", getLightSpaceMatrix(i));
    }
}

void LightManager::BindShadowMap(int index)
{
    if (index >= 0 && index < MAX_DIRECTIONAL_LIGHTS) // Check bounds
    {
        if (m_shadowMapTexture[index]) // Check if the pointer is valid
        {
            m_shadowMapTexture[index]->Bind();
        }
        else
        {
            // Handle error: shadow map at index is null
            Debug::LogError("Error: Shadow map at index " + ToString(index) + " is null.", false);
        }
    }
    else
    {
        // Handle error: index is out of bounds
        Debug::LogError("Error: Index " + ToString(index) + " is out of bounds.", false);
    }
}


void LightManager::UnBindShadowMap(int index)
{
    m_shadowMapTexture[index]->UnBind();
}

uint LightManager::getDirectionalLightCount() const
{
    return m_directionalLightCount;
}

Mat4 LightManager::getLightSpaceMatrix(uint index) const
{
    // Validate index
    if (index < 0 || index >= m_directionalLightCount) {
        return Mat4(); // Return idGameObject matrix if index is out of range
    }

    // Check if the directional light is active
    if (!DirectionalLightStatus) {
        return Mat4(); // Return idGameObject matrix if the light is inactive
    }

    // Get the directional light properties
    const DirectionalLightData& light = m_directionalLights[index];

    // Orthographic projection for shadow mapping
    float sceneExtent = 1500.0f; // Adjust based on your scene's size
    Mat4 projection = glm::ortho(-sceneExtent, sceneExtent, -sceneExtent, sceneExtent, 0.1f, 2000.0f);

    // Normalize the direction vector
    Vector3 lightDirection = Normalize(light.Direction);

    // Position the light far from the scene
    Vector3 lightPosition = -lightDirection * 450.0f; // Move the light far away along the direction

    // Define the view matrix for the directional light
    Mat4 view = glm::lookAt(lightPosition, Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));

    // Combine the projection and view matrices to get the light-space matrix
    Mat4 lightSpaceMatrix = projection * view;

    return lightSpaceMatrix;
}

void LightManager::setShadowMapTexture(ShadowMapPtr shadowMap, int index)
{
    m_shadowMapTexture[index] = shadowMap;
}

void LightManager::setDirectionalLightStatus(bool status)
{
    DirectionalLightStatus = status;
}

bool LightManager::getSpotlightStatus() const
{
    return SpotlightStatus;
}

void LightManager::setSpotlightStatus(bool status)
{
    SpotlightStatus = status;
}

void LightManager::setSpotlightPosition(Vector3 position)
{
    m_spotLight.Position = position;
}

void LightManager::setSpotlightDirection(Vector3 direction)
{
    m_spotLight.Direction = direction;
}

void LightManager::reset()
{
    DirectionalLightStatus = true;
    SpotlightStatus = true;
    PointLightsStatus = true; 
    clearLights();
}

void LightManager::clearLights()
{
    // Reset point lights
    m_pointLightCount = 0;
    for (auto& pointLight : m_pointLights)
    {
        pointLight = PointLightData(); // Reset each point light to its default state
    }

    // Reset the spot lights
    m_spotLightCount = 0;
    for (auto& spotLight : m_spotLights)
    {
        spotLight = SpotLightData(); // Reset each spot light to its default state
    }

    // Reset the directional lights
    m_directionalLightCount = 0;
    for (auto& dirLight : m_directionalLights)
    {
        dirLight = DirectionalLightData(); // Reset each directional light to its default state
    }
}
