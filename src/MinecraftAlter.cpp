#include "MinecraftAlter.h"
#include "Shader.h"
#include "Cube.h"
#include "World.h"

#include <map>

#define WATER_SURFACE_CUBE_HEIGHT 0.875
#define SEA_SURFACE_ALTITUDE 23

MinecraftAlter::Shader shader{};
MinecraftAlter::Shader shNormal{};
MinecraftAlter::Shader shShadowMap{};
MinecraftAlter::Shader shDebugDepth{};

MinecraftAlter::World world{};
MinecraftAlter::Player* player_ptr = nullptr;

MinecraftAlter::Cube unitCube{true};
std::map<int, glm::vec4> cubeIdToColor;

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

    shader.compile("src/shaders/vsh.glsl", "src/shaders/fsh.glsl");
    shNormal.compile("src/shaders/normal_v.glsl", "src/shaders/normal_f.glsl");
    shShadowMap.compile("src/shaders/shadowMap_v.glsl", "src/shaders/shadowMap_f.glsl");
    shDebugDepth.compile("src/shaders/debug_depth_v.glsl", "src/shaders/debug_depth_f.glsl");

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

    setupColorMap();
    setupTextures();
    setupDepthMap();

    // Light Position Transform
    auto lightPosMat = glm::mat4(1.0f);
    lightPosMat = glm::translate(lightPosMat, world.worldCenter); // Move to world center
    lightPosMat = glm::scale(lightPosMat, glm::vec3((float)world.worldDimMax));
    lightPosition = lightPosMat * glm::vec4(lightPosition, 1);

    MinecraftAlter::World::randomizeSeed();
    world.AltitudeSeaSurface = SEA_SURFACE_ALTITUDE;
    world.generate();

    MinecraftAlter::Player player{};
    player_ptr = &player;
    player.world_ptr = &world;
    if (!player.generatePlayerSpawn()) { return -1; }

    while (!glfwWindowShouldClose(window)) {
        displayFPS(window, &player);

        // Clear screen
        // Update Sky color based on the rotation of lightPosition
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawVertices(player);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}

unsigned int quadVAO = 0;
unsigned int quadVBO;

