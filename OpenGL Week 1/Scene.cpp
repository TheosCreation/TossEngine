/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2024 Media Design School
File Name : Scene.h
Description : Base class representing a game scene, providing essential methods for rendering and updating the game logic.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
**/

#include "Scene.h"
#include "SSRQuad.h"
#include "GeometryBuffer.h"
#include "AudioEngine.h"
#include "MeshRenderer.h"
#include "Rigidbody.h"

Scene::Scene(Game* game)
{
    gameOwner = game;
    m_postProcessingFramebuffer = std::make_unique<Framebuffer>(gameOwner->getWindow()->getInnerSize());
    m_gameObjectManager = std::make_unique<GameObjectManager>(this);
    m_deferredRenderSSRQ = std::make_unique<SSRQuad>();
    m_postProcessSSRQ = std::make_unique<SSRQuad>();

    // Create a physics world with default gravity
    rp3d::PhysicsWorld::WorldSettings settings;
    settings.gravity = rp3d::Vector3(0, -9.81f, 0);
    m_PhysicsWorld = m_PhysicsCommon.createPhysicsWorld(settings);


    m_PhysicsWorld->setIsDebugRenderingEnabled(true);
    m_PhysicsWorld->setNbIterationsVelocitySolver(10);
    m_PhysicsWorld->setNbIterationsPositionSolver(5);

    //rp3d::DebugRenderer& debugRenderer = m_PhysicsWorld->getDebugRenderer();
    //// Select the contact points and contact normals to be displayed
    //debugRenderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::CONTACT_POINT, true);
    //debugRenderer.setIsDebugItemDisplayed(rp3d::DebugRenderer::DebugItem::CONTACT_NORMAL, true);
}

Scene::~Scene()
{
}

