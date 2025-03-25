#include "Spotlight.h"
#include "GameObject.h"
#include "LightManager.h"

json Spotlight::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this));
    data["color"] = { m_color.x, m_color.y, m_color.z };
    data["cutoff"] = m_cutoff;
    data["outerCutoff"] = m_outerCutoff;
    return data;
}

void Spotlight::deserialize(const json& data)
{
    if (data.contains("color") && data["color"].is_array() && data["color"].size() == 3)
    {
        m_color.x = data["color"][0];
        m_color.y = data["color"][1];
        m_color.z = data["color"][2];
    }

    if (data.contains("cutoff") && data["cutoff"].is_number_float())
    {
        m_cutoff = data["cutoff"];
    }

    if (data.contains("outerCutoff") && data["outerCutoff"].is_number_float())
    {
        m_outerCutoff = data["outerCutoff"];
    }

    m_owner->getLightManager()->updateSpotLightColor(m_lightId, m_color);
    m_owner->getLightManager()->updateSpotLightCutOff(m_lightId, glm::cos(glm::radians(m_cutoff)));
    m_owner->getLightManager()->updateSpotLightOuterCutOff(m_lightId, glm::cos(glm::radians(m_outerCutoff)));
}

void Spotlight::OnInspectorGUI()
{
    ImGui::Text("Spotlight Inspector - ID: %p", this);
    ImGui::Separator();

    // Intencity input
    if (ImGui::SliderFloat("Intencity", &m_intencity, 0.0f, 5.0f))
    {
        SetIntencity(m_intencity);
    }

    // Color input
    if (ImGui::ColorEdit3("Color", &m_color.x))
    {
        m_owner->getLightManager()->updateSpotLightColor(m_lightId, m_color);
    }

    // Cutoff angle input
    if (ImGui::SliderFloat("Cutoff Angle", &m_cutoff, 0.0f, 90.0f))
    {
        SetCutoff(m_cutoff);
    }

    // Outer cutoff angle input
    if (ImGui::SliderFloat("Outer Cutoff Angle", &m_outerCutoff, m_cutoff, 90.0f))
    {
        SetOuterCutoff(m_outerCutoff);
    }
}

void Spotlight::onCreate()
{
    // Configure point light properties
    SpotLightData spotLight;
    spotLight.Position = m_owner->m_transform.position;
    spotLight.Direction = m_owner->m_transform.GetForward();
    spotLight.Color = Color::White;
    spotLight.SpecularStrength = 1.0f;
    spotLight.AttenuationConstant = 1.0f;
    spotLight.AttenuationLinear = 0.014f;
    spotLight.AttenuationExponent = 0.0007f;
    spotLight.CutOff = glm::cos(glm::radians(m_cutoff));
    spotLight.OuterCutOff = glm::cos(glm::radians(m_outerCutoff));
    m_lightId = m_owner->getLightManager()->createSpotLight(spotLight);
}

void Spotlight::onUpdate(float deltaTime)
{
    if (m_owner)
    {
        m_owner->getLightManager()->updateSpotLightPosition(m_lightId, m_owner->m_transform.position);
        m_owner->getLightManager()->updateSpotLightDirection(m_lightId, m_owner->m_transform.GetForward());
    }
}

void Spotlight::onUpdateInternal()
{

    m_owner->getLightManager()->updateSpotLightPosition(m_lightId, m_owner->m_transform.position);
    m_owner->getLightManager()->updateDirectionalLightDirection(m_lightId, m_owner->m_transform.GetForward());
}

void Spotlight::SetColor(Vector3 color)
{
    m_color = color;
    m_owner->getLightManager()->updateSpotLightColor(m_lightId, m_color);
}

void Spotlight::SetCutoff(float newCutoff)
{
    m_cutoff = newCutoff;
    m_owner->getLightManager()->updateSpotLightCutOff(m_lightId, glm::cos(glm::radians(m_cutoff)));
}

void Spotlight::SetOuterCutoff(float newOuterCutoff)
{
    m_outerCutoff = newOuterCutoff;
    m_owner->getLightManager()->updateSpotLightOuterCutOff(m_lightId, glm::cos(glm::radians(m_outerCutoff)));
}

void Spotlight::SetIntencity(float intencity)
{
    m_intencity = intencity;
    m_owner->getLightManager()->updateSpotLightIntencity(m_lightId, m_intencity);
}
