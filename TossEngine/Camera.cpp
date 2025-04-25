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
	if (m_type == CameraType::Perspective)
	{
		m_view = LookAt(m_owner->m_transform.position, m_owner->m_transform.position + m_owner->m_transform.GetForward(), m_owner->m_transform.GetUp());
	}
	else if (m_type == CameraType::Orthogonal)
	{
		Vector3 cameraPosition = Vector3(m_screenArea.z, -m_screenArea.w, 1.0f);
		Vector3 targetPosition = Vector3(m_screenArea.z, -m_screenArea.w, 0.0f);
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

Vector3 Camera::screenToWorldPoint(Vector3 position)
{
    // Retrieve screen dimensions
    float screenWidth = m_screenArea.z;
    float screenHeight = m_screenArea.w;

    // Compute normalized device coordinates (NDC)
    // (x,y) will be in [-1, 1]. Note: (0,0) is bottom left.
    float ndcX = (position.x / (screenWidth - 1.0f)) * 2.0f - 1.0f;
    float ndcY = (position.y / (screenHeight - 1.0f)) * 2.0f - 1.0f;

    // Get the view matrix (assumed to be set by getViewMatrix)
    Mat4 view;
    getViewMatrix(view);

    if (m_type == CameraType::Perspective)
    {
        // For a perspective camera, compute a ray starting at the camera position.

        // Compute view-space coordinates at the near plane.
        float near = m_nearPlane;
        float tanFov = tan(glm::radians(m_fov) * 0.5f);
        float aspect = screenWidth / screenHeight;
        float viewX = ndcX * aspect * tanFov * near;
        float viewY = ndcY * tanFov * near;
        float viewZ = -near;  // In view space the camera looks down negative Z.

        // Create the near point in view space.
        glm::vec4 nearPointView(viewX, viewY, viewZ, 1.0f);

        // Transform the near point to world space using the inverse view matrix.
        glm::mat4 invView = glm::inverse(view.value);
        glm::vec4 nearPointWorld = invView * nearPointView;

        // Get the camera position (assumed stored in the owner transform).
        Vector3 camPos = m_owner->m_transform.position;

        // Compute the ray direction.
        Vector3 rayDir = Vector3(nearPointWorld) - camPos;
        rayDir = rayDir.Normalized();

        // Return the world position along the ray at the given distance (position.z).
        return camPos + rayDir * position.z;
    }
    else if (m_type == CameraType::Orthogonal)
    {
        // For an orthographic camera, the mapping is linear.
        // Our projection was computed with:
        //   left = -screenWidth, right = screenWidth,
        //   bottom = -screenHeight, top = screenHeight.
        // We remap the pixel coordinate accordingly.
        float worldX = glm::mix(-screenWidth, screenWidth, position.x / (screenWidth - 1.0f));
        float worldY = glm::mix(-screenHeight, screenHeight, position.y / (screenHeight - 1.0f));
        float worldZ = position.z; // Use the provided z directly as the depth in view space.

        // Create the point in view space.
        glm::vec4 pointView(worldX, worldY, worldZ, 1.0f);

        // Transform the point to world space using the inverse view matrix.
        glm::mat4 invView = glm::inverse(view.value);
        glm::vec4 worldPoint = invView * pointView;
        return Vector3(worldPoint);
    }

    // Fallback in case an unknown camera type is set.
    return Vector3();
}

Vector3 Camera::screenToWorldPoint(Vector2 position)
{
    return screenToWorldPoint(Vector3(position.x, position.y, m_nearPlane));
}

void Camera::computeProjectionMatrix()
{
	if (m_type == CameraType::Perspective)
		m_projection = Mat4(glm::perspective(glm::radians(m_fov), (float)m_screenArea.z / (float)m_screenArea.w, m_nearPlane, m_farPlane));
	else if (m_type == CameraType::Orthogonal)
		m_projection = Mat4(glm::ortho(-(float)m_screenArea.z, (float)m_screenArea.z, -(float)m_screenArea.w, (float)m_screenArea.w, m_nearPlane, m_farPlane));
}