void Scene::onCreate()
{
    auto& resourceManager = ResourceManager::GetInstance();
    auto& lightManager = LightManager::GetInstance();
    auto& graphicsEngine = GraphicsEngine::GetInstance();
    m_gameObjectManager->m_entityFactory->registerEntity<Player>();

    defaultFullscreenShader = graphicsEngine.createShader({
            "ScreenQuad",
            "QuadShader"
        });

    ssrQuadLightingShader = graphicsEngine.createShader({
            "ScreenQuad",
            "SSRLightingShader"
        });

    m_deferredRenderSSRQ->onCreate();
    m_deferredRenderSSRQ->setShader(ssrQuadLightingShader);

    m_postProcessSSRQ->onCreate();
    m_postProcessSSRQ->setShader(defaultFullscreenShader);
    m_postProcessSSRQ->setTexture(m_postProcessingFramebuffer->RenderTexture);

    ShaderPtr skyboxShader = graphicsEngine.createShader({
            "SkyBoxShader",
            "SkyBoxShader"
        });

    m_solidColorMeshShader = graphicsEngine.createShader({
            "SolidColorMesh",
            "SolidColorMesh"
        });

    m_shadowShader = graphicsEngine.createShader({
            "ShadowShader",
            "ShadowShader"
        });

    m_shadowInstancedShader = graphicsEngine.createShader({
            "ShadowShaderInstanced",
            "ShadowShader"
        });

    m_skyboxGeometryShader = graphicsEngine.createShader({
            "SkyBoxShader",
            "GeometryPassSkybox"
        });
    m_meshGeometryShader = graphicsEngine.createShader({
            "MeshShader",
            "GeometryPass"
        });
    m_instancedmeshGeometryShader = graphicsEngine.createShader({
            "InstancedMesh",
            "GeometryPass"
        });

    m_terrainGeometryShader = graphicsEngine.createShader({
            "TerrainShader",
            "GeometryPassTerrian"
        });

    m_solidColorMeshShader = graphicsEngine.createShader({
            "SolidColorMesh",
            "SolidColorMesh"
        });

    m_particleSystemShader = graphicsEngine.createShader({
            "ParticleSystem",
            "ParticleSystem"
        });

    m_computeShader = graphicsEngine.createComputeShader("ComputeParticles");

    //m_meshLightingShader = graphicsEngine.createShader({
    //        "MeshShader",
    //        "MeshLightingShader"
    //    });

    //Creating skybox object
    m_skyBox = std::make_unique<SkyboxEntity>();
    m_skyBox->setGameObjectManager(m_gameObjectManager.get());
    m_skyBox->setMesh(gameOwner->getCubeMesh());
    m_skyBox->setShader(skyboxShader);
    m_skyBox->setGeometryShader(m_skyboxGeometryShader);
    //m_skyBox->setLightingShader(m_meshLightingShader);

    // create a cube map texture and set the texture of the skybox to the cubemap texture
    std::vector<std::string> skyboxCubeMapTextureFilePaths;
    skyboxCubeMapTextureFilePaths.push_back("Resources/Textures/RedEclipse/Right.png");
    skyboxCubeMapTextureFilePaths.push_back("Resources/Textures/RedEclipse/Left.png");
    skyboxCubeMapTextureFilePaths.push_back("Resources/Textures/RedEclipse/Top.png");
    skyboxCubeMapTextureFilePaths.push_back("Resources/Textures/RedEclipse/Bottom.png");
    skyboxCubeMapTextureFilePaths.push_back("Resources/Textures/RedEclipse/Back.png");
    skyboxCubeMapTextureFilePaths.push_back("Resources/Textures/RedEclipse/Front.png");
    TextureCubeMapPtr skyBoxTexture = resourceManager.createCubeMapTextureFromFile(skyboxCubeMapTextureFilePaths);
    m_skyBox->setTexture(skyBoxTexture);

    m_player = m_gameObjectManager->createGameObject<Player>();
    m_player->m_transform.scale = Vector3(0.0f);
    m_player->m_transform.position = Vector3(0.0f, 20.0f, 0.0f);



    Texture2DPtr heightMapTexture = resourceManager.createTexture2DFromFile("Resources/Textures/Heightmap0.jpg");
    Texture2DPtr shipReflectiveMap = resourceManager.createTexture2DFromFile("Resources/Textures/ReflectionMap_White.png");
    Texture2DPtr sciFiSpaceTexture2D = resourceManager.createTexture2DFromFile("Resources/Textures/PolygonSciFiSpace_Texture_01_A.png");
    Texture2DPtr ancientWorldsTexture2D = resourceManager.createTexture2DFromFile("Resources/Textures/PolygonAncientWorlds_Texture_01_A.png");
    Texture2DPtr grassTexture = resourceManager.createTexture2DFromFile("Resources/Textures/Terrain/stone.png");
    Texture2DPtr dirtTexture = resourceManager.createTexture2DFromFile("Resources/Textures/Terrain/dirt.png");
    Texture2DPtr stoneTexture = resourceManager.createTexture2DFromFile("Resources/Textures/Terrain/stone.png");
    Texture2DPtr snowTexture = resourceManager.createTexture2DFromFile("Resources/Textures/Terrain/snow.png");

    MeshPtr fighterShip = resourceManager.createMeshFromFile("Resources/Meshes/Space/SM_Ship_Fighter_02.obj");

    ShaderPtr meshShader = graphicsEngine.createShader({
            "MeshShader",
            "MeshShader"
        });
    ShaderPtr instancedMeshShader = graphicsEngine.createShader({
            "InstancedMesh",
            "MeshShader"
        });

    ShaderPtr terrainShader = graphicsEngine.createShader({
            "TerrainShader",
            "TerrainShader"
        });

    MeshPtr statueMesh = resourceManager.createMeshFromFile("Resources/Meshes/SM_Prop_Statue_01.obj");
    float spacing = 50.0f;
    for (int row = -4; row < 4; ++row) {
        for (int col = -4; col < 4; ++col) {
            // Calculate the position of the current tree based on the grid and spacing
            Vector3 position = Vector3(col * spacing, 0, row * spacing);

            if (position == Vector3(0.0f)) break;

            // Generate random rotation angles
            float angleY = randomNumber(360.0f);

            // Add the tree instance with random rotations
            statueMesh->addInstance(position, Vector3(0.2f), Vector3(0, angleY, 0));
        }
    }
    //Init instance buffer
    statueMesh->initInstanceBuffer();

    {
        m_ship = m_gameObjectManager->createGameObject<GameObject>();
        m_ship->m_transform.scale = Vector3(0.05f);
        m_ship->m_transform.position = Vector3(0, 40, 0);
        MeshRenderer* renderer = m_ship->addComponent<MeshRenderer>();
        renderer->SetShininess(64.0f);
        renderer->SetTexture(sciFiSpaceTexture2D);
        renderer->SetReflectiveMapTexture(shipReflectiveMap);
        renderer->SetMesh(fighterShip);
        renderer->SetShader(meshShader);
        renderer->SetShadowShader(m_shadowShader);
        renderer->SetGeometryShader(m_meshGeometryShader);
    }

    //HeightMapInfo buildInfo = { "Resources/Heightmaps/Heightmap0.raw", 256, 256, 4.0f };
    //HeightMapPtr heightmap = resourceManager.createHeightMap(buildInfo);
    //
    //m_terrain = m_gameObjectManager->createGameObject<TerrainEntity>();
    //m_terrain->generateTerrainMesh(heightmap);
    //m_terrain->m_transform.position = Vector3(0, -20, 0);
    //m_terrain->setTexture(grassTexture);
    //m_terrain->setTexture1(dirtTexture);
    //m_terrain->setTexture2(stoneTexture);
    //m_terrain->setTexture3(snowTexture);
    //m_terrain->setHeightMap(heightMapTexture);
    //m_terrain->setShader(terrainShader);
    //m_terrain->setShadowShader(m_shadowShader);
    //m_terrain->setGeometryShader(m_terrainGeometryShader);
    //
    ////Creating instanced tree obj
    {
        auto statues = m_gameObjectManager->createGameObject<GameObject>();
        auto renderer = statues->addComponent<MeshRenderer>();
        renderer->SetShininess(32.0f);
        renderer->SetTexture(ancientWorldsTexture2D);
        renderer->SetShader(instancedMeshShader);
        renderer->SetMesh(statueMesh);
        renderer->SetReflectiveMapTexture(shipReflectiveMap);
        renderer->SetShadowShader(m_shadowInstancedShader);
        renderer->SetGeometryShader(m_instancedmeshGeometryShader);
    }

    
    // Create and initialize a DirectionalLight struct
    DirectionalLight directionalLight1;
    directionalLight1.Direction = Vector3(0.0f, -1.0f, -0.5f);
    directionalLight1.Color = Vector3(0.6f);
    directionalLight1.SpecularStrength = 0.1f;
    lightManager.createDirectionalLight(directionalLight1);

    // Create and initialize a DirectionalLight struct
    DirectionalLight directionalLight2;
    directionalLight2.Direction = Vector3(0.0f, -1.0f, 0.5f);
    directionalLight2.Color = Vector3(0.6f);
    directionalLight2.SpecularStrength = 0.1f;
    lightManager.createDirectionalLight(directionalLight2);

    // Create and initialize SpotLight struct
    SpotLight spotLight;
    spotLight.Position = Vector3(0.0f);
    spotLight.Direction = Vector3(0.0f, 0.0f, -1.0f);
    spotLight.Color = Color::White;
    spotLight.SpecularStrength = 1.0f;
    spotLight.CutOff = glm::cos(glm::radians(25.0f));
    spotLight.OuterCutOff = glm::cos(glm::radians(35.0f));
    spotLight.AttenuationConstant = 1.0f;
    spotLight.AttenuationLinear = 0.014f;
    spotLight.AttenuationExponent = 0.0007f;
    lightManager.createSpotLight(spotLight);
    lightManager.setSpotlightStatus(false);

    float pointLightSpacing = 30.0f;
    // Initialize 2 point lights
    for (int i = 0; i < 4; i++) {
        // Create a new point light entity
        auto pointLightObject = m_gameObjectManager->createGameObject<GameObject>();
        MeshRenderer* meshRenderer = pointLightObject->addComponent<MeshRenderer>();
        meshRenderer->SetAlpha(0.5f);

        // Randomly set color to either red or blue
        int randomColorChoice = (int)randomNumber(2.0f); // Generates 0 or 1
        Vector3 lightColor = (randomColorChoice == 0) ? Color::Red * 2.0f : Color::Blue * 2.0f;
        meshRenderer->SetColor(lightColor);

        // Calculate the position based on row and column, center the grid around (0,0)
        float xPosition = i * pointLightSpacing; // Center horizontally
        float yPosition = 15.0f; // Fixed Y position
        float zPosition = 0;
        pointLightObject->m_transform.position = Vector3(xPosition, yPosition, zPosition);
        pointLightObject->m_transform.scale = Vector3(3.0f);

        // Set mesh and shaders
        meshRenderer->SetMesh(gameOwner->getSphereMesh());
        meshRenderer->SetShader(meshShader);
        meshRenderer->SetShadowShader(m_shadowShader);
        meshRenderer->SetGeometryShader(m_meshGeometryShader);

        // Configure point light properties
        PointLight pointLight;
        pointLight.Position = pointLightObject->m_transform.position;
        pointLight.Color = lightColor;
        pointLight.SpecularStrength = 1.0f;
        pointLight.AttenuationConstant = 1.0f;
        pointLight.AttenuationLinear = 0.022f;
        pointLight.AttenuationExponent = 0.0019f;
        pointLight.Radius = 200.0f;

        // Add the point light to the light manager
        lightManager.createPointLight(pointLight);
    }

    for (int i = 0; i < 30; ++i) {
        auto physicsSphere = m_gameObjectManager->createGameObject<GameObject>();

        // Adjusting position based on index to avoid overlapping spheres
        float offset = i * 8.0f; // Adjust this to control the distance between the spheres
        physicsSphere->m_transform.position = Vector3(0, 10 + offset, 0.1f);
        physicsSphere->m_transform.scale = Vector3(3.0f);

        // Mesh and shader setup
        auto meshRenderer = physicsSphere->addComponent<MeshRenderer>();
        meshRenderer->SetColor(Color::Black);
        meshRenderer->SetMesh(gameOwner->getSphereMesh());
        meshRenderer->SetShader(meshShader);
        meshRenderer->SetShadowShader(m_shadowShader);
        meshRenderer->SetGeometryShader(m_meshGeometryShader);

        // Rigidbody setup with sphere collider
        auto rb = physicsSphere->addComponent<Rigidbody>();
        rb->SetSphereCollider(3.0f);
    }

    {
        // Create the ground
        auto physicsCube = m_gameObjectManager->createGameObject<GameObject>();
        physicsCube->m_transform.position = Vector3(0, -5, 0);
        physicsCube->m_transform.scale = Vector3(100.0f, 0.5f, 100.0f);

        auto meshRenderer = physicsCube->addComponent<MeshRenderer>();
        meshRenderer->SetColor(Color::White);
        meshRenderer->SetMesh(gameOwner->getCubeMesh());
        meshRenderer->SetShader(meshShader);
        meshRenderer->SetShadowShader(m_shadowShader);
        meshRenderer->SetGeometryShader(m_meshGeometryShader);

        auto rb = physicsCube->addComponent<Rigidbody>();
        rb->SetBoxCollider(Vector3(100.0f, 0.5f, 100.0f));
        rb->SetBodyType(BodyType::Static);
    }

    // Create four walls to hold the balls
    {
        // Wall dimensions
        float wallHeight = 10.0f; // Height of the walls
        float wallThickness = 0.2f; // Thickness of the walls
        float areaSize = 100.0f; // Size of the area (assuming a square area)

        // Wall positions
        Vector3 wallPositions[] = {
            Vector3(0, wallHeight / 2 - 5, areaSize / 2), // Front wall
            Vector3(0, wallHeight / 2 - 5, -areaSize / 2), // Back wall
            Vector3(areaSize / 2, wallHeight / 2 - 5, 0),  // Right wall
            Vector3(-areaSize / 2, wallHeight / 2 - 5, 0)  // Left wall
        };

        // Wall scales
        Vector3 wallScales[] = {
            Vector3(areaSize, wallHeight, wallThickness), // Front and back walls
            Vector3(wallThickness, wallHeight, areaSize)  // Right and left walls
        };

        for (int i = 0; i < 4; ++i) {
            auto wall = m_gameObjectManager->createGameObject<GameObject>();
            wall->m_transform.position = wallPositions[i];
            wall->m_transform.scale = wallScales[i < 2 ? 0 : 1]; // Use appropriate scale for front/back vs right/left walls

            auto meshRenderer = wall->addComponent<MeshRenderer>();
            meshRenderer->SetColor(Color::Gray); // Set a different color for walls
            meshRenderer->SetMesh(gameOwner->getCubeMesh());
            meshRenderer->SetShader(m_solidColorMeshShader);
            meshRenderer->SetShadowShader(m_shadowShader);
            meshRenderer->SetGeometryShader(m_meshGeometryShader);

            auto rb = wall->addComponent<Rigidbody>();
            rb->SetBoxCollider(wall->m_transform.scale); // Collider size matches the wall scale
            rb->SetBodyType(BodyType::Static); // Walls are static
        }
    }

    //{
    //    auto newGameObject = m_gameObjectManager->createGameObject<GameObject>();
    //    MeshRenderer* renderer = newGameObject->addComponent<MeshRenderer>();
    //    renderer->SetMesh(fighterShip);
    //    renderer->SetShader(meshShader);
    //    renderer->SetTexture(sciFiSpaceTexture2D);
    //    renderer->SetReflectiveMapTexture(shipReflectiveMap);
    //    renderer->SetShadowShader(m_shadowShader);
    //    renderer->SetGeometryShader(m_meshGeometryShader);
    //}
    //m_entitySystem->loadEntitiesFromFile("Scenes/Scene1.json");
}

