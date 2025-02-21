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

Scene::Scene(Game* game)
{
    gameOwner = game;
    m_postProcessingFramebuffer = std::make_unique<Framebuffer>(gameOwner->getWindow()->getInnerSize());
    m_gameObjectManager = std::make_unique<GameObjectManager>(this);
    m_postProcessSSRQ = std::make_unique<SSRQuad>();
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

    m_postProcessSSRQ->onCreate();
    m_postProcessSSRQ->setShader(defaultFullscreenShader);
    m_postProcessSSRQ->setTexture(m_postProcessingFramebuffer->RenderTexture);
    //Creating skybox object
    m_skyBox = std::make_unique<SkyboxEntity>();
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

    m_skyBox->setEntitySystem(m_gameObjectManager.get());
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
    InstancedMeshPtr statueMesh = resourceManager.createInstancedMeshFromFile("Resources/Meshes/SM_Prop_Statue_01.obj");

    ShaderPtr meshShader = graphicsEngine.createShader({
            "MeshShader",
            "MeshShader"
        });
    ShaderPtr terrainShader = graphicsEngine.createShader({
            "TerrainShader",
            "TerrainShader"
        });

    ShaderPtr instancedMeshShader = graphicsEngine.createShader({
            "InstancedMesh",
            "InstancedMesh"
        });


    m_ship = m_gameObjectManager->createGameObject<MeshEntity>();
    m_ship->m_transform.scale = Vector3(0.05f);
    m_ship->m_transform.position = Vector3(0, 40, 0);
    m_ship->setShininess(64.0f);
    m_ship->setTexture(sciFiSpaceTexture2D);
    m_ship->setReflectiveMapTexture(shipReflectiveMap);
    m_ship->setMesh(fighterShip);
    m_ship->setShader(meshShader);
    m_ship->setShadowShader(m_shadowShader);
    m_ship->setGeometryShader(m_meshGeometryShader);
    //m_ship->setLightingShader(m_meshLightingShader);

    HeightMapInfo buildInfo = { "Resources/Heightmaps/Heightmap0.raw", 256, 256, 4.0f };
    HeightMapPtr heightmap = resourceManager.createHeightMap(buildInfo);

    m_terrain = m_gameObjectManager->createGameObject<TerrainEntity>();
    m_terrain->generateTerrainMesh(heightmap);
    m_terrain->m_transform.position = Vector3(0, -20, 0);
    m_terrain->setTexture(grassTexture);
    m_terrain->setTexture1(dirtTexture);
    m_terrain->setTexture2(stoneTexture);
    m_terrain->setTexture3(snowTexture);
    m_terrain->setHeightMap(heightMapTexture);
    m_terrain->setShader(terrainShader);
    m_terrain->setShadowShader(m_shadowShader);
    m_terrain->setGeometryShader(m_terrainGeometryShader);

    //Creating instanced tree obj
    auto statues = m_gameObjectManager->createGameObject<InstancedMeshEntity>();
    statues->setShininess(32.0f);
    statues->setTexture(ancientWorldsTexture2D);
    statues->setShader(instancedMeshShader);
    statues->setMesh(statueMesh);
    statues->setReflectiveMapTexture(shipReflectiveMap); //this is wrong
    statues->setShadowShader(m_shadowInstancedShader);
    statues->setGeometryShader(m_instancedmeshGeometryShader);
    //statues->setLightingShader(m_meshLightingShader);


    //adds instances to the instanced mine mesh
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
    for (int i = 0; i < 2; i++) {
        // Create a new point light entity
        auto pointLightObject = m_gameObjectManager->createGameObject<MeshEntity>();
        pointLightObject->setTransparency(0.75f);

        // Randomly set color to either red or blue
        int randomColorChoice = (int)randomNumber(2.0f); // Generates 0 or 1
        Vector3 lightColor = (randomColorChoice == 0) ? Color::Red * 2.0f : Color::Blue * 2.0f;
        pointLightObject->setColor(lightColor);

        // Calculate the position based on row and column, center the grid around (0,0)
        float xPosition = i * pointLightSpacing; // Center horizontally
        float yPosition = 15.0f; // Fixed Y position
        float zPosition = 0;
        pointLightObject->m_transform.position = Vector3(xPosition, yPosition, zPosition);
        pointLightObject->m_transform.scale = Vector3(3.0f);

        // Set mesh and shaders
        pointLightObject->setMesh(gameOwner->getSphereMesh());
        pointLightObject->setShader(m_solidColorMeshShader);
        pointLightObject->setShadowShader(m_shadowShader);
        pointLightObject->setGeometryShader(m_meshGeometryShader);

        // Configure point light properties
        PointLight pointLight;
        pointLight.Position = pointLightObject->m_transform.position;
        pointLight.Color = pointLightObject->getColor();
        pointLight.SpecularStrength = 1.0f;
        pointLight.AttenuationConstant = 1.0f;
        pointLight.AttenuationLinear = 0.022f;
        pointLight.AttenuationExponent = 0.0019f;

        // Add the point light to the light manager
        lightManager.createPointLight(pointLight);
    }

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
    m_gameObjectManager->onFixedUpdate(fixedDeltaTime);
}

