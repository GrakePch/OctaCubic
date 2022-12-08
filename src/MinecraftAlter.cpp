#include "MinecraftAlter.h"
#include <windows.h>
#include "Shader.h"
#include "Cube.h"
#include "perlin.h"

static MinecraftAlter::Shader shader;

int worldDim = 64;
std::array<std::array<std::array<MinecraftAlter::Cube, 64>, 64>, 64> world{};

MinecraftAlter::Cube unitCube = MinecraftAlter::Cube();
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
        windowTitle,
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

    shader.compile();
    shader.bind();

    // Set Inputs
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);

    // GL Settings
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST); // Enable Depth Testing (Z-buffer)
    glEnable(GL_CULL_FACE); // Enable Face culling
    glCullFace(GL_FRONT);

    std::cout << glGetString(GL_VERSION) << std::endl;

    setupRender();
    srand(time(nullptr));
    generateWorldInfo();

    while (!glfwWindowShouldClose(window)) {
        // Clear screen
        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawVertices();

        // std::cout << glfwGetTime() << std::endl;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}

// Generate
void generateWorldInfo() {
    // Light Position Transform
    auto lightPosMat = glm::mat4(1.0f);
    lightPosMat = glm::scale(lightPosMat, glm::vec3(worldDim));
    lightPosition = lightPosMat * glm::vec4(lightPosition, 1);

    // Noise generate Terrain: assign position and cube Id
    for (int x = 0; x < world.size(); ++x) {
        for (int z = 0; z < world[0].size(); ++z) {
            int surfaceHeight = (MinecraftAlter::perlin((float)x * 4 / worldDim, (float)z * 4 / worldDim) * .5 + .5) *
                32 + 10;
            for (int y = 0; y < world[0][0].size(); ++y) {
                world[x][z][y].position.x = x;
                world[x][z][y].position.y = y;
                world[x][z][y].position.z = z;
                world[x][z][y].cubeId = y == 0
                                            ? 1 // Bedrock
                                            : y < surfaceHeight - 4
                                            ? 2 // Stone
                                            : y < surfaceHeight - 1
                                            ? 3 // Dirt
                                            : y < surfaceHeight
                                            ? surfaceHeight > 24 // Grass or Sand (if surface height <= 24)
                                                  ? 4
                                                  : 5
                                            : y > 23 // Air or Water (y <= 23)
                                            ? 0
                                            : 10;
            }
        }
    }

    // Generate vertices: ignore faces that is not exposed to air
    int showFaces[6] = {1,1,1,1,1,1};
    for (int x = 0; x < world.size(); ++x) {
        for (int z = 0; z < world[0].size(); ++z) {
            for (int y = 0; y < world[0][0].size(); ++y) {
                // Check -Y
                // showFaces[0] = y == 0 || !world[x][z][y-1].cubeId;
                // Check +Y
                // Check -Z
                // Check +Z
                // Check -X
                // Check +X
                world[x][z][y].updateVerticesFacesShown(showFaces);
            }
        }
    }
}

// Render
void setupRender() {
    // Bind Vertex Array Object
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // Copy the vertices array in a vertex buffer for OpenGL to use
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Set the vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    // Copy the index array in a element buffer for OpenGL to use
    // glGenBuffers(1, &ebo);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    // for (int i = 0; i < 36; ++i) {
    //     std::cout <<
    //         testCube.cubeVertices[i].position[0] << " " <<
    //         testCube.cubeVertices[i].position[1] << " " <<
    //         testCube.cubeVertices[i].position[2] << " " << std::endl;
    // }

    cubeIdToColor.insert(std::pair<int, glm::vec4>(0, glm::vec4{0})); // 0: air
    cubeIdToColor.insert(std::pair<int, glm::vec4>(1, glm::vec4{.1, .1, .1, 1})); // 1: bedrock
    cubeIdToColor.insert(std::pair<int, glm::vec4>(2, glm::vec4{.5, .5, .5, 1})); // 2: stone
    cubeIdToColor.insert(std::pair<int, glm::vec4>(3, glm::vec4{.6, .4, .1, 1})); // 3: dirt
    cubeIdToColor.insert(std::pair<int, glm::vec4>(4, glm::vec4{.2, .6, .1, 1})); // 4: grass
    cubeIdToColor.insert(std::pair<int, glm::vec4>(5, glm::vec4{.8, .8, .5, 1})); // 5: sand
    cubeIdToColor.insert(std::pair<int, glm::vec4>(10, glm::vec4{.2, .4, .9, .6})); // 10: water
}

void drawVertices() {
    // Model Transform
    auto model = glm::mat4(1.0f);

    // View(Camera) Transform
    CamView = glm::mat4(1.0f);
    CamView = glm::translate(CamView, glm::vec3(0.0f, 0.0f, -worldDim * 2)); // Cam z distance
    if (CamInputPitch > 0 && CamValPitch < +90) CamValPitch += (float)CamInputPitch;
    if (CamInputPitch < 0 && CamValPitch > -90) CamValPitch += (float)CamInputPitch;
    if (CamInputYaw) CamValYaw += (float)CamInputYaw;
    CamView = glm::rotate(CamView, glm::radians(CamValPitch), glm::vec3(1.0f, 0.0f, 0.0f));
    CamView = glm::rotate(CamView, glm::radians(CamValYaw), glm::vec3(0.0f, 1.0f, 0.0f));
    CamView = glm::translate(CamView, glm::vec3(.5 - worldDim / 2, .5 - worldDim / 4, .5 - worldDim / 2));

    // Perspective Transform
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f,
                                            500.0f);

    // Light Position Transform
    auto lightPosMtx = glm::mat4(1.0f);
    if (lightPosInputYaw) lightPosValYaw += (float)lightPosInputYaw;
    lightPosMtx = glm::rotate(lightPosMtx, glm::radians(lightPosValYaw), glm::vec3(0.0f, 0.0f, 1.0f));

    // Link Variables to Shader Uniforms
    const int modelLoc = glGetUniformLocation(shader.programId, "model");
    const int viewLoc = glGetUniformLocation(shader.programId, "view");
    const int projectionLoc = glGetUniformLocation(shader.programId, "projection");
    const int lightPosLoc = glGetUniformLocation(shader.programId, "lightPos");
    const int lightColorLoc = glGetUniformLocation(shader.programId, "lightColor");
    const int ambientLoc = glGetUniformLocation(shader.programId, "ambient");
    const int diffuseClrLoc = glGetUniformLocation(shader.programId, "diffuseColor");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(CamView));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPosMtx * glm::vec4(lightPosition, 1.0f)));
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    glUniform1f(ambientLoc, ambient);


    glBindVertexArray(vao);
    // glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    drawWorldCubes(modelLoc, diffuseClrLoc);

    glBindVertexArray(0);
}

