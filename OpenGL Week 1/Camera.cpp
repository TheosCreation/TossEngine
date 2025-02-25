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
#include "GameObjectManager.h"

void Camera::getViewMatrix(Mat4& view)
{
	if (m_type == CameraType::Perspective)
	{
		m_view = glm::lookAt(m_owner->m_transform.position, m_owner->m_transform.position + m_owner->m_transform.GetForward(), m_owner->m_transform.GetUp());
	}
	else if (m_type == CameraType::Orthogonal)
	{
		Vector3 cameraPosition = Vector3(m_screenArea.width, -m_screenArea.height, 1.0f);
		Vector3 targetPosition = Vector3(m_screenArea.width, -m_screenArea.height, 0.0f);
		m_view = glm::lookAt(cameraPosition, targetPosition, m_owner->m_transform.GetUp());
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
		m_projection = glm::perspective(glm::radians(m_fov), (float)m_screenArea.width / (float)m_screenArea.height, m_nearPlane, m_farPlane);
	else if (m_type == CameraType::Orthogonal)
		m_projection = glm::ortho(-(float)m_screenArea.width, (float)m_screenArea.width, -(float)m_screenArea.height, (float)m_screenArea.height, m_nearPlane, m_farPlane);
}
