#pragma once
#include <TossEngine.h>

class DestroyObjectWithTime : public Component
{
public:
	void onStart() override;
	void onUpdate() override;
	void OnInspectorGUI() override;
    json serialize() const override;
    void deserialize(const json& data) override;
protected:
	float lifeTime = 5.0f;
private:
	float elapsedTime = 0.0f;
	float abc = 0.0f;
};

REGISTER_COMPONENT(DestroyObjectWithTime);