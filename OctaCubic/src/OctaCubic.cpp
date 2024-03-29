#include "OctaCubic.h"
#include "Shader.h"
#include "Cube.h"
#include "World.h"
#include "debugQuad.h"
#include "utils.h"
#include "utils_render.h"
#include "inputs.h"

#define WATER_SURFACE_CUBE_HEIGHT 0.875
#define SEA_SURFACE_ALTITUDE 23
#define NUM_OF_VERT_S_ATTR 12


OctaCubic::Shader shader{};
OctaCubic::Shader shWater{};
OctaCubic::Shader shNormal{};
OctaCubic::Shader shShadowMap{};
OctaCubic::Shader shDebugDepth{};
OctaCubic::Shader shCoord{};
OctaCubic::Shader shDebugFrameBuffer{};
OctaCubic::Shader shHighlightBlock{};

OctaCubic::World world{};
OctaCubic::Player* player_ptr = nullptr;

OctaCubic::Cube unitCube{true};


int main() {
    if (!glfwInit()) {
        printf("Failed to init GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    windowWidth = 1280;
    windowHeight = 720;
    windowPosX = 0;
    windowPosY = 0;
    GLFWwindow* window = glfwCreateWindow(
        windowWidth,
        windowHeight,
        windowTitle.c_str(),
        nullptr, // Monitor; glfwGetPrimaryMonitor(): full screen
        nullptr // Resource sharing
    );

    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return printf("Failed to initialize GLAD\n");
    }
    // Make sure not to call any OpenGL functions until *after* we initialize our function loader

    // Compile shaders
    shader.compile("src/shaders/vsh.glsl", "src/shaders/fsh.glsl");
    shWater.compile("src/shaders/water_v.glsl", "src/shaders/water_f.glsl");
    shNormal.compile("src/shaders/normal_v.glsl", "src/shaders/normal_f.glsl");
    shShadowMap.compile("src/shaders/shadowMap_v.glsl", "src/shaders/shadowMap_f.glsl");
    shDebugDepth.compile("src/shaders/debug_depth_v.glsl", "src/shaders/debug_depth_f.glsl");
    shCoord.compile("src/shaders/coord_v.glsl", "src/shaders/coord_f.glsl");
    shDebugFrameBuffer.compile("src/shaders/frameBuffer_v.glsl", "src/shaders/frameBuffer_f.glsl");
    shHighlightBlock.compile("src/shaders/highlightBlock_v.glsl", "src/shaders/highlightBlock_f.glsl");
    
    // Set Inputs
    setInputs(window);

    // GL Settings
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST); // Enable Depth Testing (Z-buffer)
    glEnable(GL_CULL_FACE); // Enable Face culling
    glCullFace(GL_FRONT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::cout << glGetString(GL_VERSION) << std::endl;

    // Light Position Transform
    auto lightPosMat = glm::mat4(1.0f);
    lightPosMat = glm::translate(lightPosMat, world.worldCenter); // Move to world center
    lightPosMat = glm::scale(lightPosMat, glm::vec3((float)world.worldDimMax));
    lightPosition = lightPosMat * glm::vec4(lightPosition, 1);

    OctaCubic::World::randomizeSeed();
    world.AltitudeSeaSurface = SEA_SURFACE_ALTITUDE;
    world.generate();

    OctaCubic::Player player{};
    player_ptr = &player;
    player.world_ptr = &world;
    if (!player.generatePlayerSpawn()) { return -1; }

    genWorldVertices();
    setupDepthMap();
    setupBlockCoordMap();
    setupTextures();
    setupBuffers(terrainVAO, terrainVBO, terrainEBO, verticesBuff, indicesBuff);
    setupBuffers(terrWaterVAO, terrWaterVBO, terrWaterEBO, verticesWaterBuff, indicesWaterBuff);

    while (!glfwWindowShouldClose(window)) {
        displayFPS(window, &player);

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawVertices(player);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}


// Generate world vertices
void genWorldVertices() {
    worldVertCount = 0;
    genTerrainVertices();
    genWaterVertices();
}


void genTerrainVertices() {
    int currQuadIdx = 0;
    for (int x = 0; x < world.worldDimMax; ++x) {
        for (int z = 0; z < world.worldDimMax; ++z) {
            for (int y = 0; y < world.worldDimMax; ++y) {
                int currId = world.world[x][z][y];
                // skip air block
                if (!currId) continue;
                // skip water
                if (currId == 10) continue;
                // skip block that is fully covered from the 6 directions
                if (x != 0 && x != world.worldDimX - 1 &&
                    y != 0 && y != world.worldDimY - 1 &&
                    z != 0 && z != world.worldDimZ - 1 &&
                    isBlockOpaque(world.world[x + 1][z][y]) && isBlockOpaque(world.world[x - 1][z][y]) &&
                    isBlockOpaque(world.world[x][z + 1][y]) && isBlockOpaque(world.world[x][z - 1][y]) &&
                    isBlockOpaque(world.world[x][z][y + 1]) && isBlockOpaque(world.world[x][z][y - 1]))
                    continue;

                if (x == world.worldDimX - 1 || !isBlockOpaque(world.world[x + 1][z][y]))
                    overwriteVertexBuff(NUM_OF_VERT_S_ATTR, verticesBuff, indicesBuff, &currQuadIdx, unitCube.XPos.getVertices(), currId, x,
                                        y, z, worldVertCount);
                if (z == world.worldDimZ - 1 || !isBlockOpaque(world.world[x][z + 1][y]))
                    overwriteVertexBuff(NUM_OF_VERT_S_ATTR, verticesBuff, indicesBuff, &currQuadIdx, unitCube.ZPos.getVertices(), currId, x,
                                        y, z, worldVertCount);
                if (y == world.worldDimY - 1 || !isBlockOpaque(world.world[x][z][y + 1]))
                    overwriteVertexBuff(NUM_OF_VERT_S_ATTR, verticesBuff, indicesBuff, &currQuadIdx, unitCube.YPos.getVertices(), currId, x,
                                        y, z, worldVertCount);
                if (x == 0 || !isBlockOpaque(world.world[x - 1][z][y]))
                    overwriteVertexBuff(NUM_OF_VERT_S_ATTR, verticesBuff, indicesBuff, &currQuadIdx, unitCube.XNeg.getVertices(), currId, x,
                                        y, z, worldVertCount);
                if (z == 0 || !isBlockOpaque(world.world[x][z - 1][y]))
                    overwriteVertexBuff(NUM_OF_VERT_S_ATTR, verticesBuff, indicesBuff, &currQuadIdx, unitCube.ZNeg.getVertices(), currId, x,
                                        y, z, worldVertCount);
                if (y == 0 || !isBlockOpaque(world.world[x][z][y - 1]))
                    overwriteVertexBuff(NUM_OF_VERT_S_ATTR, verticesBuff, indicesBuff, &currQuadIdx, unitCube.YNeg.getVertices(), currId, x,
                                        y, z, worldVertCount);
            }
        }
    }
}

void genWaterVertices() {
    int currQuadIdx = 0;
    for (int x = 0; x < world.worldDimMax; ++x) {
        for (int z = 0; z < world.worldDimMax; ++z) {
            for (int y = 0; y < world.worldDimMax; ++y) {
                int currId = world.world[x][z][y];
                // skip not water
                if (currId != 10) continue;

                // If it is water surface, shrink height to pre-set value
                glm::mat4 CubePos = glm::mat4(1.0f);
                if (y == world.worldDimY - 1 || world.world[x][z][y + 1] != 10)
                    CubePos = glm::scale(CubePos, glm::vec3{1,WATER_SURFACE_CUBE_HEIGHT, 1});

                // if water face faces to water, not render
                if (x == world.worldDimX - 1 || world.world[x + 1][z][y] != 10)
                    overwriteVertexBuff(NUM_OF_VERT_S_ATTR, verticesWaterBuff, indicesWaterBuff, &currQuadIdx,
                                        unitCube.XPos.getVerticesWithTrans(CubePos),
                                        currId, x, y, z, worldVertCount);
                if (z == world.worldDimZ - 1 || world.world[x][z + 1][y] != 10)
                    overwriteVertexBuff(NUM_OF_VERT_S_ATTR, verticesWaterBuff, indicesWaterBuff, &currQuadIdx,
                                        unitCube.ZPos.getVerticesWithTrans(CubePos),
                                        currId, x, y, z, worldVertCount);
                if (y == world.worldDimY - 1 || world.world[x][z][y + 1] != 10)
                    overwriteVertexBuff(NUM_OF_VERT_S_ATTR, verticesWaterBuff, indicesWaterBuff, &currQuadIdx,
                                        unitCube.YPos.getVerticesWithTrans(CubePos),
                                        currId, x, y, z, worldVertCount);
                if (x == 0 || world.world[x - 1][z][y] != 10)
                    overwriteVertexBuff(NUM_OF_VERT_S_ATTR, verticesWaterBuff, indicesWaterBuff, &currQuadIdx,
                                        unitCube.XNeg.getVerticesWithTrans(CubePos),
                                        currId, x, y, z, worldVertCount);
                if (z == 0 || world.world[x][z - 1][y] != 10)
                    overwriteVertexBuff(NUM_OF_VERT_S_ATTR, verticesWaterBuff, indicesWaterBuff, &currQuadIdx,
                                        unitCube.ZNeg.getVerticesWithTrans(CubePos),
                                        currId, x, y, z, worldVertCount);
                if (y == 0 || world.world[x][z][y - 1] != 10)
                    overwriteVertexBuff(NUM_OF_VERT_S_ATTR, verticesWaterBuff, indicesWaterBuff, &currQuadIdx,
                                        unitCube.YNeg.getVerticesWithTrans(CubePos),
                                        currId, x, y, z, worldVertCount);
            }
        }
    }
}


// Render
void drawVertices(OctaCubic::Player& player) {
    // OctaCubic::Quad::vertRenderCount = 0;

    // Light Position Transform
    auto lightPosMtx = glm::mat4(1.0f);
    if (lightPosInputRotZ) lightPosRotZ += (float)lightPosInputRotZ;
    lightPosRotZ = remainder(lightPosRotZ, 360);
    lightPosMtx = glm::rotate(lightPosMtx, glm::radians(lightPosRotZ), glm::vec3(0.0f, 0.0f, 1.0f));

    // Render to depth map from light's POV
    const float near_plane = (float)world.worldDimMax, far_plane = 5 * (float)world.worldDimMax;
    glm::mat4 lightProjection = glm::ortho(
        -(float)world.worldDimMax * .75f,
        (float)world.worldDimMax * .75f,
        -(float)world.worldDimMax * .9f,
        (float)world.worldDimMax * .9f,
        near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(glm::vec3(lightPosMtx * glm::vec4(lightPosition, 1.0f)), world.worldCenter,
                                      glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    shShadowMap.use();
    drawTerrain(terrainVAO, false, nullptr, nullptr, nullptr, &lightSpaceMatrix);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (showLightSpaceDepth) {
        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shDebugDepth.use();
        shDebugDepth.setFloat("near_plane", near_plane);
        shDebugDepth.setFloat("far_plane", far_plane);
        shDebugDepth.setInt("depthMap", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderQuad();
        return;
    }

    /// Render Final Frame ///
    glViewport(0, 0, windowWidth, windowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // View(Camera) Transform
    CamView = glm::mat4(1.0f);
    if (isFirstPersonView) {
        player.updateLocation((float)Second, playerMoveForward, playerMoveRight, playerMoveUp);
        CamView = player.rotateFacing(CamView, CursorDeltaX, CursorDeltaY, CamRotSensitivity);
    } else {
        // Cam z distance
        CamView = glm::translate(CamView, {0.0f, 0.0f, -(float)world.worldDimMax * CamValDistance});
        if (CursorControlCam) {
            CamValPitch = glm::clamp<float>(CamValPitch + CursorDeltaY * CamRotSensitivity, -90, 90);
            CamValYaw += CursorDeltaX * CamRotSensitivity;
            if (CamValYaw >= 180) CamValYaw -= 360;
            if (CamValYaw < -180) CamValYaw += 360;
        }
        CamView = glm::rotate(CamView, glm::radians(CamValPitch), glm::vec3(1.0f, 0.0f, 0.0f));
        CamView = glm::rotate(CamView, glm::radians(CamValYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        CamView = glm::translate(CamView, -world.worldCenter);
    }
    // Clear cursor delta value
    CursorDeltaX = 0;
    CursorDeltaY = 0;

    // Perspective Transform
    glm::mat4 projection = glm::perspective(glm::radians(isFirstPersonView
                                                             ? 90.0f * (float)windowHeight / (float)windowWidth
                                                             : 45.0f),
                                            (float)windowWidth / (float)windowHeight,
                                            0.1f,
                                            5 * (float)world.worldDimMax);


    glBindFramebuffer(GL_FRAMEBUFFER, blockCoordMapFBO);
    glClearColor(1.0,1.0,1.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    shCoord.use();
    shCoord.setFloat("worldMaxDim", (float)world.worldDimMax);
    shCoord.setVec3("playerCoord", player_ptr->location);
    drawTerrain(terrainVAO, false, &CamView, &projection, &lightPosMtx, &lightSpaceMatrix);
    
    unsigned char pixel[4];
    glReadPixels(windowWidth / 2, windowHeight / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    printf("x y z: %f %f %f\n", (float)pixel[0], (float)pixel[1], (float)pixel[2]);
    // player_ptr->AimingAtBlockCoord.x = floor(player_ptr->location.x);
    // player_ptr->AimingAtBlockCoord.y = floor(player_ptr->location.y);
    // player_ptr->AimingAtBlockCoord.z = floor(player_ptr->location.z);
    player_ptr->AimingAtBlockCoord.x = (float)(pixel[0] - 16 + (int)floor(player_ptr->location.x));
    player_ptr->AimingAtBlockCoord.y = (float)(pixel[1] - 16 + (int)floor(player_ptr->location.y));
    player_ptr->AimingAtBlockCoord.z = (float)(pixel[2] - 16 + (int)floor(player_ptr->location.z));
    player_ptr->isAimingAtSomeBlock = player_ptr->AimingAtBlockCoord.x < world.worldDimMax;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    if (showBlockCoord) {
        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT);
        shDebugFrameBuffer.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, blockCoordMap);
        renderQuad();
        return;
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texBlocks);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    
    updateSkyColor();
    
    shader.use();
    drawTerrain(terrainVAO, false, &CamView, &projection, &lightPosMtx, &lightSpaceMatrix);
    shWater.use();
    drawTerrain(terrWaterVAO, true, &CamView, &projection, &lightPosMtx, &lightSpaceMatrix);

    // Render Player Aiming Block
    shHighlightBlock.use();
    glm::mat4 aimingBlockModel = glm::mat4{1.0f};
    aimingBlockModel = translate(aimingBlockModel, player_ptr->AimingAtBlockCoord);
    aimingBlockModel = translate(aimingBlockModel, glm::vec3{-.005f});
    aimingBlockModel = glm::scale(aimingBlockModel, glm::vec3{1.01f});
    shHighlightBlock.setMat4("model", aimingBlockModel);
    shHighlightBlock.setMat4("view", CamView);
    shHighlightBlock.setMat4("projection", projection);
    unitCube.XPos.renderQuad();
    unitCube.XNeg.renderQuad();
    unitCube.YPos.renderQuad();
    unitCube.YNeg.renderQuad();
    unitCube.ZPos.renderQuad();
    unitCube.ZNeg.renderQuad();
    
    return;
    // Render Sun
    glm::mat4 SunPos = glm::mat4(1.0f);
    SunPos = glm::translate(SunPos, glm::vec3(lightPosMtx * glm::vec4(lightPosition, 1.0f)));
    SunPos = glm::scale(SunPos, glm::vec3{(float)world.worldDimMax / 4});
    OctaCubic::Shader::activeShader->setMat4("model", SunPos);
    OctaCubic::Shader::activeShader->setVec4("diffuseColor", glm::vec4{1.0});
    OctaCubic::Shader::activeShader->setFloat("ambient", 1);
    OctaCubic::Shader::activeShader->setBool("useTexture", false);
    unitCube.XPos.renderQuad();
    unitCube.XNeg.renderQuad();
    unitCube.YPos.renderQuad();
    unitCube.YNeg.renderQuad();
    unitCube.ZPos.renderQuad();
    unitCube.ZNeg.renderQuad();
}

void drawTerrain(const unsigned int& vao, bool isWater,
                 const glm::mat4* camView,
                 const glm::mat4* projection,
                 const glm::mat4* lightPosMat,
                 const glm::mat4* lightSpaceMat) {
    OctaCubic::Shader::activeShader->setMat4("model", glm::mat4(1.0f));
    if (camView)
        OctaCubic::Shader::activeShader->setMat4("view", *camView);
    if (projection)
        OctaCubic::Shader::activeShader->setMat4("projection", *projection);
    OctaCubic::Shader::activeShader->setInt("texBlocks", 0);
    OctaCubic::Shader::activeShader->setInt("shadowMap", 1);
    if (lightSpaceMat)
        OctaCubic::Shader::activeShader->setMat4("lightSpaceMatrix", *lightSpaceMat);
    if (lightPosMat)
        OctaCubic::Shader::activeShader->setVec3("lightPos", *lightPosMat * glm::vec4(lightPosition, 1.0f));
    OctaCubic::Shader::activeShader->setVec3("lightColor", lightColor);
    OctaCubic::Shader::activeShader->setFloat("ambient", ambient);
    OctaCubic::Shader::activeShader->setFloat("time", (float)glfwGetTime());
    if (isWater) {
        // Specular lighting and wave
        OctaCubic::Shader::activeShader->setFloat("specularStrength", 2);
        OctaCubic::Shader::activeShader->setFloat("waveStrength", 1);
        OctaCubic::Shader::activeShader->setVec4("diffuseColor", glm::vec4{.2, .4, .9, .8});
    } else {
        // No specular lighting and wave
        OctaCubic::Shader::activeShader->setVec4("diffuseColor", glm::vec4{.5, .9, .1, 1});
    }

    glBindVertexArray(vao);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glDrawElements(GL_TRIANGLES, INDICES_BUFFER_SIZE, GL_UNSIGNED_INT, 0);
}

bool isBlockOpaque(int id) {
    return id != 0 && id != 10;
}

void updateSkyColor() {
    const float lightPosRotZ_0_180 = remainder(lightPosRotZ + 180.0f, 360);
    const float timeOfDay = lightPosRotZ_0_180 / 360.0f * 24;
    for (int i = 0; i < (int)(sizeof(skyColorMap) / sizeof(float) / 4 - 1); ++i) {
        const float lastPivot = skyColorMap[(ptrdiff_t)i * 4];
        const float nextPivot = skyColorMap[(ptrdiff_t)(i + 1) * 4];
        if (timeOfDay > lastPivot && timeOfDay < nextPivot) {
            const float timePortion = (timeOfDay - lastPivot) / (nextPivot - lastPivot);
            glClearColor(
                interpolate(skyColorMap[i * 4 + 1], skyColorMap[(i + 1) * 4 + 1], timePortion),
                interpolate(skyColorMap[i * 4 + 2], skyColorMap[(i + 1) * 4 + 2], timePortion),
                interpolate(skyColorMap[i * 4 + 3], skyColorMap[(i + 1) * 4 + 3], timePortion),
                1.0f);
        }
    }
}


void setupTextures() {
    glGenTextures(1, &texBlocks);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texBlocks);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    texBlocks_data = stbi_load(
        "assets/textures/blocks.png",
        &texBlocksDimX,
        &texBlocksDimY,
        &texBlocksNrChannels,
        STBI_rgb_alpha);
    if (texBlocks_data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texBlocksDimX, texBlocksDimY, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     texBlocks_data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        printf("Failed to load texture");
    }
    stbi_image_free(texBlocks_data); // free the loaded image
    shader.setInt("blockRes", 16);
    shader.setInt("textureRes", texBlocksDimX);
}

void setupBuffers(unsigned int& vao, unsigned int& vbo, unsigned int& ebo,
                  const float* verticesBuffer, const unsigned int* indicesBuffer) {
    // setup plane's Vertex Array Object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // Copy the vertices array in a vertex buffer for OpenGL to use
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, VERTICES_BUFFER_SIZE * sizeof(float), verticesBuffer, GL_STATIC_DRAW);
    // Set the vertex attributes pointers
    /// vec3 vertex's Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, NUM_OF_VERT_S_ATTR * sizeof(float), (void*)0);
    /// vec3 vertex's Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, NUM_OF_VERT_S_ATTR * sizeof(float), (void*)(3 * sizeof(float)));
    /// vec2 vertex's Texture Coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, NUM_OF_VERT_S_ATTR * sizeof(float), (void*)(6 * sizeof(float)));
    /// float vertex's Block Id
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, NUM_OF_VERT_S_ATTR * sizeof(float), (void*)(8 * sizeof(float)));
    /// vec3 vertex's Block Coordinates
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, NUM_OF_VERT_S_ATTR * sizeof(float), (void*)(9 * sizeof(float)));
    // Element Buffer Objects
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDICES_BUFFER_SIZE * sizeof(unsigned int), indicesBuffer, GL_STATIC_DRAW);
}

void setupDepthMap() {
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setupBlockCoordMap() {
    glGenFramebuffers(1, &blockCoordMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, blockCoordMapFBO);
    
    glGenTextures(1, &blockCoordMap);
    glBindTexture(GL_TEXTURE_2D, blockCoordMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE,
    nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blockCoordMap, 0);

    // bind Render buffer to enable depth testing
    glGenRenderbuffers(1, &blockCoordMapRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, blockCoordMapRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, blockCoordMapRBO);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


// FPS displaying
void displayFPS(GLFWwindow* window, const OctaCubic::Player* player_ptr_local) {
    timeCurr = glfwGetTime();
    timeDiff = timeCurr - timePrev;
    FrameCounter++;
    if (timeDiff >= 1.0 / 30.0) {
        // Update FPS & Frame interval
        FPS = 1.0 / timeDiff * FrameCounter;
        Second = timeDiff / FrameCounter;
        // Update title of window
        const std::string newTitle =
            windowTitle + "   "
            + dToDecimalStr(FPS) + " FPS | "
            + dToDecimalStr(Second * 1000) + " MS | "
            + std::to_string(worldVertCount) + " Vertices | "
            + "Player @ "
            + dToDecimalStr((double)player_ptr_local->location.x) + " "
            + dToDecimalStr((double)player_ptr_local->location.y) + " "
            + dToDecimalStr((double)player_ptr_local->location.z) + " | "
            + "Aiming At Block "
            + (player_ptr_local->isAimingAtSomeBlock
                   ? std::to_string((int)floor(player_ptr_local->AimingAtBlockCoord.x)) + " "
                   + std::to_string((int)floor(player_ptr_local->AimingAtBlockCoord.y)) + " "
                   + std::to_string((int)floor(player_ptr_local->AimingAtBlockCoord.z)) + " "
                   : "Null"
            );
        glfwSetWindowTitle(window, newTitle.c_str());

        // Reset times and counter
        timePrev = timeCurr;
        FrameCounter = 0;
    }
}