void Scene::onCreateLate()
{
    for (auto& camera : m_gameObjectManager->getCameras())
    {
        // Set the screen area for all cameras
        camera->setScreenArea(gameOwner->getWindow()->getInnerSize());
    }
}


void Scene::onUpdate(float deltaTime)
{
    if(deltaTime <= 0.0f) return;

    m_gameObjectManager->onUpdate(deltaTime);

    m_elapsedSeconds += deltaTime;

    // Parameters for the circular path
    float radius = 50.0f; // Radius of the circle
    float speed = 1.0f;   // Speed of the ship's movement

    // Calculate the position on the circular path
    float angle = m_elapsedSeconds * speed; // Angle in radians
    float x = radius * cos(angle);
    float z = radius * sin(angle);

    // Update the ship's position
    m_ship->m_transform.position = Vector3(x, 50.0f, z);

    // Make the ship face the direction it's moving by calculating the forward vector
    glm::vec3 forward = glm::normalize(glm::vec3(-sin(angle), 0.0f, cos(angle)));

    // Convert the forward vector to a rotation quaternion
    Quaternion shipRotation = QuaternionUtils::LookAt(forward, glm::vec3(0.0f, 1.0f, 0.0f));

    // Set the ship's rotation
    m_ship->m_transform.rotation = shipRotation;
}

