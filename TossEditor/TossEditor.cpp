#include "TossEditor.h"
#include "TossEngine.h"
#include "Window.h"
#include "Game.h"
#include "ProjectSettings.h"
#include "GraphicsEngine.h"
#include "EditorPlayer.h"
#include "ISelectable.h"
#include <imgui.h>

TossEditor::TossEditor()
{
    m_projectSettings = std::make_unique<ProjectSettings>();
    m_projectSettings->LoadFromFile("ProjectSettings.json");

    Vector2 windowSize = Vector2(800, 800);

    auto& tossEngine = TossEngine::GetInstance();
    tossEngine.Init();
    tossEngine.TryCreateWindow(this, windowSize, "TossEditor");


    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.Init(m_projectSettings);
    graphicsEngine.setViewport(windowSize);
    graphicsEngine.setDepthFunc(DepthType::Less);
    graphicsEngine.setBlendFunc(BlendType::SrcAlpha, BlendType::OneMinusSrcAlpha);
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setMultiSampling(true);

    auto& inputManager = InputManager::GetInstance();
    inputManager.Init(m_projectSettings);
    inputManager.setScreenArea(windowSize);

    GeometryBuffer::GetInstance().Init(windowSize);
    AudioEngine::GetInstance().Init();


    m_sceneViewFrameBuffer = std::make_shared<Framebuffer>(tossEngine.GetWindow()->getInnerSize());
    m_gameViewFrameBuffer = std::make_shared<Framebuffer>(tossEngine.GetWindow()->getInnerSize());
}

TossEditor::~TossEditor()
{
}

void TossEditor::run()
{
    auto& tossEngine = TossEngine::GetInstance();
    tossEngine.LoadGenericResources();

    onCreate();
    onCreateLate();

    //run funcs while window open
    while (tossEngine.GetWindow()->shouldClose() == false)
    {
        tossEngine.PollEvents();
        onUpdateInternal();
    }

    
    onQuit();
    tossEngine.CleanUp();
}

void TossEditor::onCreate()
{
    string filePath = m_projectSettings->lastKnownOpenScenePath;

    if (!filePath.empty()) // If a file was selected
    {
        auto scene = std::make_shared<Scene>(filePath);
        scene->SetWindowFrameBuffer(m_sceneViewFrameBuffer);
        OpenScene(scene);
    }

    m_player = std::make_unique<EditorPlayer>(this);
    m_player->onCreate();
}

void TossEditor::onCreateLate()
{
}

void TossEditor::DeleteSelected()
{
    if (selectedSelectable != nullptr)
    {
        if (selectedSelectable->Delete(false))
        {
            selectedSelectable = nullptr;
        }
    }
}

void TossEditor::Exit()
{
    TossEngine::GetInstance().GetWindow()->close();
}

