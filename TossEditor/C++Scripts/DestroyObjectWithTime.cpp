#include "DestroyObjectWithTime.h"

json DestroyObjectWithTime::serialize() const
{
    json data;
    data["type"] = getClassName(typeid(*this)); // Store the component type
    data["lifeTime"] = lifeTime;

    return data;
}

void DestroyObjectWithTime::deserialize(const json& data)
{
    if (data.contains("lifeTime"))
    {
        lifeTime = data["lifeTime"];
    }
}

void DestroyObjectWithTime::onStart()
{
	//Debug::Log("Destroy object called detroyobjectwithtime script");
}

void DestroyObjectWithTime::onUpdate()
{
	elapsedTime += Time::DeltaTime;
	if (elapsedTime > lifeTime)
	{
		elapsedTime = -100;
		//Debug::Log("Destroyed object");
		Destroy(m_owner);//
	}
}

void DestroyObjectWithTime::OnInspectorGUI()
{
    FloatSliderField("LifeTime", lifeTime);
    FloatSliderField("abc", abc);
}