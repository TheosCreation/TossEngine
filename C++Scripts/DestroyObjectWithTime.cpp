#include "DestroyObjectWithTime.h"

void DestroyObjectWithTime::onCreate()
{
	Debug::Log("Destroy object called detroyobjectwithtime script");
}

void DestroyObjectWithTime::onUpdate(float deltaTime)
{
	elapsedTime += deltaTime;
	if (elapsedTime > lifeTime)
	{
		elapsedTime = -100;
		Debug::Log("Destoyed object");
		//m_owner->getGameObjectManager()->removeGameObject(m_owner);
	}
}