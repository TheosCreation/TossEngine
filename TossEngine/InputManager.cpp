/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : InputManager.cpp
Description : Handles inputs from the player/user of the program
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "InputManager.h"
#include "Window.h"
#include "TossEngine.h"
#include "ProjectSettings.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <glew.h>
#include <glfw3.h>

double InputManager::scrollX = 0.0;
double InputManager::scrollY = 0.0;
float InputManager::currentMouseX = 0.0;
float InputManager::currentMouseY = 0.0;

std::map<Key, bool> InputManager::currentKeyStates;
std::map<Key, bool> InputManager::previousKeyStates;
std::map<MouseButton, bool> InputManager::currentMouseStates;
std::map<MouseButton, bool> InputManager::previousMouseStates;

void InputManager::Init(ProjectSettingsPtr& projectSettings)
{
	if (isInitilized) return;

	WindowPtr = TossEngine::GetInstance().GetWindow()->getWindow();
	glfwSetScrollCallback(WindowPtr, scroll_callback); // Set the scroll callback function
	glfwSetKeyCallback(WindowPtr, key_callback);       // Set the key callback function
	glfwSetMouseButtonCallback(WindowPtr, mouse_button_callback); // Set the mouse button callback function
    glfwSetCharCallback(WindowPtr, char_callback); // Set the char callback function

	isInitilized = true;
}

void InputManager::Init(TossPlayerSettingsPtr& playerSettings)
{
    if (isInitilized) return;

    WindowPtr = TossEngine::GetInstance().GetWindow()->getWindow();
    glfwSetScrollCallback(WindowPtr, scroll_callback); // Set the scroll callback function
    glfwSetKeyCallback(WindowPtr, key_callback);       // Set the key callback function
    glfwSetMouseButtonCallback(WindowPtr, mouse_button_callback); // Set the mouse button callback function
    glfwSetCharCallback(WindowPtr, char_callback); // Set the char callback function

    isInitilized = true;
}

bool InputManager::isKeyDown(Key key, bool checkPlayMode) const
{
    if (checkPlayMode && !isPlayModeEnabled()) return false;

	return currentKeyStates[key];
}

bool InputManager::isKeyPressed(Key key, bool checkPlayMode) const
{
    if (checkPlayMode && !isPlayModeEnabled()) return false;

	// Return true if the key is currently pressed but was not pressed in the previous frame
	return currentKeyStates[key] && !previousKeyStates[key];
}

bool InputManager::isKeyUp(Key key, bool checkPlayMode) const
{
    if (checkPlayMode && !isPlayModeEnabled()) return false;

	return !currentKeyStates[key];
}

bool InputManager::isMouseDown(MouseButton button, bool checkPlayMode) const
{
    if (checkPlayMode && !isPlayModeEnabled()) return false;

	return currentMouseStates[button];
}

bool InputManager::isMousePressed(MouseButton button, bool checkPlayMode) const
{
    if (checkPlayMode && !isPlayModeEnabled()) return false;

	return currentMouseStates[button] && !previousMouseStates[button];
}

bool InputManager::isMouseUp(MouseButton button, bool checkPlayMode) const
{
    if (checkPlayMode && !isPlayModeEnabled()) return false;

	return !currentMouseStates[button];
}

float InputManager::getMouseXAxis(bool checkPlayMode) const
{
    if (checkPlayMode && !isPlayModeEnabled()) return false;

	return m_deltaMouse.x;
}

float InputManager::getMouseYAxis(bool checkPlayMode) const
{
    if (checkPlayMode && !isPlayModeEnabled()) return false;

	return m_deltaMouse.y;
}

Vector2 InputManager::getMousePosition()
{
	return {currentMouseX, currentMouseY};
}

Vector2 InputManager::getMouseScroll()
{
	return {static_cast<float>(scrollX), static_cast<float>(scrollY)};
}

void InputManager::resetMouseScroll()
{
	scrollX = 0.0;
	scrollY = 0.0;
}

void InputManager::enablePlayMode(bool enable, bool alsoChangeGameplayMode)
{
	m_playEnable = enable;
    if (alsoChangeGameplayMode)
    {
        m_gameMode = enable;
    }

	if (enable) {
		glfwSetInputMode(WindowPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPos(WindowPtr, m_screenArea.x / 2.0f, m_screenArea.y / 2.0f);
	}
	else {
		glfwSetInputMode(WindowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void InputManager::togglePlayMode(bool alsoChangeGameplayMode)
{
    enablePlayMode(!m_playEnable, alsoChangeGameplayMode);
}

bool InputManager::isPlayModeEnabled() const
{
    return m_playEnable && m_gameMode;
}

bool InputManager::isGameModeEnabled() const
{
    return m_gameMode;
}

void InputManager::setScreenArea(const Vector2& area)
{
	m_screenArea = area;
}

void InputManager::onUpdate()
{
    double newMouseX, newMouseY;
    glfwGetCursorPos(WindowPtr, &newMouseX, &newMouseY);

    currentMouseX = static_cast<float>(newMouseX);
    currentMouseY = static_cast<float>(newMouseY);

    if (m_playEnable)
    {
        float centerX = (m_screenArea.x * 0.5f);
        float centerY = (m_screenArea.y * 0.5f);

        // m_deltaMouse is relative inside viewport!
        m_deltaMouse = Vector2(currentMouseX - centerX, currentMouseY - centerY);

        glfwSetCursorPos(WindowPtr, centerX, centerY);
    }
    else
    {
        m_deltaMouse = Vector2(currentMouseX - m_oldMousePos.x, currentMouseY - m_oldMousePos.y);
    }
}

void InputManager::onLateUpdate()
{
    previousKeyStates = currentKeyStates;
    previousMouseStates = currentMouseStates;

    resetMouseScroll();

    if (m_playEnable)
    {
        float centerX = (m_screenArea.x * 0.5f);
        float centerY = (m_screenArea.y * 0.5f);

            glfwSetCursorPos(WindowPtr, centerX, centerY);
    }

    m_oldMousePos = Vector2(currentMouseX, currentMouseY);
}


void InputManager::scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
	scrollX = xOffset;
	scrollY = yOffset;

    ImGui_ImplGlfw_ScrollCallback(window, xOffset, yOffset);
}

void InputManager::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Key translatedKey = static_cast<Key>(key);

    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

	if (action == GLFW_PRESS && !currentKeyStates[translatedKey]) {
		currentKeyStates[translatedKey] = true;
	}
	else if (action == GLFW_RELEASE) {
		currentKeyStates[translatedKey] = false;
	}
}

void InputManager::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	MouseButton translatedButton = static_cast<MouseButton>(button);

    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

	if (action == GLFW_PRESS && !currentMouseStates[translatedButton]) {
		currentMouseStates[translatedButton] = true;
	}
	else if (action == GLFW_RELEASE) {
		currentMouseStates[translatedButton] = false;
	}
}

void InputManager::char_callback(GLFWwindow* window, unsigned int c)
{
    ImGui_ImplGlfw_CharCallback(window, c);
}
