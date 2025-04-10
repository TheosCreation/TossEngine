#include "GroundCheck.h"

void GroundCheck::OnInspectorGUI()
{
    Component::OnInspectorGUI();
    BoolCheckboxField("Is Grounded", isGrounded);
}

void GroundCheck::onTriggerEnter(Collider* other)
{
    isGrounded = true;
    Debug::Log("Yes");
}

void GroundCheck::onTriggerExit(Collider* other)
{
    isGrounded = false;
    Debug::Log("No");
}