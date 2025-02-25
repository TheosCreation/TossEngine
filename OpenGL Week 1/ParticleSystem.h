/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : ParticleSystem.h
Description : Represents a particle system that manages and renders particles in a graphics scene.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
#include "Utils.h"
#include "GraphicsGameObject.h"

/**
 * @class ParticleSystem
 * @brief Represents a particle system that manages and renders particles in a graphics scene.
 *
 * The ParticleSystem handles the creation and update of particles, including their
 * position, velocity, and lifespan. It utilizes compute shaders for efficient particle
 * simulation and rendering.
 */
class ParticleSystem : public GraphicsGameObject
{
public:
    /**
     * @brief Called when the particle system is created.
     * This method initializes the particle system resources and properties.
     */
    void onCreate() override;

    /**
     * @brief Updates the particle system state.
     * @param deltaTime The time elapsed since the last update, used for updating
     *                  the particle positions and lifetimes.
     */
    void onUpdate(float deltaTime) override;

    /**
     * @brief Renders the particles using the specified uniform data.
     * @param data The uniform data to set for the shader during rendering.
     */
    void onGraphicsUpdate(UniformData data) override;

    /**
     * @brief Sets the origin point for the particle emitter.
     * @param origin The origin point as a Vector3 where particles are emitted from.
     */
    void setOrigin(Vector3 origin);

    /**
     * @brief Sets the compute shader used for particle simulation.
     * @param shader A shared pointer to the compute shader for the particle system.
     */
    void setComputeShader(ShaderPtr shader);

    void setDuration(float _duration);

    void Play();
    void StopPlaying();
    void StopEmitting();

private:
    uint VAO;                  // Vertex Array Object for the particle system.
    uint VBO_PositionLife;     // Vertex Buffer Object for storing particle positions and lifetimes.
    uint VBO_Velocity;         // Vertex Buffer Object for storing particle velocities.
    uint VBO_Color;            // Vertex Buffer Object for storing particle colors.

    ShaderPtr m_computeShader; // Shared pointer to the compute shader used for simulating particles.

    Vector3 EmitterOrigin = Vector3(0.0f); // The origin point from which particles are emitted.
    Vector4 VelocityLifeChange;              // Stores the velocity and life change parameters for the particles.

    int GroupCountX;            // Number of groups in the X dimension for compute shader dispatch.
    int WorkGroupSizeX;         // Size of the work group in the X dimension for compute shader.
    int NumParticles;           // Total number of particles managed by the system.
    bool isPlaying = false;     // Flag to check if the particle system is playing.
    bool isEmitting = false;     // Flag to check if the particle system is emitting particles.
    float m_elapsedTime = 0.0f;
    float m_duration = 1.0f;    // Duration for the particle systems play time.
};