#include "PointLight.h"
#include "GameObject.h"
#include "LightManager.h"

json PointLight::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this)); // Store the component type
    data["color"] = { m_color.x, m_color.y, m_color.z };
    data["radius"] = m_radius;
    return data;
}

void PointLight::deserialize(const json& data)
{
    if (data.contains("color") && data["color"].is_array() && data["color"].size() == 3)
    {
        m_color.x = data["color"][0];
        m_color.y = data["color"][1];
        m_color.z = data["color"][2];
    }

    if (data.contains("radius") && data["radius"].is_number_float())
    {
        m_radius = data["radius"];
    }

    m_owner->getLightManager()->updatePointLightColor(m_lightId, m_color);
    m_owner->getLightManager()->updatePointLightRadius(m_lightId, m_radius);
}

void PointLight::OnInspectorGUI()
{
    ImGui::Text("Point Light Inspector - ID: %p", this);
    ImGui::Separator();

    // Intencity input
    if (ImGui::SliderFloat("Intencity", &m_intencity, 0.0f, 5.0f))
    {
        SetIntencity(m_intencity);
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

void PointLight::onCreate()
{
    // Configure point light properties
    PointLightData pointLight;
    pointLight.Position = m_owner->m_transform.position;
    pointLight.Color = m_color;
    pointLight.SpecularStrength = 1.0f;
    pointLight.AttenuationConstant = 1.0f;
    pointLight.AttenuationLinear = 0.022f;
    pointLight.AttenuationExponent = 0.0019f;
    pointLight.Radius = m_radius;

    // Add the point light to the light manager
    m_lightId = m_owner->getLightManager()->createPointLight(pointLight);
}

void PointLight::onUpdate(float deltaTime)
{
    // Ensure the light updates its position if the owner moves
    if (m_owner)
    {
        m_owner->getLightManager()->updatePointLightPosition(m_lightId, m_owner->m_transform.position);
    }
}

void PointLight::onUpdateInternal()
{

    m_owner->getLightManager()->updatePointLightPosition(m_lightId, m_owner->m_transform.position);
}

void PointLight::SetIntencity(float intencity)
{
    m_intencity = intencity;
    m_owner->getLightManager()->updatePointLightIntencity(m_lightId, m_intencity);
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
