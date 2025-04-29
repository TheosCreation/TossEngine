/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : Time.h
Description : Provides global timing values such as DeltaTime and FixedDeltaTime for frame-based updates.
              Includes support for time scaling.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "TossEngineAPI.h"

/**
 * @class Time
 * @brief Provides global timing values for frame updates.
 *        Includes DeltaTime, FixedDeltaTime, and a time scale multiplier.
 *        Intended for use in all engine timing-sensitive operations (physics, update loops, animations, etc.).
 */
class TOSSENGINE_API Time
{
public:
    /**
     * @brief Multiplier for controlling global time flow.
     *        Default is 1.0f (real-time), set to 0.0f to pause time, or >1.0f for fast-forward effects.
     */
    static float TimeScale;

    /**
     * @brief Time elapsed between the current and previous frame, in seconds (affected by TimeScale).
     */
    static float DeltaTime;

    /**
     * @brief Fixed time step used for physics and logic updates.
     */
    static float FixedDeltaTime;

    /**
     * @brief Updates the current DeltaTime (used once per frame).
     * @param deltaTime Raw time difference in seconds (usually from the OS/high-res timer).
     */
    static void UpdateDeltaTime(float deltaTime);

private:
    /**
     * @brief Deleted constructor to prevent instantiation.
     */
    Time() = delete;
};