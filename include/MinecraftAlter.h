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

static const char* windowTitle = "MinecraftAlter";
static int windowWidth = 1280;
static int windowHeight = 720;
static int windowPosX = 0;
static int windowPosY = 0;

static glm::vec3 lightColor = {1.0f, 1.0f, .95f};
static glm::vec3 skyColor = {.5f, .7f, 1.0f};
static glm::vec3 skyColor_s = {.6f, .1f, .1f};
static glm::vec3 skyColor_n = {.1f, .1f, .2f};
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


static uint32 vao; // Vertex Array Object
static uint32 vbo; // Vertex Buffer Object
static uint32 ebo; // Element Buffer Objects

// generate
void generateWorldInfo();

// render
void setupRender();
void drawVertices();
void deleteBuffers();
void drawWorldCubes();
void setupColorMap();

// inputs
void toggleFullScreen(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// interpolate
inline float interpolate(float f1, float f2, float p) {
    return f1 + p * (f2 - f1);
}
inline float remainder(float dividend, int divisor) {
    int dividendInt = (int)dividend;
    float decimal = dividend - (int)dividend;
    return (float)(dividendInt % divisor) + decimal;
}

#endif
