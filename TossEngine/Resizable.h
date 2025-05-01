#pragma once
#include "Utils.h"

class TOSSENGINE_API Resizable
{
public:
    virtual ~Resizable() = default;
    virtual void onResize(Vector2 size);
	virtual void onReposition(Vector2 position);
    virtual void onMaximize(int maximized);
    Vector2 getSize() const { return m_size;  }
    Vector2 getPosition() const { return m_position;  }

protected:
	Vector2 m_size = Vector2(100, 100);
	Vector2 m_position = Vector2(0, 0);
    bool m_maximized = false;
};