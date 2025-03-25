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
#include "TossPlayerSettings.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glew.h>
#include <glfw3.h>
#include <imgui_internal.h>

double InputManager::scrollX = 0.0;
double InputManager::scrollY = 0.0;
double InputManager::currentMouseX = 0.0;
double InputManager::currentMouseY = 0.0;

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

	isInitilized = true;
}

void InputManager::Init(TossPlayerSettingsPtr& playerSettings)
{
    if (isInitilized) return;

    WindowPtr = TossEngine::GetInstance().GetWindow()->getWindow();
    glfwSetScrollCallback(WindowPtr, scroll_callback); // Set the scroll callback function
    glfwSetKeyCallback(WindowPtr, key_callback);       // Set the key callback function
    glfwSetMouseButtonCallback(WindowPtr, mouse_button_callback); // Set the mouse button callback function

    isInitilized = true;
}

bool InputManager::isKeyDown(Key key)
{
	return currentKeyStates[key];
}

bool InputManager::isKeyPressed(Key key)
{
	// Return true if the key is currently pressed but was not pressed in the previous frame
	return currentKeyStates[key] && !previousKeyStates[key];
}

bool InputManager::isKeyUp(Key key)
{
	return !currentKeyStates[key];
}

bool InputManager::isMouseDown(MouseButton button)
{
	return currentMouseStates[button];
}

bool InputManager::isMousePressed(MouseButton button)
{
	return currentMouseStates[button] && !previousMouseStates[button];
}

bool InputManager::isMouseUp(MouseButton button)
{
	return !currentMouseStates[button];
}

float InputManager::getMouseXAxis()
{
	return m_deltaMouse.x;
}

float InputManager::getMouseYAxis()
{
	return m_deltaMouse.y;
}

Vector2 InputManager::getMousePosition()
{
	return Vector2(currentMouseX, currentMouseY);
}

Vector2 InputManager::getMouseScroll()
{
	return Vector2(scrollX, scrollY);
}

void InputManager::resetMouseScroll()
{
	scrollX = 0.0;
	scrollY = 0.0;
}

void InputManager::enablePlayMode(bool enable)
{
	m_playEnable = enable;

	if (enable) {
		glfwSetInputMode(WindowPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPos(WindowPtr, m_screenArea.width / 2.0, m_screenArea.height / 2.0);
	}
	else {
		glfwSetInputMode(WindowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void InputManager::setScreenArea(const Vector2& area)
{
	m_screenArea = Rect(0, 0, (int)area.x, (int)area.y);
}

void InputManager::onUpdate()
{
	double newMouseX, newMouseY;
	glfwGetCursorPos(WindowPtr, &newMouseX, &newMouseY);
	currentMouseX = newMouseX;
	currentMouseY = newMouseY;

	if (m_playEnable) {
		// Calculate delta mouse position
		m_deltaMouse = Vector2(currentMouseX - m_screenArea.width / 2.0, currentMouseY - m_screenArea.height / 2.0);
		// Reset the cursor to the center of the window
		glfwSetCursorPos(WindowPtr, m_screenArea.width / 2.0, m_screenArea.height / 2.0);
	}
	else {
		// Calculate delta mouse position based on the previous frame's position
		m_deltaMouse = Vector2(currentMouseX - m_oldMousePos.x, currentMouseY - m_oldMousePos.y);
	}
}

void InputManager::onLateUpdate()
{
	previousKeyStates = currentKeyStates;
	previousMouseStates = currentMouseStates;

	// Reset mouse scroll
	resetMouseScroll();

	if (m_playEnable)
	{
		// Reset the cursor to the center of the window
		glfwSetCursorPos(WindowPtr, m_screenArea.width / 2.0, m_screenArea.height / 2.0);
	}

	// Update old mouse position
	m_oldMousePos = Vector2(currentMouseX, currentMouseY);
}

void InputManager::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	scrollX = xoffset;
	scrollY = yoffset;

	if (ImGui::GetIO().WantCaptureMouse)
	{
		ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	}
}

void InputManager::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Key translatedKey = static_cast<Key>(key);

	if (ImGui::GetIO().WantCaptureKeyboard)
	{
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	}

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
	if (ImGui::GetIO().WantCaptureMouse)
	{
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
		
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
		{
			ImGuiWindow* hoveredWindow = ImGui::GetCurrentContext()->HoveredWindow;

			if (string(hoveredWindow->Name) != "Scene" &&
				std::string(hoveredWindow->Name) != "Game")
			{
				currentMouseStates[translatedButton] = false;
				return;
			}
		}
	}

	if (action == GLFW_PRESS && !currentMouseStates[translatedButton]) {
		currentMouseStates[translatedButton] = true;
	}
	else if (action == GLFW_RELEASE) {
		currentMouseStates[translatedButton] = false;
	}
}

//void InputManager::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
//{
//	currentMouseX = xpos;
//	currentMouseY = ypos;
//}