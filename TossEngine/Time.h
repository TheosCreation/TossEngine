#pragma once
#include "TossEngineAPI.h"

class TOSSENGINE_API Time
{
public:
    static float TimeScale;
    static float DeltaTime;
    static void UpdateDeltaTime(float deltaTime);

private:

    // Prevent instantiation of Time
    Time() = delete;
};