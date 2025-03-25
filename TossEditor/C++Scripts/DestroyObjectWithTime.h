#pragma once
#include <TossEngine.h>

class DestroyObjectWithTime : public Component
{
public:
	DestroyObjectWithTime() = default;
	~DestroyObjectWithTime() = default;

    // Serialize the GameObject to JSON
    json serialize() const override;

    // Deserialize the GameObject from JSON
    void deserialize(const json& data) override;


	void onStart() override;
	void onUpdate(float deltaTime) override;
	void OnInspectorGUI() override;

protected:
	float lifeTime = 5.0f;
private:
	float elapsedTime = 0.0f;
};

REGISTER_COMPONENT(DestroyObjectWithTime);