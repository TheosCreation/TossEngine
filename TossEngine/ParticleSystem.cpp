/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : ParticleSystem.cpp
Description : Represents a particle system that manages and renders particles in a graphics scene.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "ParticleSystem.h"
#include "GraphicsEngine.h"
#include "Shader.h"

void ParticleSystem::onCreate()
{
	GroupCountX = 300;
	WorkGroupSizeX = 128;
	NumParticles = WorkGroupSizeX * GroupCountX;

	// Store Position and Lifespan info
	glGenBuffers(1, &VBO_PositionLife);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, VBO_PositionLife);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		sizeof(glm::vec4) * NumParticles,
		NULL,
		GL_DYNAMIC_DRAW);

	// Store Velocity info
	glGenBuffers(1, &VBO_Velocity);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, VBO_Velocity);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		sizeof(glm::vec4) * NumParticles,
		NULL,
		GL_DYNAMIC_DRAW);

	// Store Color info
	glGenBuffers(1, &VBO_Color);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, VBO_Color);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		sizeof(glm::vec4) * NumParticles,
		NULL,
		GL_DYNAMIC_DRAW);

	// VAO for the standard render
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Bind the position and life buffer for the standard pipeline render
	glBindBuffer(GL_ARRAY_BUFFER, VBO_PositionLife);
	// Setup position attribute (4 components: x, y, z, lifespan)
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(0);

	// Bind the velocity buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Velocity);
	// Setup velocity attribute (4 components: x, y, z, w)
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(1);

	// Bind the color buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Color);
	// Setup color attribute (4 components: r, g, b, a)
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(2);

	// Unbinding
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ParticleSystem::onUpdate(float deltaTime)
{
	if (!isPlaying) return;

	m_elapsedTime += deltaTime;
	// Update particle properties (e.g., velocity and position) even if emission is stopped
	Vector3 Gravity = Vector3(0.0f, -9.8f, 0.0f) * deltaTime;
	VelocityLifeChange = Vector4(Gravity, deltaTime);

	// Stop emitting new particles if the elapsed time exceeds the duration
	if (isEmitting && m_elapsedTime > m_duration)
	{
		StopEmitting(); // Stop emitting, but let existing particles continue
	}
}

void ParticleSystem::Render(UniformData data, RenderingPath renderPath)
{
	if (!isPlaying) return; // If not playing, skip updating and rendering

	auto& graphicsEngine = GraphicsEngine::GetInstance();
	graphicsEngine.setShader(m_computeShader);

	// Set the uniform values for the compute shader
	int seedLife = static_cast<int>(randomRange(1000.0f, 10000.0f));
	int seedX = static_cast<int>(randomRange(1000.0f, 10000.0f));
	int seedY = static_cast<int>(randomRange(1000.0f, 10000.0f));
	int seedZ = static_cast<int>(randomRange(1000.0f, 10000.0f));

	// Pass seed values and other data to the compute shader
	m_computeShader->setInt("SeedLife", seedLife);
	m_computeShader->setInt("SeedX", seedX);
	m_computeShader->setInt("SeedY", seedY);
	m_computeShader->setInt("SeedZ", seedZ);
	m_computeShader->setVec4("VelocityLifeChange", VelocityLifeChange);
	m_computeShader->setVec3("EmitterOrigin", EmitterOrigin);

	// Pass a uniform to control whether new particles should be emitted
	m_computeShader->setBool("EmitNewParticles", isEmitting);

	// Bind the storage buffers for compute shader manipulations
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, VBO_PositionLife);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, VBO_Velocity);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, VBO_Color); // Bind color buffer

	// Dispatch the compute shader to update the particles
	glDispatchCompute(GroupCountX, 1, 1);

	// Wait for the compute shader completion and sync all threads
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	auto shader = m_material->GetShader();
	// Standard rendering pass: render all particles
	graphicsEngine.setShader(shader);
	shader->setMat4("VPMatrix", data.projectionMatrix * data.viewMatrix);
	graphicsEngine.setVertexArrayObject(VAO); // Bind vertex buffer to graphics pipeline
	graphicsEngine.drawTriangles(TriangleType::Points, NumParticles, 0); // Draw the particles

	// Unbinding
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void ParticleSystem::setOrigin(Vector3 origin)
{
	EmitterOrigin = origin;
}

void ParticleSystem::setComputeShader(ShaderPtr shader)
{
	m_computeShader = shader;
}

void ParticleSystem::setDuration(float _duration)
{
	m_duration = _duration;
}

void ParticleSystem::Play()
{
	isPlaying = true;
	isEmitting = true; // Start emitting new particles
	m_elapsedTime = 0.0f; // Reset elapsed time when starting the effect
}

void ParticleSystem::StopPlaying()
{
	isPlaying = false;
	m_elapsedTime = 0.0f; // Reset elapsed time when starting the effect
}

void ParticleSystem::StopEmitting()
{
	isEmitting = false;
}