#pragma once
#include "Utils.h"

class TOSSENGINE_API Resizable
{
public:
	virtual void onResize(Vector2 size);
    virtual void onMaximize(int maximized);

protected:
	Vector2 m_size = Vector2(100, 100);
    bool m_maximized = false;
};