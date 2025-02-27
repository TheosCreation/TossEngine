/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Camera.h
Description : Implements a camera for OpenGL
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Component.h"
#include "Math.h"
#include "Rect.h"

class GameObject;

class TOSSENGINE_API Camera : public Component
{
public:
    Camera() = default;
    ~Camera() = default;

    /**
     * @brief Gets the view matrix for the camera.
     * @param[out] view The view matrix to fill.
     */
    void getViewMatrix(Mat4& view);

    /**
     * @brief Gets the projection matrix for the camera.
     * @param[out] proj The projection matrix to fill.
     */
    void getProjectionMatrix(Mat4& proj) const;

    /**
     * @brief Sets the far plane distance of the camera.
     * @param farPlane The distance to set.
     */
    void setFarPlane(float farPlane);

    /**
     * @brief Sets the near plane distance of the camera.
     * @param nearPlane The distance to set.
     */
    void setNearPlane(float nearPlane);

    /**
     * @brief Sets the field of view (FOV) angle of the camera.
     * @param fov The FOV angle in degrees.
     */
    void setFieldOfView(float fov);

    /**
     * @brief Gets the type of the camera (Perspective or Orthographic).
     * @return The camera type.
     */
    CameraType getCameraType();

    /**
     * @brief Sets the type of the camera (Perspective or Orthographic).
     * @param type The camera type to set.
     */
    void setCameraType(const CameraType& type);


    /**
     * @brief Sets the screen area of the camera.
     * @param screen The screen area rectangle.
     */
    void setScreenArea(const Vector2& screen);

    /**
     * @brief Sets the target position that the camera is looking at.
     * @param newTargetPosition The new target position.
     */
    void setTargetPosition(Vector3 newTargetPosition);
 
    Vector3 getPosition();
    Vector3 getFacingDirection();

private:
    /**
     * @brief Computes the projection matrix based on camera parameters.
     */
    void computeProjectionMatrix();

private:
    Mat4 m_view{};                 //The view matrix of the camera.
    Mat4 m_projection{};           //The projection matrix of the camera.
    Vector3 m_targetPosition{};       //The target position of the camera.

    // move this to some sort of manager class
    Vector3 m_worldUp{ 0.0f, 1.0f, 0.0f };  //The upwards direction of the camera.

    float m_farPlane = 10000.0f;       //The distance of the far plane.
    float m_nearPlane = 0.1f;          //The distance of the near plane.
    float m_fov = 90.0f;                //The field of view (FOV) angle.
    const float radius = 10.0f;         //The radius of the camera's orbit.
    CameraType m_type = CameraType::Perspective;  //The type of the camera (Perspective or Orthographic).
    Rect m_screenArea;                   //The screen area of the camera.
};

