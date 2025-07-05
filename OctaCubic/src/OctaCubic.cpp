#include "OctaCubic.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h> // Must before GLFW
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Cube.h"
#include "World.h"
#include "debugQuad.h"
#include "utils.h"
#include "inputs.h"

#define WATER_SURFACE_CUBE_HEIGHT 0.875
#define SEA_SURFACE_ALTITUDE 23


OctaCubic::Shader shader{};
OctaCubic::Shader shWater{};
OctaCubic::Shader shNormal{};
OctaCubic::Shader shShadowMap{};
OctaCubic::Shader shDebugDepth{};
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
        window_title.c_str(),
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
        printf("Failed to initialize GLAD\n");
        glfwTerminate();
        return -1;
    }
    // Make sure not to call any OpenGL functions until *after* we initialize our function loader

    // Compile shaders
    shader.compile("src/shaders/vsh.glsl", "src/shaders/fsh.glsl");
    shWater.compile("src/shaders/water_v.glsl", "src/shaders/water_f.glsl");
    shNormal.compile("src/shaders/normal_v.glsl", "src/shaders/normal_f.glsl");
    shShadowMap.compile("src/shaders/shadowMap_v.glsl", "src/shaders/shadowMap_f.glsl");
    shDebugDepth.compile("src/shaders/debug_depth_v.glsl", "src/shaders/debug_depth_f.glsl");
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
    glfwSwapInterval(0); // Disable VSync

    printf("%s\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    // Initialize World
    OctaCubic::World::randomizeSeed();
    world.altitudeSeaSurface = SEA_SURFACE_ALTITUDE;

    // Initialize Player
    OctaCubic::Player player{};
    player_ptr = &player;
    player.world_ptr = &world;
    if (!player.generatePlayerSpawn()) { return -1; }

    
    // Initialize Light Position Transform
    auto lightPosMat = glm::mat4(1.0f);
    lightPosMat = glm::translate(lightPosMat, player_ptr->location); // Move to player location
    lightPosMat = glm::scale(lightPosMat, glm::vec3((float)world.worldDimMax));
    lightPosition = lightPosMat * glm::vec4(lightPosition, 1);

    setupDepthMap();
    setupTextures();

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true); // install_callback=true: install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    while (!glfwWindowShouldClose(window)) {

        // Update inputs
        cursorDeltaX = cursorDeltaY = 0;
        glfwPollEvents();
        mouseButtonLeftPressDown = mouseButtonLeftPressed && !mouseButtonLeftPressedPrev;
        mouseButtonLeftPressedPrev = mouseButtonLeftPressed;
        mouseButtonRightPressDown = mouseButtonRightPressed && !mouseButtonRightPressedPrev;
        mouseButtonRightPressedPrev = mouseButtonRightPressed;

        /* Start handling game logics */
        // Update time related variables
        timeCurr = glfwGetTime();
        timeDiff = timeCurr - timePrev;
        frameCounter++;
        if (timeDiff >= 1.0 / 30.0) {
            second = timeDiff / frameCounter;
        }
        // Observer mode camera controls
        if (!isFirstPersonView && cursorControlCam) {
            camValPitch = glm::clamp<float>(camValPitch + cursorDeltaY * camRotSensitivity, -90, 90);
            camValYaw += cursorDeltaX * camRotSensitivity;
            if (camValYaw >= 180) camValYaw -= 360;
            if (camValYaw < -180) camValYaw += 360;
        }
        // Update player location and rotation
        player.updateLocation(static_cast<float>(second), playerMoveForward, playerMoveRight, playerMoveUp);
        player.updateRotation(cursorDeltaX, cursorDeltaY, camRotSensitivity);
        // Player Aiming
        const glm::vec3 posPlayerEye = player.location + glm::vec3{0, player.eyeHeight, 0};
        const OctaCubic::CoordinatesAndFace aimBlockInfo = world.lineTraceToFace(posPlayerEye, player.directionLooking, 10.0f);
        player.isAimingAtSomeBlock = aimBlockInfo.isHit;
        player.aimingAtBlockCoord = glm::vec3{
            static_cast<float>(aimBlockInfo.x),
            static_cast<float>(aimBlockInfo.y),
            static_cast<float>(aimBlockInfo.z)
        };
        player.aimingAtBlockFace = aimBlockInfo.f;
        // Destroy & place block
        if (isFirstPersonView) {
            const int aX = static_cast<int>(player_ptr->aimingAtBlockCoord.x);
            const int aY = static_cast<int>(player_ptr->aimingAtBlockCoord.y);
            const int aZ = static_cast<int>(player_ptr->aimingAtBlockCoord.z);
            bool blockChanged = false;
            if (mouseButtonLeftPressDown) {
                blockChanged = world.setBlockId(aX, aY, aZ, 0) >= 0;
            }
            if (mouseButtonRightPressDown) {
                int pX = aX, pY = aY, pZ = aZ;
                switch (player_ptr->aimingAtBlockFace) {
                    case OctaCubic::xPos: pX += 1; break;
                    case OctaCubic::xNeg: pX -= 1; break;
                    case OctaCubic::yPos: pY += 1; break;
                    case OctaCubic::yNeg: pY -= 1; break;
                    case OctaCubic::zPos: pZ += 1; break;
                    case OctaCubic::zNeg: pZ -= 1; break;
                }
                blockChanged = world.setBlockId(pX, pY, pZ, 2) >= 0;
            }
        }
        /* End handling game logics */

        // Debugging GUI
        updateDebuggingGUI(&player);

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Main Draw
        drawVertices(player);

        // Render Debugging GUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}


