#pragma once
#include "TossEngineAPI.h"

class TOSSENGINE_API Time
{
public:
    static float TimeScale;
    static float DeltaTime;
    static const float FixedDeltaTime;//will make it modifiable at some point
    static void UpdateDeltaTime(float deltaTime);

private:

    // Prevent instantiation of Time
    Time() = delete;
};