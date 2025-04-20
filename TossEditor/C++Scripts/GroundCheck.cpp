#include "GroundCheck.h"

void GroundCheck::OnInspectorGUI()
{
    Component::OnInspectorGUI();
    BoolCheckboxField("Is Grounded", isGrounded);
}

void GroundCheck::onTriggerEnter(Collider* other)
{
    isGrounded = true;
}

void GroundCheck::onTriggerExit(Collider* other)
{
    isGrounded = false;
}