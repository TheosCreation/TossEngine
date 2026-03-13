#pragma once
#include "Component.h"

class TOSSENGINE_API PointLight : public Component
{
public:
	PointLight() = default;
	~PointLight() = default;

    virtual void OnInspectorGUI() override;

	virtual void onCreateLate() override;
	virtual void onUpdate() override;
    virtual void onUpdateInternal() override;
    void onDestroy() override;

    void SetIntencity(float intencity);
    void SetSpecularStrength(float intencity);
	void SetColor(Vector3 color);
	void SetRadius(float radius);
private:
    float m_intencity = 1.0f;
    float m_specularStrength = 0.1f;
	Vector3 m_color = Color::White;
	float m_radius = 50.0f;

	uint m_lightId = 0; //Assigned by lightmanager

    SERIALIZABLE_MEMBERS(m_intencity, m_specularStrength, m_color, m_radius)
};

REGISTER_COMPONENT(PointLight);