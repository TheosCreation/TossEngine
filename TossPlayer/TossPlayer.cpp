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
    tossEngine.StartCoroutine(resourceManager.loadResourcesFromFile("Resources/Resources.json"));

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

    onCreate();

    //run funcs while window open
    while (tossEngine.GetWindow()->shouldClose() == false)
    {
        onUpdateInternal();
        tossEngine.PollEvents();
    }

    
    onQuit();
    tossEngine.CleanUp();
}

void TossPlayer::onCreate()
{
    auto scene = std::make_shared<Scene>(m_playerSettings->firstSceneToOpen);
    TossEngine::GetInstance().OpenScene(scene);
}

void TossPlayer::onUpdateInternal()
{
    auto& tossEngine = TossEngine::GetInstance();
    InputManager& inputManager = InputManager::GetInstance();
    auto scene = tossEngine.getCurrentScene();
    if (!scene) return;


    // delta time
    m_currentTime = TossEngine::GetTime();
    float deltaTimeInternal = m_currentTime - m_previousTime;
    Time::UpdateDeltaTime(deltaTimeInternal);
    m_previousTime = m_currentTime;


    // Accumulate time
    m_accumulatedTime += deltaTimeInternal;
    inputManager.onUpdate();

    while (m_accumulatedTime >= Time::FixedDeltaTime)
    {
        Physics::GetInstance().Update();
        scene->onFixedUpdate();
        m_accumulatedTime -= Time::FixedDeltaTime;
    }


    scene->onUpdate();
    scene->onLateUpdate();
    Physics::GetInstance().UpdateInternal();

    //float RenderTime_Begin = TossEngine::GetTime();

    scene->onGraphicsUpdate();

    //float RenderTime_End = TossEngine::GetTime();

    // Render to window
    tossEngine.GetWindow()->present();
    inputManager.onLateUpdate();
}

void TossPlayer::onQuit()
{
    TossEngine::GetInstance().getCurrentScene()->onQuit();
    ResourceManager::GetInstance().CleanUp();
    Physics::GetInstance().UnLoadPrefabWorld();
}

void TossPlayer::onResize(Vector2 size)
{
    Resizable::onResize(size);

    TossEngine::GetInstance().getCurrentScene()->onResize(size);

    GeometryBuffer::GetInstance().Resize(size);
}
