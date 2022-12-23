#ifndef MINECRAFTALTER_CORE_H
#define MINECRAFTALTER_CORE_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdint.h>
#include <array>
#include <map>
#include <vector>
#include "Quad.h"


struct Vertex {
    std::array<float, 3> position;
    std::array<float, 3> normal;
};

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

static const std::string windowTitle = "MinecraftAlter";
static int windowWidth = 1280;
static int windowHeight = 720;
static int windowPosX = 0;
static int windowPosY = 0;

static glm::vec3 lightColor = {1.0f, 1.0f, .95f};
static glm::vec3 skyColor = {.5f, .7f, 1.0f};
static glm::vec3 skyColor_s = {.6f, .1f, .1f};
static glm::vec3 skyColor_n = {.1f, .1f, .2f};
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
static float ambient = 0.1f;

// View(Camera) Transform
static glm::mat4 CamView;

// light Position Transform
static glm::vec3 lightPosition = {0.0f, 3.0f, 1.0f};

// Transform Inputs
static bool isFullScreen = false;
static int CamInputYaw = 0;
static int CamInputPitch = 0;
static int lightPosInputRotZ = 0;
static float CamValYaw = 45.0f;
static float CamValPitch = 30.0f;
static float CamValDistance = 2.0f;
static float lightPosRotZ = 0.0f;


// generate
void generateWorldInfo();

// render
void drawVertices();
void drawWorldCubes();
void setupColorMap();
bool isBlockOpaque(int id);

// inputs
void toggleFullScreen(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// FPS displaying
static double timePrev = 0.0;
static double timeCrnt = 0.0;
static double timeDiff;
static unsigned int FrameCounter = 0;
void displayFPS();

// interpolate
inline float interpolate(float f1, float f2, float p) {
    return f1 + p * (f2 - f1);
}

inline float remainder(float dividend, int divisor) {
    if (dividend < 0) dividend += divisor;
    int dividendInt = (int)dividend;
    float decimal = dividend - (int)dividend;
    return (float)(dividendInt % divisor) + decimal;
}

#endif
