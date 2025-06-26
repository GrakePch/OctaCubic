#ifndef OCTACUBIC_CORE_H
#define OCTACUBIC_CORE_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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


static const std::string window_title = "OctaCubic";
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
static glm::mat4 camView;

// light Position Transform
static glm::vec3 lightPosition = {0.0f, 3.0f, 1.0f};

// Transform Inputs
static bool isFullScreen = false;
static int lightPosInputRotZ = 0;
static float camValYaw = 45.0f;
static float camValPitch = 30.0f;
static float camValDistance = 2.0f;
static float lightPosRotZ = 0.0f;

// Terrain vertices buffers
#define VERTICES_BUFFER_SIZE 9437184
// 32^3 blocks * 6 faces * 4 vertices * 12 attributes
#define INDICES_BUFFER_SIZE 1179648
// 32^3 blocks * 6 faces * 6 indices
unsigned int vaoTerrainOpaque = 0;
unsigned int vboTerrainOpaque;
unsigned int eboTerrainOpaque;
float buffVtxTerrainOpaque[VERTICES_BUFFER_SIZE] = {0};
unsigned int buffIdxTerrainOpaque[INDICES_BUFFER_SIZE] = {0};

unsigned int vaoTerrainWater = 0;
unsigned int vboTerrainWater;
unsigned int eboTerrainWater;
float buffVtxTerrainWater[VERTICES_BUFFER_SIZE] = {0};
unsigned int buffIdxTerrainWater[INDICES_BUFFER_SIZE] = {0};
void setupBuffers(unsigned int& vao, unsigned int& vbo, unsigned int& ebo);
void bufferData(const unsigned int& vao, const unsigned int& vbo, const unsigned int& ebo,
                  const float* verticesBuffer, const unsigned int* indicesBuffer);

// Generate world vertices
unsigned int worldVertCount = 0;
void genBuffVtxWorld();
void genBuffVtxTerrainOpaque();
void genBuffVtxTerrainWater();

// render
void drawVertices(OctaCubic::Player& player);
void drawTerrain(const unsigned int& vao, bool isWater,
                 const glm::mat4* camView,
                 const glm::mat4* projection,
                 const glm::mat4* lightPosMat,
                 const glm::mat4* lightSpaceMat);
// Update Sky color based on the rotation of lightPosition
void updateSkyColor();

// render textures
int texBlocksDimX, texBlocksDimY, texBlocksNrChannels;
unsigned char* texBlocks_data;
unsigned int texBlocks;
void setupTextures();

// depth map
unsigned int fboDepthMap;
constexpr unsigned int shadow_width = 16384, shadow_height = 16384;
unsigned int texDepthMap;
void setupDepthMap();
bool showLightSpaceDepth = false;

// block coord map
unsigned int fboBlockCoordMap;
unsigned int texBlockCoordMap;
unsigned int rboBlockCoordMap;
void setupBlockCoordMap();
bool showBlockCoord = false;

// inputs
static bool isFirstPersonView = false;
static int playerMoveForward = 0;
static int playerMoveRight = 0;
static int playerMoveUp = 0;
static bool cursorControlCam = false;
static float cursorDeltaX = 0;
static float cursorDeltaY = 0;
static float camRotSensitivity = .2f;
static bool mouseButtonLeftPressed = false;
static bool mouseButtonLeftPressedPrev = false;  // true if last frame is pressed.
static bool mouseButtonLeftPressDown = false;
static bool mouseButtonRightPressed = false;
static bool mouseButtonRightPressedPrev = false; // true if last frame is pressed.
static bool mouseButtonRightPressDown = false;

// FPS displaying
static double timePrev = 0.0;
static double timeCurr = 0.0;
static double timeDiff;
static double fps = 0.0;
static double second = 0.0;
static unsigned int frameCounter = 0;
void displayFps(GLFWwindow* window, const OctaCubic::Player* player_ptr_local);

#endif
