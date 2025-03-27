/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Camera.cpp
Description : Implements a camera for OpenGL
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "Camera.h"
#include "GameObject.h"

json Camera::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this)); // Store the component type
    data["nearPlane"] = m_nearPlane;
    data["farPlane"] = m_farPlane;
    data["fov"] = m_fov;
    data["projection"] = ToString(m_type);

	return data;
}

void Camera::deserialize(const json& data)
{
    if (data.contains("nearPlane"))
    {
        m_nearPlane = data["nearPlane"];
    }
    
    if (data.contains("farPlane"))
    {
        m_farPlane = data["farPlane"];
    }
    
    if (data.contains("fov"))
    {
        m_fov = data["fov"];
    }
    if (data.contains("projection"))
    {
        m_type = FromString<CameraType>(data["projection"]);
    }
}

void Camera::OnInspectorGUI()
{
    ImGui::Text("Camera Inspector - ID: %p", this);
    ImGui::Separator();

    ImGui::DragFloat("Near Plane", &m_nearPlane, 0.1f);
    ImGui::DragFloat("Far Plane", &m_farPlane, 0.1f);
    ImGui::DragFloat("Fov", &m_fov, 0.1f, 1.0f, 179.0f);


    ImGui::Text("Camera Projection");
    // Rendering Path selection
    static const char* items[]{ "Orthogonal", "Perspective" };
    static int Selecteditem = (int)m_type;
    if (ImGui::Combo("Projection", &Selecteditem, items, IM_ARRAYSIZE(items)))
    {
        CameraType selectedPath = static_cast<CameraType>(Selecteditem);
        Debug::Log("Camera Projection changed to option: " + ToString(selectedPath));
        m_type = selectedPath;
    }
}

void Camera::getViewMatrix(Mat4& view)
{
	if (m_type == CameraType::Perspective)
	{
		m_view = LookAt(m_owner->m_transform.position, m_owner->m_transform.position + m_owner->m_transform.GetForward(), m_owner->m_transform.GetUp());
	}
	else if (m_type == CameraType::Orthogonal)
	{
		Vector3 cameraPosition = Vector3(m_screenArea.width, -m_screenArea.height, 1.0f);
		Vector3 targetPosition = Vector3(m_screenArea.width, -m_screenArea.height, 0.0f);
		m_view = LookAt(cameraPosition, targetPosition, m_owner->m_transform.GetUp());
	}
	view = m_view;
}

void Camera::getProjectionMatrix(Mat4& proj) const
{
	proj = m_projection;
}

void Camera::setFarPlane(float farPlane)
{
	m_farPlane = farPlane;
	computeProjectionMatrix();
}

void Camera::setNearPlane(float nearPlane)
{
	m_nearPlane = nearPlane;
	computeProjectionMatrix();
}

void Camera::setFieldOfView(float fov)
{
	m_fov = fov;
	computeProjectionMatrix();
}

CameraType Camera::getCameraType()
{
	return m_type;
}

void Camera::setCameraType(const CameraType& type)
{
	m_type = type;
	computeProjectionMatrix();
}

void Camera::setScreenArea(const Vector2& screen)
{
	m_screenArea = Rect(0, 0, (int)screen.x, (int)screen.y);
	computeProjectionMatrix();
}

void Camera::setTargetPosition(Vector3 newTargetPosition)
{
	m_targetPosition = newTargetPosition;
}

Vector3 Camera::getPosition()
{
	return m_owner->m_transform.position;
}

Vector3 Camera::getFacingDirection()
{
	return m_owner->m_transform.GetForward();
}

void Camera::computeProjectionMatrix()
{
	if (m_type == CameraType::Perspective)
		m_projection = Mat4(glm::perspective(glm::radians(m_fov), (float)m_screenArea.width / (float)m_screenArea.height, m_nearPlane, m_farPlane));
	else if (m_type == CameraType::Orthogonal)
		m_projection = Mat4(glm::ortho(-(float)m_screenArea.width, (float)m_screenArea.width, -(float)m_screenArea.height, (float)m_screenArea.height, m_nearPlane, m_farPlane));
}