void TossEditor::onUpdateInternal()
{
    TossEngine& tossEngine = TossEngine::GetInstance();
    GraphicsEngine& graphicsEngine = GraphicsEngine::GetInstance();
    ResourceManager& resourceManager = ResourceManager::GetInstance();

    // delta time
    m_currentTime = tossEngine.GetTime();
    float deltaTime = m_currentTime - m_previousTime;
    m_previousTime = m_currentTime;


    // Accumulate time
    m_accumulatedTime += deltaTime;

    if (m_game != nullptr)
    {
        while (m_accumulatedTime >= m_fixedTimeStep)
        {
            float fixedDeltaTime = m_currentTime - m_previousFixedUpdateTime;
            m_previousFixedUpdateTime = m_currentTime;
            m_game->onFixedUpdate(fixedDeltaTime);
            m_accumulatedTime -= m_fixedTimeStep;
        }

        // player update
        //m_player->Update(deltaTime);
        m_game->onUpdate(deltaTime);
        m_game->onLateUpdate(deltaTime);
    }
    else
    {
        InputManager& inputManager = InputManager::GetInstance();
        inputManager.onUpdate();

        // player update
        m_player->Update(deltaTime);
        if (m_currentScene)
        {
            m_currentScene->onUpdateInternal();
        }

        inputManager.onLateUpdate();
    }


    graphicsEngine.clear(glm::vec4(0, 0, 0, 1)); //clear the existing stuff first is a must
    graphicsEngine.createImGuiFrame();
    ImGui::SetCurrentContext(graphicsEngine.getImGuiContext());

    float menuBarHeight = ImGui::GetFrameHeightWithSpacing();  // More accurate with spacing
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    ImVec2 dockPos = viewport->Pos;
    dockPos.y += menuBarHeight;  // Offset by the menu bar height
    
    ImVec2 dockSize = viewport->Size;
    dockSize.y -= menuBarHeight; // Reduce the height accordingly
    
    ImGui::SetNextWindowPos(dockPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(dockSize, ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);
    
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    const ImGuiWindowClass* window_class = nullptr;
    
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags, window_class);
    ImGui::End();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "CTRL+0"))
            {
                OpenSceneViaFileSystem();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save", "CTRL+S"))
            {
                Save();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "ESC"))
            {
                Exit();
            }
            ImGui::EndMenu();
        }
        ImGui::SameLine(0, 200);
        if (ImGui::Button("Play"))
        {
            if (!m_game && m_currentScene)
            {
                auto scene = std::make_shared<Scene>(m_currentScene->GetFilePath());
                scene->SetWindowFrameBuffer(m_gameViewFrameBuffer);
                m_game = new Game(m_projectSettings);
                m_game->SetScene(scene);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
        {
            if (m_game)
            {
                delete m_game;  // Clean up memory
                m_game = nullptr;
            }
        }
        ImGui::EndMainMenuBar();
    }

    
    ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        // Get the available size for the Scene window.
        ImVec2 availSize = ImGui::GetContentRegionAvail();

        // Convert to your Vector2 type (assuming Vector2 takes width and height).
        Vector2 newSize(availSize.x, availSize.y);
        if (m_game != nullptr)
        {
            // Resize the scene's framebuffer to match the current window size.
            m_game->getScene()->onResize(newSize);

            // Now update the scene rendering with the current camera.
            m_game->onGraphicsUpdate();

            // Get the updated texture after rendering.
            ImTextureID gameViewTexture = (ImTextureID)m_gameViewFrameBuffer->RenderTexture->getId();

            // Display the rendered scene scaled to the available region.
            ImGui::Image(gameViewTexture, availSize,
                ImVec2{ 0.f, 1.f },  // UV0
                ImVec2{ 1.f, 0.f }   // UV1 (flipped vertically if needed)
            );
        }
    }
    ImGui::End();

    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        // Get the available size for the Scene window.
        ImVec2 availSize = ImGui::GetContentRegionAvail();

        // Convert to your Vector2 type (assuming Vector2 takes width and height).
        Vector2 newSize(availSize.x, availSize.y);
        if (m_currentScene)
        {
            // Resize the scene's framebuffer to match the current window size.
            onResize(newSize);

            // Now update the scene rendering with the current camera.
            m_currentScene->onGraphicsUpdate(m_player->getCamera());

            // Get the updated texture after rendering.
            ImTextureID sceneViewTexture = (ImTextureID)m_sceneViewFrameBuffer->RenderTexture->getId();

            // Display the rendered scene scaled to the available region.
            ImGui::Image(sceneViewTexture, availSize,
                ImVec2{ 0.f, 1.f },  // UV0
                ImVec2{ 1.f, 0.f }   // UV1 (flipped vertically if needed)
            );
        }
    }
    ImGui::End();
    
    if (ImGui::Begin("Settings"))
    {
        static const char* items[]{ "Deferred Rendering","Forward" }; static int Selecteditem = (int)m_projectSettings->renderingPath;
        if (ImGui::Combo("Rendering Path", &Selecteditem, items, IM_ARRAYSIZE(items)))
        {
            RenderingPath selectedPath = static_cast<RenderingPath>(Selecteditem);
            Debug::Log("Rendering Path changed to option: " + ToString(selectedPath));
            graphicsEngine.setRenderingPath(selectedPath);
            m_projectSettings->renderingPath = selectedPath;
        }
    }
    ImGui::End();

    if (ImGui::Begin("Inspector"))
    {
        if (selectedSelectable)
        {
            // Display the selected object's name and transform.
            selectedSelectable->OnInspectorGUI();
        }
        else
        {
            ImGui::Text("Nothing selected.");
        }
        ImGui::End();
    }

    if (ImGui::Begin("Hierarchy"))
    {
        // Right-click context menu on the window background
        if (ImGui::BeginPopupContextWindow("Hierarchy Context"))
        {
            if (ImGui::MenuItem("Add Empty GameObject"))
            {
                m_currentScene->getObjectManager()->createGameObject<GameObject>();
            }
            ImGui::EndPopup();
        }

        // Display root game objects.
        if (m_currentScene != nullptr)
        {
            for (const auto& pair : m_currentScene->getObjectManager()->m_gameObjects)
            {
                // Only show root game objects.
                if (pair.second->m_transform.parent == nullptr)
                {
                    ShowGameObjectNode(pair.second);
                }
            }
        }
        ImGui::End();
    }

    if (renamingGameObject != nullptr)
    {
        ImGui::OpenPopup("Rename GameObject");
    }
    // Place the modal popup outside of any window blocks
    if (ImGui::BeginPopupModal("Rename GameObject", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("New name", renameBuffer, sizeof(renameBuffer));
        if (ImGui::Button("OK"))
        {
            if (renamingGameObject)
            {
                renamingGameObject->name = renameBuffer;
            }
            renamingGameObject = nullptr;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            renamingGameObject = nullptr;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::Begin("Assets")) {
        const auto& resources = resourceManager.GetAllResources();

        // Iterate over the resource map and display each resource's unique ID
        for (const auto& [uniqueID, resource] : resources) {
            if (uniqueID == "") continue;

            bool isSelected = (resource == resourceManager.GetSelectedResource());
            if (ImGui::Selectable(uniqueID.c_str(), isSelected)) {
                resourceManager.SetSelectedResource(resource);
            }
        }
    }
    ImGui::End();

    Debug::DrawConsole();

    graphicsEngine.renderImGuiFrame();
    
    
    double RenderTime_End = (double)glfwGetTime();
    
    // Render to window
    tossEngine.GetWindow()->present();
}

void TossEditor::onQuit()
{
    if (m_currentScene)
    {
        m_currentScene->onQuit();
    }
}

void TossEditor::ShowGameObjectNode(GameObject* gameObject)
{
    ImGuiTreeNodeFlags flags = (gameObject->m_transform.children.empty() ? ImGuiTreeNodeFlags_Leaf : 0);
    if (gameObject == selectedSelectable)
        flags |= ImGuiTreeNodeFlags_Selected;

    bool open = ImGui::TreeNodeEx(gameObject->name.c_str(), flags);
    if (ImGui::IsItemClicked())
    {
        selectedSelectable = gameObject;
    }

    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("DND_GAMEOBJECT", &gameObject, sizeof(GameObject*));
        ImGui::Text("Dragging %s", gameObject->name.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_GAMEOBJECT"))
        {
            GameObject* droppedGameObject = *(GameObject**)payload->Data;
            if (droppedGameObject != gameObject)
            {
                droppedGameObject->m_transform.SetParent(&gameObject->m_transform);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Rename"))
        {
            renamingGameObject = gameObject;
            strncpy_s(renameBuffer, sizeof(renameBuffer), gameObject->name.c_str(), _TRUNCATE);
            Debug::Log("Rename selected for " + gameObject->name);
        }
        if (ImGui::MenuItem("Delete"))
        {
            if (gameObject->m_transform.parent)
            {
                gameObject->m_transform.SetParent(nullptr);
            }
            gameObject->Delete();
            if (selectedSelectable == gameObject)
                selectedSelectable = nullptr;
        }
        ImGui::EndPopup();
    }

    if (open)
    {
        for (Transform* child : gameObject->m_transform.children)
        {
            ShowGameObjectNode(child->gameObject);
        }
        ImGui::TreePop();
    }
}

void TossEditor::onResize(Vector2 size)
{
    Resizable::onResize(size);

    m_player->getCamera()->setScreenArea(size);

    if (m_currentScene)
    {
        m_currentScene->onResize(size);
    }

    GeometryBuffer::GetInstance().Resize(size);
}

void TossEditor::Undo()
{
}

void TossEditor::Save()
{
    m_projectSettings->SaveToFile("ProjectSettings.json");
    if (m_currentScene)
    {
        m_currentScene->Save();
    }
}

void TossEditor::OpenSceneViaFileSystem()
{
    string filePath = TossEngine::GetInstance().openFileDialog("*.json");

    if (!filePath.empty()) // If a file was selected
    {
        auto scene = std::make_shared<Scene>(filePath);
        OpenScene(scene);
    }
}

void TossEditor::OpenScene(shared_ptr<Scene> _scene)
{
    // if there is a current scene, call onQuit
    if (m_currentScene != nullptr)
    {
        m_currentScene->onQuit();
    }


    // set the current scene to the new scene
    m_currentScene = std::move(_scene);
    m_projectSettings->lastKnownOpenScenePath = m_currentScene->GetFilePath();
    m_currentScene->onCreate();
    m_currentScene->onCreateLate();
}
