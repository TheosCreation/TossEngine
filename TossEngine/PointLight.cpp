#include "PointLight.h"
#include "GameObject.h"
#include "ImGuizmo.h"
#include "LightManager.h"

void PointLight::OnInspectorGUI()
{
    ImGui::Text("Point Light Inspector - ID: %p", this);
    ImGui::Separator();

    // Intencity input
    if (ImGui::SliderFloat("Intencity", &m_intencity, 0.0f, 5.0f))
    {
        SetIntencity(m_intencity);
    }
    // Intencity input
    if (ImGui::SliderFloat("Specular Strength", &m_specularStrength, 0.0f, 5.0f))
    {
        SetSpecularStrength(m_specularStrength);
    }

    // Color input
    if (ImGui::ColorEdit3("Color", &m_color.x))
    {
        m_owner->getLightManager()->updatePointLightColor(m_lightId, m_color);
    }

    // Radius input
    if (ImGui::SliderFloat("Radius", &m_radius, 0.0f, 250.0f))
    {
        SetRadius(m_radius);
    }
}

void PointLight::onCreateLate()
{
    // Configure point light properties
    PointLightData pointLight;
    pointLight.Position = m_owner->m_transform.position;
    pointLight.Color = m_color;
    pointLight.Intensity = m_intencity;
    pointLight.SpecularStrength = m_specularStrength;
    //pointLight.AttenuationConstant = 1.0f;
    //pointLight.AttenuationLinear = 0.022f;
    //pointLight.AttenuationExponent = 0.0019f;
    pointLight.Radius = m_radius;

    // Add the point light to the light manager
    m_lightId = m_owner->getLightManager()->createPointLight(pointLight);
}

void PointLight::onUpdate()
{
    // Ensure the light updates its position if the owner moves
    if (m_owner)
    {
        m_owner->getLightManager()->updatePointLightPosition(m_lightId, m_owner->m_transform.position);
    }
}

void PointLight::onUpdateInternal()
{
    Component::onUpdateInternal();
    m_owner->getLightManager()->updatePointLightPosition(m_lightId, m_owner->m_transform.position);
}

void PointLight::OnDrawGizmosSelected(UniformData data)
{
    ImGuizmo::PointLightDraw pointLight = {};
    pointLight.Position[0] = m_owner->m_transform.position.x;
    pointLight.Position[1] = m_owner->m_transform.position.y;
    pointLight.Position[2] = m_owner->m_transform.position.z;
    pointLight.Radius = m_radius;
    
    ImGuizmo::DrawPointLight(data.viewMatrix.data(), data.projectionMatrix.data(), pointLight);
}

void PointLight::onDestroy()
{
    m_owner->getLightManager()->deletePointLight(m_lightId);
}

void PointLight::SetIntencity(float intencity)
{
    m_intencity = intencity;
    m_owner->getLightManager()->updatePointLightIntencity(m_lightId, m_intencity);
}

void PointLight::SetSpecularStrength(float specularStrength)
{
    m_specularStrength = specularStrength;
    m_owner->getLightManager()->updatePointLightSpecularStrength(m_lightId, m_specularStrength);
}

void PointLight::SetColor(Vector3 color)
{
    m_color = color;
    m_owner->getLightManager()->updatePointLightColor(m_lightId, m_color);
}

void PointLight::SetRadius(float radius)
{
    m_radius = radius;
    m_owner->getLightManager()->updatePointLightRadius(m_lightId, m_radius);
}