void renderQuad() {
    if (quadVAO == 0) {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// Render
void drawVertices(MinecraftAlter::Player& player) {
    MinecraftAlter::Quad::vertRenderCount = 0;

    // Light Position Transform
    auto lightPosMtx = glm::mat4(1.0f);
    if (lightPosInputRotZ) lightPosRotZ += (float)lightPosInputRotZ;
    lightPosMtx = glm::rotate(lightPosMtx, glm::radians(lightPosRotZ), glm::vec3(0.0f, 0.0f, 1.0f));

    // Render to depth map from light's POV
    const float near_plane = 100.0f, far_plane = 500.0f;
    glm::mat4 lightProjection = glm::ortho(
        -(float)world.worldDimMax,
        (float)world.worldDimMax,
        -(float)world.worldDimMax,
        (float)world.worldDimMax,
        near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(glm::vec3(lightPosMtx * glm::vec4(lightPosition, 1.0f)), world.worldCenter,
                                      glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    shShadowMap.use();
    shShadowMap.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    drawWorldCubes(false);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // glViewport(0, 0, windowWidth, windowHeight);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // shDebugDepth.use();
    // shDebugDepth.setFloat("near_plane", near_plane);
    // shDebugDepth.setFloat("far_plane", far_plane);
    // shDebugDepth.setInt("depthMap", 0);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, depthMap);
    // renderQuad();

    // return;
    // View(Camera) Transform
    glViewport(0, 0, windowWidth, windowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.use();
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
                                            500.0f);

    // Link Variables to Shader Uniforms
    MinecraftAlter::Shader::activeShader->setMat4("view", CamView);
    MinecraftAlter::Shader::activeShader->setMat4("projection", projection);
    MinecraftAlter::Shader::activeShader->setVec3("lightPos", lightPosMtx * glm::vec4(lightPosition, 1.0f));
    MinecraftAlter::Shader::activeShader->setVec3("lightColor", lightColor);
    MinecraftAlter::Shader::activeShader->setFloat("ambient", ambient);
    MinecraftAlter::Shader::activeShader->setFloat("time", (float)glfwGetTime());

    MinecraftAlter::Shader::activeShader->setInt("texBlocks", 0);
    MinecraftAlter::Shader::activeShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader.setInt("shadowMap", 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texBlocks);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    drawWorldCubes();

    // Render Sun
    glm::mat4 SunPos = glm::mat4(1.0f);
    SunPos = glm::translate(SunPos, glm::vec3(lightPosMtx * glm::vec4(lightPosition, 1.0f)));
    SunPos = glm::scale(SunPos, glm::vec3{(float)world.worldDimMax / 4});
    MinecraftAlter::Shader::activeShader->setMat4("model", SunPos);
    MinecraftAlter::Shader::activeShader->setVec4("diffuseColor", glm::vec4{1.0});
    MinecraftAlter::Shader::activeShader->setFloat("ambient", 1);
    MinecraftAlter::Shader::activeShader->setBool("useTexture", false);
    unitCube.XPos.renderQuad();
    unitCube.XNeg.renderQuad();
    unitCube.YPos.renderQuad();
    unitCube.YNeg.renderQuad();
    unitCube.ZPos.renderQuad();
    unitCube.ZNeg.renderQuad();
}

void drawWorldCubes(bool drawWater) {
    // Draw opaque blocks
    // Remove specular lighting and wave
    MinecraftAlter::Shader::activeShader->setFloat("specularStrength", 0);
    MinecraftAlter::Shader::activeShader->setFloat("waveStrength", 0);
    MinecraftAlter::Shader::activeShader->setBool("useTexture", true);
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
                glm::mat4 CubePos = glm::mat4(1.0f);
                CubePos = glm::translate(CubePos, glm::vec3{x, y, z});

                MinecraftAlter::Shader::activeShader->setMat4("model", CubePos);
                MinecraftAlter::Shader::activeShader->setVec4("diffuseColor", cubeIdToColor.at(currId));
                MinecraftAlter::Shader::activeShader->setInt("blockId", currId);
                if (x == world.worldDimX - 1 || !isBlockOpaque(world.world[x + 1][z][y])) unitCube.XPos.renderQuad();
                if (z == world.worldDimZ - 1 || !isBlockOpaque(world.world[x][z + 1][y])) unitCube.ZPos.renderQuad();
                if (y == world.worldDimY - 1 || !isBlockOpaque(world.world[x][z][y + 1])) unitCube.YPos.renderQuad();
                if (x == 0 || !isBlockOpaque(world.world[x - 1][z][y])) unitCube.XNeg.renderQuad();
                if (z == 0 || !isBlockOpaque(world.world[x][z - 1][y])) unitCube.ZNeg.renderQuad();
                if (y == 0 || !isBlockOpaque(world.world[x][z][y - 1])) unitCube.YNeg.renderQuad();
            }
        }
    }
    if (!drawWater) return;
    // Draw water
    // Add specular lighting and wave
    MinecraftAlter::Shader::activeShader->setFloat("specularStrength", 2);
    MinecraftAlter::Shader::activeShader->setFloat("waveStrength", 1);
    MinecraftAlter::Shader::activeShader->setBool("useTexture", false);
    for (int x = 0; x < world.worldDimMax; ++x) {
        for (int z = 0; z < world.worldDimMax; ++z) {
            for (int y = 0; y < world.worldDimMax; ++y) {
                // skip not water
                if (world.world[x][z][y] != 10) continue;
                glm::mat4 CubePos = glm::mat4(1.0f);
                CubePos = glm::translate(CubePos, glm::vec3{x, y, z});
                // If it is water surface, shrink height to pre-set value
                if (y == world.worldDimY - 1 || world.world[x][z][y + 1] != 10)
                    CubePos = glm::scale(CubePos, glm::vec3{1,WATER_SURFACE_CUBE_HEIGHT, 1});

                MinecraftAlter::Shader::activeShader->setMat4("model", CubePos);
                MinecraftAlter::Shader::activeShader->setVec4("diffuseColor", cubeIdToColor.at(world.world[x][z][y]));
                // if water face faces to water, not render
                if (x == world.worldDimX - 1 || world.world[x + 1][z][y] != 10) unitCube.XPos.renderQuad();
                if (z == world.worldDimZ - 1 || world.world[x][z + 1][y] != 10) unitCube.ZPos.renderQuad();
                if (y == world.worldDimY - 1 || world.world[x][z][y + 1] != 10) unitCube.YPos.renderQuad();
                if (x == 0 || world.world[x - 1][z][y] != 10) unitCube.XNeg.renderQuad();
                if (z == 0 || world.world[x][z - 1][y] != 10) unitCube.ZNeg.renderQuad();
                if (y == 0 || world.world[x][z][y - 1] != 10) unitCube.YNeg.renderQuad();
            }
        }
    }
}

void setupColorMap() {
    cubeIdToColor.insert(std::pair<int, glm::vec4>(0, glm::vec4{0})); // 0: air
    cubeIdToColor.insert(std::pair<int, glm::vec4>(1, glm::vec4{.1, .1, .1, 1})); // 1: bedrock
    cubeIdToColor.insert(std::pair<int, glm::vec4>(2, glm::vec4{.5, .5, .5, 1})); // 2: stone
    cubeIdToColor.insert(std::pair<int, glm::vec4>(3, glm::vec4{.6, .4, .1, 1})); // 3: dirt
    cubeIdToColor.insert(std::pair<int, glm::vec4>(4, glm::vec4{.5, .9, .1, 1})); // 4: grass
    cubeIdToColor.insert(std::pair<int, glm::vec4>(5, glm::vec4{.8, .8, .5, 1})); // 5: sand
    cubeIdToColor.insert(std::pair<int, glm::vec4>(6, glm::vec4{.85, .9, .95, 1})); // 6: snow
    cubeIdToColor.insert(std::pair<int, glm::vec4>(10, glm::vec4{.2, .4, .9, .8})); // 10: water
}

bool isBlockOpaque(int id) {
    return id != 0 && id != 10;
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


// Inputs

void setInputs(GLFWwindow* window) {
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetWindowSizeCallback(window, windowSizeCallback);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (isFirstPersonView) {
        if (key == GLFW_KEY_W && action == GLFW_PRESS) playerMoveForward = +1;
        if (key == GLFW_KEY_W && action == GLFW_RELEASE) playerMoveForward = 0;
        if (key == GLFW_KEY_S && action == GLFW_PRESS) playerMoveForward = -1;
        if (key == GLFW_KEY_S && action == GLFW_RELEASE) playerMoveForward = 0;
        if (key == GLFW_KEY_D && action == GLFW_PRESS) playerMoveRight = +1;
        if (key == GLFW_KEY_D && action == GLFW_RELEASE) playerMoveRight = 0;
        if (key == GLFW_KEY_A && action == GLFW_PRESS) playerMoveRight = -1;
        if (key == GLFW_KEY_A && action == GLFW_RELEASE) playerMoveRight = 0;
        if (key == GLFW_KEY_F10 && action == GLFW_PRESS) player_ptr->isFloating = !player_ptr->isFloating;
        if (player_ptr->isFloating) {
            if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) playerMoveUp = +1;
            if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) playerMoveUp = 0;
            if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) playerMoveUp = -1;
            if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) playerMoveUp = 0;
        } else {
            if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) player_ptr->jump();
        }
        if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) player_ptr->isSprinting = true;
        if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) player_ptr->isSprinting = false;
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) lightPosInputRotZ = +1;
    if (key == GLFW_KEY_Q && action == GLFW_RELEASE) lightPosInputRotZ = 0;
    if (key == GLFW_KEY_E && action == GLFW_PRESS) lightPosInputRotZ = -1;
    if (key == GLFW_KEY_E && action == GLFW_RELEASE) lightPosInputRotZ = 0;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        shader.use();
    }
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        shader.use();
    }
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        shNormal.use();
    }
    if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
        isFirstPersonView = !isFirstPersonView;
        if (isFirstPersonView) {
            CursorControlCam = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            glfwSetCursorPos(window, (double)windowWidth / 2, (double)windowHeight / 2);
        } else {
            CursorControlCam = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) toggleFullScreen(window);
    if (key == GLFW_KEY_F12 && action == GLFW_PRESS) {
        world.generate();
        if (player_ptr) player_ptr->generatePlayerSpawn();
    }
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (!isFirstPersonView) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            CursorControlCam = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            glfwSetCursorPos(window, (double)windowWidth / 2, (double)windowHeight / 2);
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            CursorControlCam = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    if (yOffset > 0 && CamValDistance > 1 || yOffset < 0 && CamValDistance < 3)
        CamValDistance -= (CamValDistance < 2 ? .05f : 0.1f) * (float)yOffset;
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
    if (!CursorControlCam) return;
    const float halfWidth = (float)windowWidth / 2;
    const float halfHeight = (float)windowHeight / 2;
    CursorDeltaX = (float)xPos - halfWidth;
    CursorDeltaY = (float)yPos - halfHeight;
    glfwSetCursorPos(window, (double)halfWidth, (double)halfHeight);
}

