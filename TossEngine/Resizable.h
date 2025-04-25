#pragma once
#include "Utils.h"

class TOSSENGINE_API Resizable
{
public:
	virtual void onResize(Vector2 size);
    virtual void onMaximize(int maximized);
    Vector2 getSize() const { return m_size;  }

protected:
	Vector2 m_size = Vector2(100, 100);
    bool m_maximized = false;
};