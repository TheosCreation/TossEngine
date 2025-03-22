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

    MonoIntegration::InitializeMono();
    GeometryBuffer::GetInstance().Init(windowSize);

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

    LightManager::GetInstance().Init();
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
    m_player = std::make_unique<EditorPlayer>();
    m_player->onCreate();
}

void TossEditor::onCreateLate()
{
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
    if (ImGui::Begin("Settings"))
    {
        if (ImGui::Button("Play"))
        {
            if (!m_game && m_currentScene) 
            {
                m_game = new Game(m_projectSettings);
                m_game->SetScene(m_currentScene, true);
            }
        }

        if (ImGui::Button("Stop"))
        {
            if (m_game)
            {
                delete m_game;  // Clean up memory
                m_game = nullptr;
            }
        }

        if (ImGui::Button("LoadScene"))
        {
            string filePath = tossEngine.openFileDialog("*.json");

            if (!filePath.empty()) // If a file was selected
            {
                auto scene = std::make_shared<Scene>(filePath);
                OpenScene(scene);
            }
        }
        
        if (ImGui::Button("Save"))
        {
            Save();
        }

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

    MonoIntegration::ShutdownMono();
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

void TossEditor::OpenScene(shared_ptr<Scene> _scene)
{
    // if there is a current scene, call onQuit
    if (m_currentScene != nullptr)
    {
        m_currentScene->onQuit();
        LightManager::GetInstance().reset();
        ResourceManager::GetInstance().ClearInstancesFromMeshes();
    }
    // set the current scene to the new scene
    m_currentScene = std::move(_scene);
    m_currentScene->onCreate();
    m_currentScene->onCreateLate();
}
