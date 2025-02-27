#include "TossEditor.h"
#include <Window.h>
#include <Game.h>

TossEditor::TossEditor()
{
    //init GLFW ver 4.6
    //if (!glfwInit())
    //{
    //    Debug::LogError("GLFW failed to initialize properly. Terminating program.");
    //    return;
    //}
    m_display = std::make_unique<Window>(this, Vector2(800, 800), "game");
    //Vector2 windowSize = m_display->getInnerSize();
}

TossEditor::~TossEditor()
{
}

void TossEditor::run()
{
    //onCreate();
   // onCreateLate();

    //run funcs while window open
    //while (m_display->shouldClose() == false)
    //{
    //    glfwPollEvents();
    //    //onUpdateInternal();
    //}

    //onQuit();
}

void TossEditor::onResize(Vector2 size)
{
    Resizable::onResize(size);


}