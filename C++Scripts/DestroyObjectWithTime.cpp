#include "DestroyObjectWithTime.h"

void DestroyObjectWithTime::onStart()
{
	Debug::Log("Destroy object called detroyobjectwithtime script");
}

void DestroyObjectWithTime::onUpdate(float deltaTime)
{
	elapsedTime += deltaTime;
	if (elapsedTime > lifeTime)
	{
		elapsedTime = -100;
		Debug::Log("Destroyed object");
		Destroy(m_owner);
		//
	}
}