void deleteBuffers() {
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}

void drawWorldCubes(const int modelShaderLoc, const int diffuseColorShaderLoc) {
    for (int x = 0; x < worldDim; ++x) {
        for (int z = 0; z < worldDim; ++z) {
            for (int y = 0; y < worldDim; ++y) {
                if (!world[x][z][y].cubeId) continue; // skip air block
                if (!world[x][z][y].verticesFilled) continue; // skip block that is fully covered from the 6 directions
                glBufferData(GL_ARRAY_BUFFER, world[x][z][y].verticesFilled * sizeof(Vertex), world[x][z][y].vertices.data(),
                             GL_STATIC_DRAW);
                glm::mat4 CubePos = glm::mat4(1.0f);
                CubePos = glm::translate(CubePos, glm::vec3{x, y, z});
                glUniformMatrix4fv(modelShaderLoc, 1, GL_FALSE, glm::value_ptr(CubePos));
                glUniform4fv(diffuseColorShaderLoc, 1, glm::value_ptr(cubeIdToColor.at(world[x][z][y].cubeId)));
                glDrawArrays(GL_TRIANGLES, 0, world[x][z][y].verticesFilled);
            }
        }
    }
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
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) lightPosInputYaw = +1;
    if (key == GLFW_KEY_Q && action == GLFW_RELEASE) lightPosInputYaw = 0;
    if (key == GLFW_KEY_E && action == GLFW_PRESS) lightPosInputYaw = -1;
    if (key == GLFW_KEY_E && action == GLFW_RELEASE) lightPosInputYaw = 0;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) toggleFullScreen(window);
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) printf("MouseL\n");
}
