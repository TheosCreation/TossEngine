#pragma once
#include <Resizable.h>

class Game;
class Window;

class TossEditor : public Resizable
{
public:
	TossEditor();
	~TossEditor();

	void run();
	void onResize(Vector2 size) override;

private:
	std::unique_ptr<Window> m_display; //Pointer to the window instance
	Game* game;
};