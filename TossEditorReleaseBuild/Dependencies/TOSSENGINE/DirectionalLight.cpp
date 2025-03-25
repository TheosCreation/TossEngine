#include "DirectionalLight.h"
#include "GameObject.h"
#include "LightManager.h"

json DirectionalLight::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this));
    data["color"] = { m_color.x, m_color.y, m_color.z };
    data["intensity"] = m_intencity;
    return data;
}

void DirectionalLight::deserialize(const json& data)
{
    if (data.contains("color") && data["color"].is_array() && data["color"].size() == 3)
    {
        m_color.x = data["color"][0];
        m_color.y = data["color"][1];
        m_color.z = data["color"][2];
    }

    if (data.contains("intensity") && data["intensity"].is_number_float())
    {
        m_intencity = data["intensity"];
    }

    SetColor(m_color);
    SetIntencity(m_intencity);
}

void DirectionalLight::OnInspectorGUI()
{
    ImGui::Text("DirectionalLight Inspector - ID: %p", this);
    ImGui::Separator();

    // Intencity input
    if (ImGui::SliderFloat("Intencity", &m_intencity, 0.0f, 5.0f))
    {
        SetIntencity(m_intencity);
    }

    // Color input
    if (ImGui::ColorEdit3("Color", &m_color.x))
    {
        m_owner->getLightManager()->updateDirectionalLightColor(m_lightId, m_color);
    }
}


void DirectionalLight::onCreate()
{
    DirectionalLightData directionalLight1;
    directionalLight1.Direction = m_owner->m_transform.GetForward();
    directionalLight1.Color = m_color;
    directionalLight1.SpecularStrength = 0.1f;
    m_lightId = m_owner->getLightManager()->createDirectionalLight(directionalLight1);
}

void DirectionalLight::onUpdate(float deltaTime)
{
    if (m_owner)
    {
        m_owner->getLightManager()->updateDirectionalLightDirection(m_lightId, m_owner->m_transform.GetForward());
    }
}

void DirectionalLight::onUpdateInternal()
{
    m_owner->getLightManager()->updateDirectionalLightDirection(m_lightId, m_owner->m_transform.GetForward());
}

void DirectionalLight::SetIntencity(float intencity)
{
    m_intencity = intencity;
    m_owner->getLightManager()->updateDirectionalLightIntencity(m_lightId, m_intencity);
}

void DirectionalLight::SetColor(Vector3 color)
{
    m_color = color;
    m_owner->getLightManager()->updateDirectionalLightColor(m_lightId, m_color);
}
