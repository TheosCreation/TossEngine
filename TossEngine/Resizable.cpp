#include "Resizable.h"

void Resizable::onResize(Vector2 size)
{
	m_size = size;
}

void Resizable::onReposition(Vector2 position)
{
    m_position = position;
}

void Resizable::onMaximize(int maximized)
{
    m_maximized = maximized;
}