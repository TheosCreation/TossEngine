#include "DirectionalLight.h"
#include "GameObject.h"
#include "ImGuizmo.h"
#include "LightManager.h"

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


void DirectionalLight::onCreateLate()
{
    DirectionalLightData directionalLight1;
    directionalLight1.Direction = m_owner->m_transform.GetForward();
    directionalLight1.Color = m_color;
    directionalLight1.Intensity = m_intencity;
    directionalLight1.SpecularStrength = 0.1f;
    m_lightId = m_owner->getLightManager()->createDirectionalLight(directionalLight1);
}

void DirectionalLight::onUpdate()
{
    if (m_owner)
    {
        m_owner->getLightManager()->updateDirectionalLightDirection(m_lightId, m_owner->m_transform.GetForward());
    }
}

void DirectionalLight::onUpdateInternal()
{
    Component::onUpdateInternal();
    m_owner->getLightManager()->updateDirectionalLightDirection(m_lightId, m_owner->m_transform.GetForward());
}

void DirectionalLight::onDestroy()
{
    m_owner->getLightManager()->deleteDirectionalLight(m_lightId);
}

void DirectionalLight::OnDrawGizmosSelected(UniformData data)
{
    Vector3 forward = m_owner->m_transform.GetForward();
    ImGuizmo::DirectionalLightDraw directionalLight = {};
    directionalLight.Direction[0] = forward.x;
    directionalLight.Direction[1] = forward.y;
    directionalLight.Direction[2] = forward.z;
    directionalLight.Origin[0] = m_owner->m_transform.position.x;
    directionalLight.Origin[1] = m_owner->m_transform.position.y;
    directionalLight.Origin[2] = m_owner->m_transform.position.z;
    directionalLight.Length = 10.0f;
    
    ImGuizmo::DrawDirectionalLight(data.viewMatrix.data(), data.projectionMatrix.data(), directionalLight);
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
