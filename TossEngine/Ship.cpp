#include "Ship.h"
#include "GameObject.h"

void Ship::onUpdate(float deltaTime)
{
    m_elapsedSeconds += deltaTime;

    // Calculate the position on the circular path
    float angle = m_elapsedSeconds * m_speed; // Angle in radians
    float x = m_radius * cos(angle);
    float z = m_radius * sin(angle);

    // Update the ship's position
    m_owner->m_transform.localPosition = Vector3(x, 50.0f, z);

    // Make the ship face the direction it's moving by calculating the forward vector
    glm::vec3 forward = glm::normalize(glm::vec3(-sin(angle), 0.0f, cos(angle)));

    // Convert the forward vector to a rotation quaternion
    Quaternion shipRotation = QuaternionUtils::LookAt(forward, glm::vec3(0.0f, 1.0f, 0.0f));

    // Set the ship's rotation
    m_owner->m_transform.localRotation = shipRotation;
}