void Scene::onFixedUpdate(float fixedDeltaTime)
{
    if (fixedDeltaTime <= 0.0f) return;

    m_gameObjectManager->onFixedUpdate(fixedDeltaTime); 
    m_PhysicsWorld->update(fixedDeltaTime);
}

void Scene::onLateUpdate(float deltaTime)
{
    m_gameObjectManager->onLateUpdate(deltaTime);
}

void Scene::onGraphicsUpdate()
{
    auto& lightManager = LightManager::GetInstance();
    auto& graphicsEngine = GraphicsEngine::GetInstance();

    // Populate the uniform data struct
    uniformData.currentTime = gameOwner->GetCurrentTime();
    for (auto& camera : m_gameObjectManager->getCameras())
    {
        if (camera->getCameraType() == CameraType::Perspective)
        {
            camera->getViewMatrix(uniformData.viewMatrix);
            camera->getProjectionMatrix(uniformData.projectionMatrix);
            uniformData.cameraPosition = camera->getPosition();
            lightManager.setSpotlightPosition(uniformData.cameraPosition);
            lightManager.setSpotlightDirection(camera->getFacingDirection());
        }
        else
        {
            camera->getViewMatrix(uniformData.uiViewMatrix);
            camera->getProjectionMatrix(uniformData.uiProjectionMatrix);
        }
    }

    // Example of Defered Rendering Pipeline
    if (graphicsEngine.getRenderingPath() == RenderingPath::Deferred)
    {
        //Geometry Pass
        auto& geometryBuffer = GeometryBuffer::GetInstance();
        geometryBuffer.Bind();
        m_gameObjectManager->onGeometryPass(uniformData); // Render opaque objects
        m_gameObjectManager->Render(uniformData);
        geometryBuffer.UnBind();


        // Shadow Pass: Render shadows for directional lights
        for (uint i = 0; i < lightManager.getDirectionalLightCount(); i++)
        {
            lightManager.BindShadowMap(i);
            m_gameObjectManager->onShadowPass(i); // Render shadow maps
            lightManager.UnBindShadowMap(i);
        }
        graphicsEngine.setViewport(gameOwner->getWindow()->getInnerSize());


        graphicsEngine.setShader(ssrQuadLightingShader);
        // Lighting Pass: Apply lighting using G-buffer data
        // Populate the shader with geometry buffer information for the lighting pass
        GeometryBuffer::GetInstance().PopulateShader(ssrQuadLightingShader);

        // Apply lighting settings using the LightManager
        LightManager::GetInstance().applyLighting(ssrQuadLightingShader);

        // Apply shadows
        auto& lightManager = LightManager::GetInstance();
        lightManager.applyShadows(ssrQuadLightingShader);

        // Render the screenspace quad using the lighting, shadow and geometry data
        m_deferredRenderSSRQ->onGraphicsUpdate(uniformData);
        geometryBuffer.WriteDepth();

        // Render the transparent objects after
        m_gameObjectManager->onTransparencyPass(uniformData);
        m_skyBox->onGraphicsUpdate(uniformData);

        //cant seem to get post process to work maybe later
    }
    
    // Example of Forward Rendering Pipeline
    if (graphicsEngine.getRenderingPath() == RenderingPath::Forward)
    {
        for (uint i = 0; i < lightManager.getDirectionalLightCount(); i++)
        {
            //Shadow Pass
            lightManager.BindShadowMap(i);
            m_gameObjectManager->onShadowPass(i);
            lightManager.UnBindShadowMap(i);
        }
        graphicsEngine.setViewport(gameOwner->getWindow()->getInnerSize());

        m_postProcessingFramebuffer->Bind();

        // trying to get debug to show physics
        //graphicsEngine.setFaceCulling(CullType::BackFace);
        //graphicsEngine.setWindingOrder(WindingOrder::CounterClockWise);
        //graphicsEngine.setDepthFunc(DepthType::Less);
        //graphicsEngine.setShader(shader);
        //
        //
        //// Render the physics debug
        //rp3d::DebugRenderer& debugRenderer = m_PhysicsWorld->getDebugRenderer();
        //auto debugLines = debugRenderer.getLines();
        //
        //// Retrieve the instance of the graphics engine
        //graphicsEngine.setVertexArrayObject(debugLines);
        //
        //// Draw the mesh to update the shadow map
        //graphicsEngine.drawLines(debugLines->getNumIndices());


        m_gameObjectManager->onGraphicsUpdate(uniformData);
        m_skyBox->onGraphicsUpdate(uniformData);
        m_gameObjectManager->Render(uniformData);

        m_postProcessingFramebuffer->UnBind();


        // Post processing 
        graphicsEngine.clear(glm::vec4(0, 0, 0, 1)); //clear the scene
        //m_postProcessingFramebuffer->PopulateShader();
        m_postProcessSSRQ->onGraphicsUpdate(uniformData);
    }

    // ui canvas of some sort with the ui camera
}

void Scene::onResize(int _width, int _height)
{
    for (auto camera : m_gameObjectManager->getCameras())
    {
        camera->setScreenArea(Vector2(_width, _height));
    }
    // resize the post processing frame buffer 
    m_postProcessingFramebuffer->Resize(Vector2(_width, _height));
}

void Scene::onQuit()
{
    //m_entitySystem->saveEntitiesToFile("Scenes/Scene1.json");
    m_gameObjectManager->clearGameObjects();

    // Destroy the physics world when the scene is deleted
    m_PhysicsCommon.destroyPhysicsWorld(m_PhysicsWorld);
}

rp3d::PhysicsWorld* Scene::GetPhysicsWorld()
{
    return m_PhysicsWorld;
}

rp3d::PhysicsCommon& Scene::GetPhysicsCommon()
{
    return m_PhysicsCommon;
}
