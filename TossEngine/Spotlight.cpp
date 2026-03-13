#include "Spotlight.h"
#include "GameObject.h"
#include "ImGuizmo.h"
#include "LightManager.h"

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

    if (ImGui::SliderFloat("Range", &m_range, 0.0f, 100.0f))
    {
        SetRange(m_range);
    }
}

void Spotlight::onCreateLate()
{
    // Configure point light properties
    SpotLightData spotLight;
    spotLight.Position = m_owner->m_transform.position;
    spotLight.Direction = m_owner->m_transform.GetForward();
    spotLight.Color = m_color;
    spotLight.Intensity = m_intencity;
    spotLight.SpecularStrength = 1.0f;
    spotLight.AttenuationConstant = 1.0f;
    spotLight.AttenuationLinear = 0.014f;
    spotLight.AttenuationExponent = 0.0007f;
    spotLight.Range = m_range;
    spotLight.CutOff = glm::cos(glm::radians(m_cutoff));
    spotLight.OuterCutOff = glm::cos(glm::radians(m_outerCutoff));
    m_lightId = m_owner->getLightManager()->createSpotLight(spotLight);
}

void Spotlight::onUpdate()
{
    if (m_owner)
    {
        m_owner->getLightManager()->updateSpotLightPosition(m_lightId, m_owner->m_transform.position);
        m_owner->getLightManager()->updateSpotLightDirection(m_lightId, m_owner->m_transform.GetForward());
    }
}

void Spotlight::onUpdateInternal()
{
    Component::onUpdateInternal();
    m_owner->getLightManager()->updateSpotLightPosition(m_lightId, m_owner->m_transform.position);
    m_owner->getLightManager()->updateSpotLightDirection(m_lightId, m_owner->m_transform.GetForward());
}

void Spotlight::OnDrawGizmosSelected(UniformData data)
{
    ImGuizmo::SpotLightDraw spotlight = {};
    spotlight.Position[0] = m_owner->m_transform.position.x;
    spotlight.Position[1] = m_owner->m_transform.position.y;
    spotlight.Position[2] = m_owner->m_transform.position.z;
    Vector3 forward = m_owner->m_transform.GetForward();
    spotlight.Direction[0] = forward.x;
    spotlight.Direction[1] = forward.y;
    spotlight.Direction[2] = forward.z;
    spotlight.Range = m_range;
    spotlight.OuterCutoffRadians = glm::radians(m_outerCutoff);
    spotlight.InnerCutoffRadians = glm::radians(m_cutoff);
    
    ImGuizmo::DrawSpotLight(data.viewMatrix.data(), data.projectionMatrix.data(), spotlight);
}

void Spotlight::onDestroy()
{
    m_owner->getLightManager()->deleteSpotLight(m_lightId);
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

void Spotlight::SetRange(float range)
{
    m_range = range;
    m_owner->getLightManager()->updateSpotLightRange(m_lightId, m_range);
}

void Spotlight::SetIntencity(float intencity)
{
    m_intencity = intencity;
    m_owner->getLightManager()->updateSpotLightIntencity(m_lightId, m_intencity);
}
