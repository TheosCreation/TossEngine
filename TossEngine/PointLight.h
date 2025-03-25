#pragma once
#include "Component.h"

class TOSSENGINE_API PointLight : public Component
{
public:
	PointLight() = default;
	~PointLight() = default;

	// Serialize 
	virtual json serialize() const override;

	// Deserialize
	virtual void deserialize(const json& data) override;
    virtual void OnInspectorGUI() override;

	virtual void onCreate() override;
	virtual void onUpdate(float deltaTime) override;
    virtual void onUpdateInternal() override;

    void SetIntencity(float intencity);
	void SetColor(Vector3 color);
	void SetRadius(float radius);
private:
    float m_intencity = 1.0f;
	Vector3 m_color = Color::White;
	float m_radius = 200.0f;

	uint m_lightId = 0;
};

REGISTER_COMPONENT(PointLight);