void windowSizeCallback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    int x, y;
    glfwGetFramebufferSize(window, &x, &y);
    glViewport(0, 0, x, y);
}

void toggleFullScreen(GLFWwindow* window) {
    isFullScreen = !isFullScreen;
    if (isFullScreen) {
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        glfwGetWindowPos(window, &windowPosX, &windowPosY);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, 0);
    } else {
        glfwSetWindowMonitor(window, nullptr, windowPosX, windowPosY, windowWidth, windowHeight, 0);
    }
}

// FPS displaying
void displayFPS(GLFWwindow* window, const MinecraftAlter::Player* player_ptr_local) {
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
            + std::to_string(MinecraftAlter::Quad::vertRenderCount) + " Vertices | "
            + "Player @ "
            + dToDecimalStr((double)player_ptr_local->location.x) + " "
            + dToDecimalStr((double)player_ptr_local->location.y) + " "
            + dToDecimalStr((double)player_ptr_local->location.z);
        glfwSetWindowTitle(window, newTitle.c_str());

        // Reset times and counter
        timePrev = timeCurr;
        FrameCounter = 0;
    }
}

std::string dToDecimalStr(double d) {
    const std::string tempStr = std::to_string(d);
    return tempStr.substr(0, tempStr.find('.') + 2);
}
