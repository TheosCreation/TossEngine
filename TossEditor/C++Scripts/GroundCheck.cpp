#include "GroundCheck.h"

void GroundCheck::OnInspectorGUI()
{
    Component::OnInspectorGUI();
    BoolCheckboxField("Is Grounded", isGrounded);
}

void GroundCheck::onTriggerEnter(Collider* other)
{
    isGrounded = true;
    Debug::Log("Entered");
}

void GroundCheck::onTriggerExit(Collider* other)
{
    isGrounded = false;
    Debug::Log("Exited");
}