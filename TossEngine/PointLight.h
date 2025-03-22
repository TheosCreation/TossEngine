#pragma once
#include "Component.h"

class PointLight : public Component
{
public:
	PointLight() = default;
	~PointLight() = default;

	// Serialize 
	virtual json serialize() const override;

	// Deserialize
	virtual void deserialize(const json& data) override;

	virtual void onCreate() override;
	virtual void onUpdate(float deltaTime) override;

	void SetColor(Vector3 color);
	void SetRadius(float radius);
private:
	Vector3 m_color = Color::White;
	float m_radius = 200.0f;

	uint m_lightId = 0;
};


REGISTER_COMPONENT(PointLight);