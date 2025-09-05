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
    data["drawUi"] = m_drawUi;

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

    if (data.contains("drawUi"))
    {
        m_drawUi = data["drawUi"];
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
    ImGui::Checkbox("Draw Ui", &m_drawUi);
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
    Mat4 world = m_owner->m_transform.GetMatrix();
    m_view = world.Inverse();
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

void Camera::SetDrawUi(bool drawUi)
{
    m_drawUi = drawUi;
}

bool Camera::GetDrawUi()
{
    return m_drawUi;
}

void Camera::setCameraType(const CameraType& type)
{
	m_type = type;
	computeProjectionMatrix();
}

void Camera::setScreenArea(const Vector2& screen)
{
	m_screenArea = Vector4(0, 0, screen.x, screen.y);
	computeProjectionMatrix();
}

void Camera::setScreenArea(const Vector4& area)
{
    m_screenArea = area;
    computeProjectionMatrix();
}

void Camera::setTargetPosition(Vector3 newTargetPosition)
{
	m_targetPosition = newTargetPosition;
}

Vector3 Camera::getPosition() const
{
	return m_owner->m_transform.position;
}

Vector3 Camera::getFacingDirection() const
{
	return m_owner->m_transform.GetForward();
}

float Camera::getNearPlane() const
{
    return m_nearPlane;
}

float Camera::getFarPlane() const
{
    return m_farPlane;
}

Vector3 Camera::screenToWorldPoint(Vector3 pixelZ)
{
    // 1) make coords relative to this camera's viewport
    const float vx = m_screenArea.x, vy = m_screenArea.y;
    const float vw = m_screenArea.z, vh = m_screenArea.w;

    float x = pixelZ.x - vx;
    float y = pixelZ.y - vy;
    // 2) NDC with top-left screen origin -> flip Y
    float ndcX = (x / vw) * 2.f - 1.f;
    float ndcY = 1.f - (y / vh) * 2.f;

    Mat4 V, P;
    getViewMatrix(V);
    getProjectionMatrix(P);
    glm::mat4 invVP = glm::inverse(P.value * V.value);

    // unproject near and far
    glm::vec4 p0 = invVP * glm::vec4(ndcX, ndcY, -1.f, 1.f); p0 /= p0.w;
    glm::vec4 p1 = invVP * glm::vec4(ndcX, ndcY, 1.f, 1.f); p1 /= p1.w;

    Vector3 origin(p0.x, p0.y, p0.z);
    Vector3 dir = (Vector3(p1.x, p1.y, p1.z) - origin).Normalized();

    // 3) interpret input z as world-space distance along the ray
    return origin + dir * pixelZ.z;
}

Vector3 Camera::screenToWorldPoint(Vector2 pixel)
{
    return screenToWorldPoint(Vector3(pixel.x, pixel.y, m_nearPlane));
}

void Camera::computeProjectionMatrix()
{
	if (m_type == CameraType::Perspective)
		m_projection = Mat4(glm::perspective(glm::radians(m_fov), (float)m_screenArea.z / (float)m_screenArea.w, m_nearPlane, m_farPlane));
	else if (m_type == CameraType::Orthogonal)
		m_projection = Mat4(glm::ortho(-(float)m_screenArea.z, (float)m_screenArea.z, -(float)m_screenArea.w, (float)m_screenArea.w, m_nearPlane, m_farPlane));
}
