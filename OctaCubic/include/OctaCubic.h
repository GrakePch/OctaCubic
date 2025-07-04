﻿#ifndef OCTACUBIC_CORE_H
#define OCTACUBIC_CORE_H

#include <string>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Player.h"


static const std::string window_title = "OctaCubic";
static int windowWidth = 1280;
static int windowHeight = 720;
static int windowPosX = 0;
static int windowPosY = 0;

// Game Logic Variables
static double timePrev = 0.0;
static double timeCurr = 0.0;
static double timeDiff;
static double second = 0.0;
static unsigned int frameCounter = 0;

static glm::vec3 lightColor = {1.0f, 1.0f, .95f};
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

// Generate world vertices
size_t worldVertCount = 0;

// render
void drawVertices(OctaCubic::Player& player);
void setShaderUniforms(bool isWater,
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
static bool mouseButtonLeftPressedPrev = false; // true if last frame is pressed.
static bool mouseButtonLeftPressDown = false;
static bool mouseButtonRightPressed = false;
static bool mouseButtonRightPressedPrev = false; // true if last frame is pressed.
static bool mouseButtonRightPressDown = false;

// Debugging
void updateDebuggingGUI(const OctaCubic::Player* player_ptr_local);

#endif
