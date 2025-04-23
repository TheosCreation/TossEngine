#include "Time.h"

float Time::TimeScale = 1.0f;
float Time::DeltaTime = 0.0f;
float Time::FixedDeltaTime = 1/60.0f;

void Time::UpdateDeltaTime(float deltaTime)
{
    DeltaTime = deltaTime * TimeScale;
    FixedDeltaTime = 1 / 60.0f * TimeScale;
}