void Scene::onLateUpdate(float deltaTime)
{
    m_gameObjectManager->onLateUpdate(deltaTime);
}

void Scene::onShadowPass()
{
    
}

void Scene::onGeometryPass()
{
    auto& lightManager = LightManager::GetInstance();

    m_gameObjectManager->onGeometryPass(uniformData);
}

void Scene::onLightingPass()
{
    auto& lightManager = LightManager::GetInstance();
    gameOwner->getScreenSpaceQuad()->onLightingPass(uniformData);
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
        onGeometryPass(); // Render opaque objects
        geometryBuffer.UnBind();

        // Shadow Pass: Render shadows for directional lights
        for (uint i = 0; i < lightManager.getDirectionalLightCount(); i++)
        {
            lightManager.BindShadowMap(i);
            m_gameObjectManager->onShadowPass(i); // Render shadow maps
            lightManager.UnBindShadowMap(i);
        }
        graphicsEngine.setViewport(gameOwner->getWindow()->getInnerSize());

        geometryBuffer.WriteDepth();
        // Lighting Pass: Apply lighting using G-buffer data
        onLightingPass(); // Compute lighting
        
        
        //m_gameObjectManager->onForwardPass():
        for (auto& light : m_lights)
        {
            light->onGraphicsUpdate(uniformData);
        }
        m_skyBox->onGraphicsUpdate(uniformData);
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

        m_gameObjectManager->onGraphicsUpdate(uniformData);
        m_skyBox->onGraphicsUpdate(uniformData);

        m_postProcessingFramebuffer->UnBind();
    }



    // Post processing 
    graphicsEngine.clear(glm::vec4(0, 0, 0, 1)); //clear the scene
    m_postProcessSSRQ->setTexture(m_postProcessingFramebuffer->RenderTexture);
    m_postProcessSSRQ->onGraphicsUpdate(uniformData);

    //NewUniformData uniformData;
    //uniformData.CreateData<float>("Time", m_currentTime);
    //uniformData.CreateData<Vector2>("Resolution", m_display->getInnerSize());
    //if (currentTexture1)
    //{
    //    //if the current shader needs a second texture we pass that into it
    //    NewExtraTextureData textureData;
    //    textureData.AddTexture("Texture1", currentTexture1, 1);
    //    m_canvasQuad->onGraphicsUpdate(uniformData, textureData);
    //}
    //else
    //{
    //}
}

void Scene::onResize(int _width, int _height)
{
    for (auto camera : m_gameObjectManager->getCameras())
    {
        camera->setScreenArea(Vector2(_width, _height));
    }
}

void Scene::onQuit()
{
    //m_entitySystem->saveEntitiesToFile("Scenes/Scene1.json");
    m_gameObjectManager->clearGameObjects();
}
