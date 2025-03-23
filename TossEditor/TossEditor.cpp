#include "TossEditor.h"
#include "TossEngine.h"
#include "Window.h"
#include "Game.h"
#include "ProjectSettings.h"
#include "GraphicsEngine.h"
//#include "InputManager.h"
#include "AudioEngine.h"
#include "MonoIntegration.h"
#include "EditorPlayer.h"
#include "GeometryBuffer.h"
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
    m_player = std::make_unique<EditorPlayer>(this);
    m_player->onCreate();
}

void TossEditor::onCreateLate()
{
}

void TossEditor::Exit()
{
    TossEngine::GetInstance().GetWindow()->close();
}

void TossEditor::onUpdateInternal()
{
    auto& tossEngine = TossEngine::GetInstance();


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
        
        m_game->onUpdate(deltaTime);
        m_game->onLateUpdate(deltaTime);
    }
    else
    {
        auto& inputManager = InputManager::GetInstance();
        inputManager.onUpdate();

        // player update
        m_player->Update(deltaTime);

        inputManager.onLateUpdate();
    }


    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.clear(glm::vec4(0, 0, 0, 1)); //clear the existing stuff first is a must
    graphicsEngine.createImGuiFrame();


    if (m_game != nullptr)
    {
        m_game->onGraphicsUpdate();
    }
    else
    {
        if (m_currentScene)
        {
            m_currentScene->onGraphicsUpdate(m_player->getCamera()); //Render the scene
        }
    }

    ImGui::SetCurrentContext(graphicsEngine.getImGuiContext()); 
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

    //float menuBarHeight = ImGui::GetFrameHeightWithSpacing();  // More accurate with spacing
    //ImGuiViewport* viewport = ImGui::GetMainViewport();
    //
    //ImVec2 dockPos = viewport->Pos;
    //dockPos.y += menuBarHeight;  // Offset by the menu bar height
    //
    //ImVec2 dockSize = viewport->Size;
    //dockSize.y -= menuBarHeight; // Reduce the height accordingly
    //
    //ImGui::SetNextWindowPos(dockPos, ImGuiCond_Always);
    //ImGui::SetNextWindowSize(dockSize, ImGuiCond_Always);
    //ImGui::SetNextWindowViewport(viewport->ID);
    //
    //ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
    //    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
    //ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    //ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    //ImGui::Begin("Main DockSpace_BelowMenu", nullptr, window_flags);
    //ImGui::PopStyleVar(3);
    //
    //ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    //const ImGuiWindowClass* window_class = nullptr;
    //
    //ImGuiID dockspace_id = ImGui::GetID("MyDockSpace_BelowMenu");
    //ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags, window_class);
    //ImGui::End();

    
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
    }
    ImGui::End();
    if (ImGui::Begin("Hierarchy"))
    {
    }
    ImGui::End();
    if (ImGui::Begin("Assets"))
    {
    }
    ImGui::End();

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
    m_currentScene->onCreate();
    m_currentScene->onCreateLate();
}
