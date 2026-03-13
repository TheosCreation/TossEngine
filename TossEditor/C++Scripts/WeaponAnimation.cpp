#include "WeaponAnimation.h"

#include "PlayerController.h"

void WeaponAnimation::OnInspectorGUI()
{
    FloatSliderField("Bobbing Horizontal Offset", m_bobbingHorizontalOffset);
    FloatSliderField("Bobbing Vertical Offset", m_bobbingverticalOffset);
    FloatSliderField("Bobbing Move Speed", m_bobbingMoveSpeed, 0.1f);
    FloatSliderField("Return To Center Speed", m_returnToCenterSpeed, 0.1f);
    FloatSliderField("Max Velocity Offset", m_maxVelocityOffset);
    FloatSliderField("Velocity Offset Multiplier", m_velocityOffsetMultiplier);
}

void WeaponAnimation::onStart()
{
    m_playerController = m_owner->getComponentInParent<PlayerController>();
    originalPos = m_owner->m_transform.localPosition;
    bobbingPosition = originalPos;

    UpdateBobbingTargets();
    target = rightPos;
}

void WeaponAnimation::onUpdate()
{
    UpdateBobbingTargets();

    if (m_playerController->IsWalking())
    {
        float movementStrength = std::min(m_playerController->GetLinearVelocity().Length(), 20.0f) / 20.0f;
        float bobbingStep = m_bobbingMoveSpeed * movementStrength * Time::DeltaTime;

        if (backToStart)
        {
            bobbingPosition = Vector3::MoveTowards(bobbingPosition, originalPos, bobbingStep);
        }
        else
        {
            bobbingPosition = Vector3::MoveTowards(bobbingPosition, target, bobbingStep);
        }

        if (bobbingPosition == originalPos)
        {
            backToStart = false;
        }
        else if (bobbingPosition == rightPos)
        {
            backToStart = true;
            target = leftPos;
        }
        else if (bobbingPosition == leftPos)
        {
            backToStart = true;
            target = rightPos;
        }
    }
    else
    {
        bobbingPosition = Vector3::MoveTowards(
            bobbingPosition,
            originalPos,
            m_returnToCenterSpeed * Time::DeltaTime
        );
    }

    Vector3 localVelocity = m_playerController->GetCamera()->getTransform().InverseTransformDirection(
        m_playerController->GetLinearVelocity()
    );

    Vector3 velocityOffset = Vector3::ClampMagnitude(
        -localVelocity * m_velocityOffsetMultiplier,
        m_maxVelocityOffset
    );

    currentOffset = Vector3::Lerp(currentOffset, velocityOffset, Time::DeltaTime * 10.0f);

    m_owner->m_transform.localPosition = bobbingPosition + currentOffset;
}
void WeaponAnimation::UpdateBobbingTargets()
{
    rightPos = Vector3(
        originalPos.x + m_bobbingHorizontalOffset,
        originalPos.y - m_bobbingverticalOffset,
        originalPos.z
    );

    leftPos = Vector3(
        originalPos.x - m_bobbingHorizontalOffset,
        originalPos.y - m_bobbingverticalOffset,
        originalPos.z
    );
}