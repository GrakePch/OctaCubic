#ifndef MINECRAFTALTER_CORE_H
#define MINECRAFTALTER_CORE_H

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>
#include <glad/glad.h> // Must before GLFW
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>

#include "Player.h"


struct Vertex {
    std::array<float, 3> position;
    std::array<float, 3> normal;
};


static const std::string windowTitle = "MinecraftAlter";
static int windowWidth = 1280;
static int windowHeight = 720;
static int windowPosX = 0;
static int windowPosY = 0;

static glm::vec3 lightColor = {1.0f, 1.0f, .95f};
static float skyColorMap[] = {
    // time r   g     b
    0.0f, 0.0f, 0.0f, 0.05f, // mid-night
    5.0f, 0.0f, 0.0f, 0.05f,
    6.0f, 0.9f, 0.5f, 0.2f, // sunrise
    7.0f, 0.5f, 0.7f, 1.0f,
    17.0f, 0.5f, 0.7f, 1.0f,
    18.0f, 0.9f, 0.5f, 0.2f, // sunset
    19.0f, 0.0f, 0.0f, 0.05f,
    24.0f, 0.0f, 0.0f, 0.05f, // mid-night
};
static float ambient = 0.15f;

// View(Camera) Transform
static glm::mat4 CamView;

// light Position Transform
static glm::vec3 lightPosition = {0.0f, 3.0f, 1.0f};

// Transform Inputs
static bool isFullScreen = false;
static int lightPosInputRotZ = 0;
static float CamValYaw = 45.0f;
static float CamValPitch = 30.0f;
static float CamValDistance = 2.0f;
static float lightPosRotZ = 0.0f;

// Terrain vertices buffers
#define VERTICES_BUFFER_SIZE 191102976
// 96^3 blocks * 6 faces * 4 vertices * 9 attributes
#define INDICES_BUFFER_SIZE 31850496
// 96^3 blocks * 6 faces * 6 indices
unsigned int terrainVAO = 0;
unsigned int terrainVBO;
unsigned int terrainEBO;
float verticesBuff[VERTICES_BUFFER_SIZE] = {0};
unsigned int indicesBuff[INDICES_BUFFER_SIZE] = {0};

unsigned int terrWaterVAO = 0;
unsigned int terrWaterVBO;
unsigned int terrWaterEBO;
float verticesWaterBuff[VERTICES_BUFFER_SIZE] = {0};
unsigned int indicesWaterBuff[INDICES_BUFFER_SIZE] = {0};
void setupBuffers(unsigned int& vao, unsigned int& vbo, unsigned int& ebo,
                  const float* verticesBuffer, const unsigned int* indicesBuffer);

// Generate terrain vertices
void genWorldVertices();
void genWaterVertices();
void overwriteVertexBuff(float* verticesBuffer, unsigned int* indicesBuffer,
                         int* currQuadIdx, const float* arr, int blockId, int x, int y, int z);
void clearVerticesBuffer(float* verticesBuffer, unsigned int* indicesBuffer);

// render
void drawVertices(MinecraftAlter::Player& player);
void drawTerrain(const unsigned int& vao, bool isWater,
                 const glm::mat4* camView,
                 const glm::mat4* projection,
                 const glm::mat4* lightPosMat,
                 const glm::mat4* lightSpaceMat);
bool isBlockOpaque(int id);

// render textures
int texBlocksDimX, texBlocksDimY, texBlocksNrChannels;
unsigned char* texBlocks_data;
unsigned int texBlocks;
void setupTextures();

// depth map
unsigned int depthMapFBO;
const unsigned int SHADOW_WIDTH = 16384, SHADOW_HEIGHT = 16384;
unsigned int depthMap;
void setupDepthMap();


// inputs
static bool isFirstPersonView = false;
static int playerMoveForward = 0;
static int playerMoveRight = 0;
static int playerMoveUp = 0;
static bool CursorControlCam = false;
static float CursorDeltaX = 0;
static float CursorDeltaY = 0;
static float CamRotSensitivity = .2f;
void setInputs(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void cursorPosCallback(GLFWwindow* window, double xPos, double yPos);
void windowSizeCallback(GLFWwindow* window, int width, int height);
void toggleFullScreen(GLFWwindow* window);

// FPS displaying
static double timePrev = 0.0;
static double timeCurr = 0.0;
static double timeDiff;
static double FPS = 0.0;
static double Second = 0.0;
static unsigned int FrameCounter = 0;
void displayFPS(GLFWwindow* window, const MinecraftAlter::Player* player_ptr_local);
std::string dToDecimalStr(double d);

// interpolate
inline float interpolate(float f1, float f2, float p) {
    return f1 + p * (f2 - f1);
}

inline float remainder(float dividend, int divisor) {
    if (dividend < 0) dividend += static_cast<float>(divisor);
    const int dividendInt = static_cast<int>(dividend);
    const float decimal = dividend - round(dividend);
    return static_cast<float>(dividendInt % divisor) + decimal;
}

#endif
