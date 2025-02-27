#pragma once
#include "Utils.h"

class TOSSENGINE_API Resizable
{
public:
	virtual void onResize(Vector2 size);

protected:
	Vector2 m_size = Vector2(100, 100);
};