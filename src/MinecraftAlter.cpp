#include "MinecraftAlter.h"
#include <windows.h>
#include "Shader.h"
#include "Cube.h"
#include "perlin.h"

#define WATER_SURFACE_CUBE_HEIGHT 0.875
#define SEA_SURFACE_ALTITUDE 23

static MinecraftAlter::Shader shader{};
static MinecraftAlter::Shader shNormal{};

int worldDim = 72;
std::array<std::array<std::array<int, 72>, 72>, 72> world{};

MinecraftAlter::Cube unitCube = MinecraftAlter::Cube();
std::map<int, glm::vec4> cubeIdToColor;

GLFWwindow* window;

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
    window = glfwCreateWindow(
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
    shader.use();
    shNormal.compile("src/shaders/normal_v.glsl", "src/shaders/normal_f.glsl");

    // Set Inputs
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // GL Settings
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST); // Enable Depth Testing (Z-buffer)
    glEnable(GL_CULL_FACE); // Enable Face culling
    glCullFace(GL_FRONT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::cout << glGetString(GL_VERSION) << std::endl;

    setupColorMap();

    // Light Position Transform
    auto lightPosMat = glm::mat4(1.0f);
    lightPosMat = glm::translate(lightPosMat, glm::vec3(-.5 + worldDim / 2,
                                                        -.5 + worldDim / 4,
                                                        -.5 + worldDim / 2)); // Move to world center
    lightPosMat = glm::scale(lightPosMat, glm::vec3(worldDim));
    lightPosition = lightPosMat * glm::vec4(lightPosition, 1);

    srand(time(nullptr));
    generateWorldInfo();

    while (!glfwWindowShouldClose(window)) {
        displayFPS();

        // Clear screen
        // Update Sky color based on the rotation of lightPosition
        const float lightPosRotZ_0_180 = remainder(lightPosRotZ + 180.0f, 360);
        const float timeOfDay = lightPosRotZ_0_180 / 360.0f * 24;
        for (int i = 0; i < sizeof(skyColorMap) / sizeof(float) / 4 - 1; ++i) {
            const float lastPivot = skyColorMap[i * 4];
            const float nextPivot = skyColorMap[(i + 1) * 4];
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

        drawVertices();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}

// Generate
void generateWorldInfo() {
    int perlinSeed = rand();
    for (int x = 0; x < world.size(); ++x) {
        for (int z = 0; z < world[0].size(); ++z) {
            float surfaceHeight =
                (MinecraftAlter::perlin(perlinSeed, (float)x / 32, (float)z / 32) * .5 + .5) * 32
                + (MinecraftAlter::perlin(perlinSeed, (float)x / 16, (float)z / 16) * .5) * 12
                + 10;
            if (surfaceHeight > SEA_SURFACE_ALTITUDE + 5) {
                surfaceHeight = SEA_SURFACE_ALTITUDE + 5 + (surfaceHeight - SEA_SURFACE_ALTITUDE - 5) * 2;
            }
            for (int y = 0; y < world[0][0].size(); ++y) {
                world[x][z][y] = y == 0
                                     ? 1 // Bedrock @ y = 0
                                     : y < surfaceHeight - 4
                                     ? 2 // Stone
                                     : y < surfaceHeight - 1
                                     ? 3 // Dirt
                                     : y < surfaceHeight
                                     // For the solid surface: below sea+2 -> Sand; below sea+12 -> Grass; above -> Snow
                                     ? surfaceHeight > SEA_SURFACE_ALTITUDE + 2
                                           ? surfaceHeight > SEA_SURFACE_ALTITUDE + 12
                                                 ? 6
                                                 : 4
                                           : 5
                                     // Above the solid surface: below sea -> Water; above -> Air
                                     : y > SEA_SURFACE_ALTITUDE
                                     ? 0
                                     : 10;
            }
        }
    }
}

// Render
void drawVertices() {
    MinecraftAlter::Quad::vertRenderCount = 0;

    // Model Transform
    auto model = glm::mat4(1.0f);

    // View(Camera) Transform
    CamView = glm::mat4(1.0f);
    CamView = glm::translate(CamView, glm::vec3(0.0f, 0.0f, -worldDim * CamValDistance)); // Cam z distance
    if (CamInputPitch > 0 && CamValPitch < +90) CamValPitch += (float)CamInputPitch * 2;
    if (CamInputPitch < 0 && CamValPitch > -90) CamValPitch += (float)CamInputPitch * 2;
    if (CamInputYaw) CamValYaw += (float)CamInputYaw * 2;
    CamView = glm::rotate(CamView, glm::radians(CamValPitch), glm::vec3(1.0f, 0.0f, 0.0f));
    CamView = glm::rotate(CamView, glm::radians(CamValYaw), glm::vec3(0.0f, 1.0f, 0.0f));
    CamView = glm::translate(CamView, glm::vec3(.5 - worldDim / 2, .5 - worldDim / 4, .5 - worldDim / 2));

    // Perspective Transform
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f,
                                            500.0f);

    // Light Position Transform
    auto lightPosMtx = glm::mat4(1.0f);
    if (lightPosInputRotZ) lightPosRotZ += (float)lightPosInputRotZ;
    lightPosMtx = glm::rotate(lightPosMtx, glm::radians(lightPosRotZ), glm::vec3(0.0f, 0.0f, 1.0f));

    // Link Variables to Shader Uniforms
    shader.setMat4("model", model);
    shader.setMat4("view", CamView);
    shader.setMat4("projection", projection);
    shader.setVec3("lightPos", lightPosMtx * glm::vec4(lightPosition, 1.0f));
    shader.setVec3("lightColor", lightColor);
    shader.setFloat("ambient", ambient);
    shader.setFloat("time", (float)glfwGetTime());
    shNormal.setMat4("model", model);
    shNormal.setMat4("view", CamView);
    shNormal.setMat4("projection", projection);

    drawWorldCubes();

    // Render Sun
    glm::mat4 SunPos = glm::mat4(1.0f);
    SunPos = glm::translate(SunPos, glm::vec3(lightPosMtx * glm::vec4(lightPosition, 1.0f)));
    SunPos = glm::scale(SunPos, glm::vec3{(float)worldDim / 4});
    shader.setMat4("model", SunPos);
    shader.setVec4("diffuseColor", glm::vec4{1.0});
    shader.setFloat("ambient", 1);
    unitCube.XPos.renderQuad();
    unitCube.XNeg.renderQuad();
    unitCube.YPos.renderQuad();
    unitCube.YNeg.renderQuad();
    unitCube.ZPos.renderQuad();
    unitCube.ZNeg.renderQuad();
}

void drawWorldCubes() {
    // Draw opaque blocks
    // Remove specular lighting and wave
    shader.setFloat("specularStrength", 0);
    shader.setFloat("waveStrength", 0);
    for (int x = 0; x < worldDim; ++x) {
        for (int z = 0; z < worldDim; ++z) {
            for (int y = 0; y < worldDim; ++y) {
                // skip air block
                if (!world[x][z][y]) continue;
                // skip water
                if (world[x][z][y] == 10) continue;
                // skip block that is fully covered from the 6 directions
                if (x != 0 && x != world.size() - 1 &&
                    y != 0 && y != world.size() - 1 &&
                    z != 0 && z != world.size() - 1 &&
                    isBlockOpaque(world[x + 1][z][y]) && isBlockOpaque(world[x - 1][z][y]) &&
                    isBlockOpaque(world[x][z + 1][y]) && isBlockOpaque(world[x][z - 1][y]) &&
                    isBlockOpaque(world[x][z][y + 1]) && isBlockOpaque(world[x][z][y - 1]))
                    continue;
                glm::mat4 CubePos = glm::mat4(1.0f);
                CubePos = glm::translate(CubePos, glm::vec3{x, y, z});

                shader.setMat4("model", CubePos);
                shader.setVec4("diffuseColor", cubeIdToColor.at(world[x][z][y]));
                shNormal.setMat4("model", CubePos);
                if (x == world.size() - 1 || !isBlockOpaque(world[x + 1][z][y])) unitCube.XPos.renderQuad();
                if (z == world.size() - 1 || !isBlockOpaque(world[x][z + 1][y])) unitCube.ZPos.renderQuad();
                if (y == world.size() - 1 || !isBlockOpaque(world[x][z][y + 1])) unitCube.YPos.renderQuad();
                if (x == 0 || !isBlockOpaque(world[x - 1][z][y])) unitCube.XNeg.renderQuad();
                if (z == 0 || !isBlockOpaque(world[x][z - 1][y])) unitCube.ZNeg.renderQuad();
                if (y == 0 || !isBlockOpaque(world[x][z][y - 1])) unitCube.YNeg.renderQuad();
            }
        }
    }
    // Draw water
    // Add specular lighting and wave
    shader.setFloat("specularStrength", 2);
    shader.setFloat("waveStrength", 1);
    for (int x = 0; x < worldDim; ++x) {
        for (int z = 0; z < worldDim; ++z) {
            for (int y = 0; y < worldDim; ++y) {
                // skip not water
                if (world[x][z][y] != 10) continue;
                glm::mat4 CubePos = glm::mat4(1.0f);
                CubePos = glm::translate(CubePos, glm::vec3{x, y, z});
                // If it is water surface, shrink height to pre-set value
                if (y != world.size() - 1 && world[x][z][y + 1] != 10) {
                    CubePos = glm::translate(CubePos, glm::vec3{0, -.5 + WATER_SURFACE_CUBE_HEIGHT * .5, 0});
                    CubePos = glm::scale(CubePos, glm::vec3{1,WATER_SURFACE_CUBE_HEIGHT, 1});
                }

                shader.setMat4("model", CubePos);
                shader.setVec4("diffuseColor", cubeIdToColor.at(world[x][z][y]));
                shNormal.setMat4("model", CubePos);
                // if water face faces to water, not render
                if (x == world.size() - 1 || world[x + 1][z][y] != 10) unitCube.XPos.renderQuad();
                if (z == world.size() - 1 || world[x][z + 1][y] != 10) unitCube.ZPos.renderQuad();
                if (y == world.size() - 1 || world[x][z][y + 1] != 10) unitCube.YPos.renderQuad();
                if (x == 0 || world[x - 1][z][y] != 10) unitCube.XNeg.renderQuad();
                if (z == 0 || world[x][z - 1][y] != 10) unitCube.ZNeg.renderQuad();
                if (y == 0 || world[x][z][y - 1] != 10) unitCube.YNeg.renderQuad();
            }
        }
    }
}

void setupColorMap() {
    cubeIdToColor.insert(std::pair<int, glm::vec4>(0, glm::vec4{0})); // 0: air
    cubeIdToColor.insert(std::pair<int, glm::vec4>(1, glm::vec4{.1, .1, .1, 1})); // 1: bedrock
    cubeIdToColor.insert(std::pair<int, glm::vec4>(2, glm::vec4{.5, .5, .5, 1})); // 2: stone
    cubeIdToColor.insert(std::pair<int, glm::vec4>(3, glm::vec4{.6, .4, .1, 1})); // 3: dirt
    cubeIdToColor.insert(std::pair<int, glm::vec4>(4, glm::vec4{.2, .6, .1, 1})); // 4: grass
    cubeIdToColor.insert(std::pair<int, glm::vec4>(5, glm::vec4{.8, .8, .5, 1})); // 5: sand
    cubeIdToColor.insert(std::pair<int, glm::vec4>(6, glm::vec4{.85, .9, .95, 1})); // 6: snow
    cubeIdToColor.insert(std::pair<int, glm::vec4>(10, glm::vec4{.2, .4, .9, .8})); // 10: water
}

bool isBlockOpaque(int id) {
    return id != 0 && id != 10;
}


// Inputs
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

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_A && action == GLFW_PRESS) CamInputYaw = +1;
    if (key == GLFW_KEY_A && action == GLFW_RELEASE) CamInputYaw = 0;
    if (key == GLFW_KEY_D && action == GLFW_PRESS) CamInputYaw = -1;
    if (key == GLFW_KEY_D && action == GLFW_RELEASE) CamInputYaw = 0;
    if (key == GLFW_KEY_W && action == GLFW_PRESS) CamInputPitch = +1;
    if (key == GLFW_KEY_W && action == GLFW_RELEASE) CamInputPitch = 0;
    if (key == GLFW_KEY_S && action == GLFW_PRESS) CamInputPitch = -1;
    if (key == GLFW_KEY_S && action == GLFW_RELEASE) CamInputPitch = 0;
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) lightPosInputRotZ = +1;
    if (key == GLFW_KEY_Q && action == GLFW_RELEASE) lightPosInputRotZ = 0;
    if (key == GLFW_KEY_E && action == GLFW_PRESS) lightPosInputRotZ = -1;
    if (key == GLFW_KEY_E && action == GLFW_RELEASE) lightPosInputRotZ = 0;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) toggleFullScreen(window);
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
    if (key == GLFW_KEY_F12 && action == GLFW_PRESS) {
        generateWorldInfo();
    }
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    // if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) printf("MouseL\n");
}

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    if (yOffset > 0 && CamValDistance > 1 || yOffset < 0 && CamValDistance < 3)
        CamValDistance -= (CamValDistance < 2 ? .05f : 0.1f) * (float)yOffset;
}

// FPS displaying
void displayFPS() {
    timeCrnt = glfwGetTime();
    timeDiff = timeCrnt - timePrev;
    FrameCounter++;
    if (timeDiff >= 1.0 / 30.0) {
        // Update title of window
        std::string FPS = std::to_string(1.0 / timeDiff * FrameCounter);
        std::string ms = std::to_string(timeDiff / FrameCounter * 1000);
        std::string newTitle =
            windowTitle + "   "
            + FPS.substr(0, FPS.find('.') + 2) + " FPS | "
            + ms.substr(0, FPS.find('.') + 2) + " MS | "
            + std::to_string(MinecraftAlter::Quad::vertRenderCount) + " Vertices";
        glfwSetWindowTitle(window, newTitle.c_str());

        // Reset times and counter
        timePrev = timeCrnt;
        FrameCounter = 0;
    }
}