// Render
void drawVertices(OctaCubic::Player& player) {
    world.smartRenderingPreprocess(player_ptr->location, 10);
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
    glm::mat4 lightView = glm::lookAt(glm::vec3(lightPosMtx * glm::vec4(lightPosition, 1.0f)), player_ptr->location,
                                      glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    glViewport(0, 0, shadow_width, shadow_height);
    glBindFramebuffer(GL_FRAMEBUFFER, fboDepthMap);
    glClear(GL_DEPTH_BUFFER_BIT);
    shShadowMap.use();
    setShaderUniforms(false, nullptr, nullptr, nullptr, &lightSpaceMatrix);
    world.renderInQueueOpaque();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (showLightSpaceDepth) {
        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shDebugDepth.use();
        shDebugDepth.setFloat("near_plane", near_plane);
        shDebugDepth.setFloat("far_plane", far_plane);
        shDebugDepth.setInt("depthMap", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texDepthMap);
        renderQuad();
        return;
    }

    /// Render Final Frame ///
    glViewport(0, 0, windowWidth, windowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // View(Camera) Transform
    camView = glm::mat4(1.0f);
    if (isFirstPersonView) {
        camView = player.getCameraViewMat4();
    }
    else {
        // Cam z distance
        camView = glm::translate(camView, {0.0f, 0.0f, -(float)world.worldDimMax * camValDistance});
        camView = glm::rotate(camView, glm::radians(camValPitch), glm::vec3(1.0f, 0.0f, 0.0f));
        camView = glm::rotate(camView, glm::radians(camValYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        camView = glm::translate(camView, -player_ptr->location);
    }

    // Perspective Transform
    glm::mat4 projection = glm::perspective(glm::radians(isFirstPersonView
                                                             ? 90.0f * (float)windowHeight / (float)windowWidth
                                                             : 45.0f),
                                            (float)windowWidth / (float)windowHeight,
                                            0.1f,
                                            5 * (float)world.worldDimMax);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texBlocks);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texDepthMap);

    updateSkyColor();

    shader.use();
    setShaderUniforms(false, &camView, &projection, &lightPosMtx, &lightSpaceMatrix);
    world.renderInQueueOpaque();
    shWater.use();
    setShaderUniforms(true, &camView, &projection, &lightPosMtx, &lightSpaceMatrix);
    world.renderInQueueWater();

    // Render Player Aiming Block
    shHighlightBlock.use();
    glm::mat4 aimingBlockModel = glm::mat4{1.0f};
    aimingBlockModel = translate(aimingBlockModel, player_ptr->aimingAtBlockCoord);
    aimingBlockModel = translate(aimingBlockModel, glm::vec3{-.005f});
    aimingBlockModel = glm::scale(aimingBlockModel, glm::vec3{1.01f});
    shHighlightBlock.setMat4("model", aimingBlockModel);
    shHighlightBlock.setMat4("view", camView);
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

void setShaderUniforms(bool isWater,
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
    }
    else {
        // No specular lighting and wave
        OctaCubic::Shader::activeShader->setVec4("diffuseColor", glm::vec4{.5, .9, .1, 1});
    }
}

void updateSkyColor() {
    static float skyColorMap[] = {
        // time,  r,    g,     b,
        00.0f, 0.0f, 0.0f, 0.05f, // midnight
        05.0f, 0.0f, 0.0f, 0.05f,
        06.0f, 0.9f, 0.5f, 0.20f, // sunrise
        07.0f, 0.5f, 0.7f, 1.00f,
        17.0f, 0.5f, 0.7f, 1.00f,
        18.0f, 0.9f, 0.5f, 0.20f, // sunset
        19.0f, 0.0f, 0.0f, 0.05f,
        24.0f, 0.0f, 0.0f, 0.05f, // midnight
    };
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
    }
    else {
        printf("Failed to load texture");
    }
    stbi_image_free(texBlocks_data); // free the loaded image
    shader.setInt("blockRes", 16);
    shader.setInt("textureRes", texBlocksDimX);
}

void setupDepthMap() {
    glGenFramebuffers(1, &fboDepthMap);
    glGenTextures(1, &texDepthMap);
    glBindTexture(GL_TEXTURE_2D, texDepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_width, shadow_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindFramebuffer(GL_FRAMEBUFFER, fboDepthMap);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texDepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


// Debugging
void updateDebuggingGUI(const OctaCubic::Player* player_ptr_local) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    
    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::GetStyle().FontScaleDpi = 2.0f;
    ImGui::GetStyle().WindowBorderSize = 0.0f;

    ImGui::Begin("Debug", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoInputs);

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("MS: %.1f", ImGui::GetIO().Framerate > 0 ? 1000.0f / ImGui::GetIO().Framerate : 0.0f);
    ImGui::Text("%llu Vertices", worldVertCount);
    ImGui::Text("%llu Chunks in GPU", OctaCubic::Chunk::getNumOfChunksInGPU());
    ImGui::Text("Player: %.1f %.1f %.1f",
                player_ptr_local->location.x,
                player_ptr_local->location.y,
                player_ptr_local->location.z);
    ImGui::Text("Yaw: %.1f Pitch: %.1f",
                player_ptr_local->yaw, player_ptr_local->pitch);
    ImGui::Text("Looking: %.1f %.1f %.1f",
                player_ptr_local->directionLooking.x,
                player_ptr_local->directionLooking.y,
                player_ptr_local->directionLooking.z);
    if (player_ptr_local->isAimingAtSomeBlock) {
        std::string blockFaceName;
        switch (player_ptr_local->aimingAtBlockFace) {
            case OctaCubic::xPos: blockFaceName = "X+"; break;
            case OctaCubic::xNeg: blockFaceName = "X-"; break;
            case OctaCubic::yPos: blockFaceName = "Y+"; break;
            case OctaCubic::yNeg: blockFaceName = "Y-"; break;
            case OctaCubic::zPos: blockFaceName = "Z+"; break;
            case OctaCubic::zNeg: blockFaceName = "Z-"; break;
        }
        ImGui::Text("Aiming: %d %d %d / %s", 
                    static_cast<int>(floor(player_ptr_local->aimingAtBlockCoord.x)),
                    static_cast<int>(floor(player_ptr_local->aimingAtBlockCoord.y)),
                    static_cast<int>(floor(player_ptr_local->aimingAtBlockCoord.z)),
                    blockFaceName.c_str());
    }
    else
        ImGui::Text("Aiming: Nothing");

    { // Draw crosshair
        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* drawList = ImGui::GetForegroundDrawList();

        ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        float size = 16.0f;
        ImU32 color = IM_COL32(255, 255, 255, 255); // white
        float thickness = 2.0f;

        // Horizontal line
        drawList->AddLine(ImVec2(center.x - size, center.y), ImVec2(center.x + size, center.y), color, thickness);

        // Vertical line
        drawList->AddLine(ImVec2(center.x, center.y - size), ImVec2(center.x, center.y + size), color, thickness);
    }

    ImGui::End();
}
