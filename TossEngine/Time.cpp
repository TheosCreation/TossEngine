#include "Time.h"

float Time::TimeScale = 1.0f;
float Time::DeltaTime = 0.0f;

void Time::UpdateDeltaTime(float deltaTime)
{
    DeltaTime = deltaTime * TimeScale;
}
