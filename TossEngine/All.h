/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : All.h
Description : header file contains includes to all the core frameworks inside the engine
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#pragma once
//Utils
#include "Utils.h"
#include "Math.h"
#include "Rect.h"

//Core
#include "Window.h"
#include "GraphicsEngine.h"
#include "VertexArrayObject.h"
#include "InputManager.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "Scene.h"
#include "Physics.h"
#include "Framebuffer.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "LayerManager.h"
#include "Renderer.h"
#include "DebugDraw.h"
#include "AudioEngine.h"
#include "GeometryBuffer.h"
#include "CoroutineTask.h"
#include "JsonUtility.h"
#include <imgui.h>

//Resources
#include "ResourceManager.h"
#include "Resource.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Texture2D.h"
#include "TextureCubeMap.h"
#include "HeightMap.h"
#include "Prefab.h"
#include "Font.h"

//Components
#include "ComponentRegistry.h"
#include "Component.h"
#include "Rigidbody.h"
#include "Collider.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "Image.h"
#include "Skybox.h"
#include "Spotlight.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "Text.h"
#include "Button.h"