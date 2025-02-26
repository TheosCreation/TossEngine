#version 460 core

#define WORK_GROUP_SIZE_X 128

layout (local_size_x = WORK_GROUP_SIZE_X) in;

layout (std430, binding = 0) buffer PositionLifeBuffer { vec4 PositionLife[]; };
layout (std430, binding = 1) buffer VelocityBuffer { vec4 Velocity[]; };
layout (std430, binding = 2) buffer ColorBuffer { vec4 Color[]; }; // New buffer for colors

uniform int SeedLife;
uniform int SeedX;
uniform int SeedY;
uniform int SeedZ;
uniform vec4 VelocityLifeChange;
uniform vec3 EmitterOrigin;
uniform bool EmitNewParticles;

float RandomValue(uint Id, int Seed)
{
    float RandomValue;
    int Noise = int(Id) * Seed;
    Noise = (Noise << 13) ^ Noise;

    int T = (Noise * (Noise * Noise * 15731 + 789221) + 1376312589) & 0x7fffffff;
    RandomValue = 1.0f - float(T) * 0.93132257461548515625e-9f;
    return RandomValue;
}

void ResetParticle(uint Index)
{
    float LifeModifier = 5.0f;
    float Speed_XZ = 5.0f; // Adjusted for upward movement
    float Speed_Y = 15.0f; // Adjusted for upward burst

    PositionLife[Index] = vec4(EmitterOrigin.xyz, (RandomValue(Index, SeedLife) + 1) * 0.5f * LifeModifier);

    // Upwards velocity
    Velocity[Index] = vec4(
        Speed_XZ * RandomValue(Index, SeedX) - Speed_XZ * 0.5, // Random horizontal direction
        Speed_Y + RandomValue(Index, SeedY) * 2.0f, // Burst upwards
        Speed_XZ * RandomValue(Index, SeedZ) - Speed_XZ * 0.5,
        0.0f);

    Color[Index] = vec4(RandomValue(Index, SeedX), RandomValue(Index, SeedY), RandomValue(Index, SeedZ), 1.0f); // Random color
}

void KillParticle(uint Index)
{
    // Mark the particle as inactive by setting its lifespan to a negative value
    PositionLife[Index] = vec4(EmitterOrigin.xyz,
                                -1.0);
    Velocity[Index] = vec4(0.0);
}

void main()
{
    uint Index = gl_GlobalInvocationID.x;

    if (PositionLife[Index].w > 0.0)
    {
        // Update velocity and position
        Velocity[Index].xyz += VelocityLifeChange.xyz;
        PositionLife[Index].xyz += Velocity[Index].xyz * VelocityLifeChange.w;

        // Decrease lifespan
        PositionLife[Index].w -= VelocityLifeChange.w;

        // If lifespan reaches zero, kill the particle
        if (PositionLife[Index].w <= 0.0)
        {
            KillParticle(Index);
        }
    }
    else if (EmitNewParticles)
    {
        ResetParticle(Index);
    }
}
