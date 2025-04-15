#include "TossPlayer.h"
#include "TossEngine.h"
#include "Window.h"
#include "TossPlayerSettings.h"
#include "GraphicsEngine.h"
#include "ISelectable.h"
#include <imgui.h>
#include <glfw3.h>

TossPlayer::TossPlayer()
{
    m_playerSettings = std::make_unique<TossPlayerSettings>();
    if (!m_playerSettings->LoadFromFile("PlayerSettings.json"))
    {
        abort = true;
    }

    Vector2 windowSize = m_playerSettings->windowSize;


    auto& tossEngine = TossEngine::GetInstance();
    tossEngine.Init();
    tossEngine.TryCreateWindow(this, windowSize, "TossPlayer");
    tossEngine.LoadScripts();

    initRandomSeed();

    AudioEngine::GetInstance().Init();

    Physics::GetInstance().SetGravity(m_playerSettings->gravity);
    Physics::GetInstance().LoadPrefabWorld();

    ResourceManager& resourceManager = ResourceManager::GetInstance();
    tossEngine.StartCoroutine(resourceManager.loadResourceDesc("Resources/Resources.json"));
    tossEngine.StartCoroutine(resourceManager.createResourcesFromDescs());

    auto& graphicsEngine = GraphicsEngine::GetInstance();
    graphicsEngine.Init(m_playerSettings);
    graphicsEngine.setViewport(windowSize);
    graphicsEngine.setDepthFunc(DepthType::Less);
    graphicsEngine.setBlendFunc(BlendType::SrcAlpha, BlendType::OneMinusSrcAlpha);
    graphicsEngine.setFaceCulling(CullType::BackFace);
    graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
    graphicsEngine.setMultiSampling(true);

    auto& inputManager = InputManager::GetInstance();
    inputManager.Init(m_playerSettings);
    inputManager.setScreenArea(windowSize);

    GeometryBuffer::GetInstance().Init(windowSize);
}

TossPlayer::~TossPlayer()
{
}

void TossPlayer::run()
{
    auto& tossEngine = TossEngine::GetInstance();
    if (abort)
    {
        tossEngine.GetWindow()->close(); 
        return;
    }

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

void TossPlayer::onCreate()
{
    auto scene = std::make_shared<Scene>(m_playerSettings->firstSceneToOpen);
    SetScene(scene, false);
}

void TossPlayer::onCreateLate()
{
}


void TossPlayer::onUpdateInternal()
{
    auto& tossEngine = TossEngine::GetInstance();

    // delta time
    m_currentTime = tossEngine.GetTime();
    Time::UpdateDeltaTime(m_currentTime - m_previousTime);
    m_previousTime = m_currentTime;

    // Accumulate time
    m_accumulatedTime += Time::DeltaTime;

    // Perform updates
    while (m_accumulatedTime >= m_fixedTimeStep)
    {
        float fixedDeltaTime = m_currentTime - m_previousFixedUpdateTime;
        m_previousFixedUpdateTime = m_currentTime;
        m_currentScene->onFixedUpdate();
        m_accumulatedTime -= m_fixedTimeStep;
    }

    m_currentScene->onUpdate();
    m_currentScene->onLateUpdate();

    double RenderTime_Begin = (double)glfwGetTime();

    m_currentScene->onGraphicsUpdate();

    double RenderTime_End = (double)glfwGetTime();

    // Render to window
    tossEngine.GetWindow()->present();
}

void TossPlayer::onQuit()
{
    ResourceManager::GetInstance().CleanUp();
    Physics::GetInstance().UnLoadPrefabWorld();
    m_currentScene->onQuit();
}

void TossPlayer::SetScene(ScenePtr _scene, bool skipCreate)
{
    // if there is a current scene, call onQuit
    if (m_currentScene != nullptr)
    {
        m_currentScene->onQuit();
    }

    // set the current scene to the new scene
    m_currentScene = _scene;
    if (!skipCreate)
    {
        m_currentScene->onCreate();
        m_currentScene->onCreateLate();
    }

    m_currentScene->onStart();
    m_currentScene->onLateStart();
}

void TossPlayer::onResize(Vector2 size)
{
    Resizable::onResize(size);

    m_currentScene->onResize(size);

    GeometryBuffer::GetInstance().Resize(size);
}
