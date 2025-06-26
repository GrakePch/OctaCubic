#pragma once

#include <GLFW/glfw3.h>

extern OctaCubic::World world;
extern OctaCubic::Player* player_ptr;
extern OctaCubic::Shader shader;
extern OctaCubic::Shader shNormal;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void cursorPosCallback(GLFWwindow* window, double xPos, double yPos);
void windowSizeCallback(GLFWwindow* window, int width, int height);
void toggleFullScreen(GLFWwindow* window);

inline void setInputs(GLFWwindow* window) {
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetWindowSizeCallback(window, windowSizeCallback);
}

inline void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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
    if (key == GLFW_KEY_F4 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        showLightSpaceDepth = !showLightSpaceDepth;
    }
    if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
        isFirstPersonView = !isFirstPersonView;
        if (isFirstPersonView) {
            CursorControlCam = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            glfwSetCursorPos(window, static_cast<double>(windowWidth) / 2, static_cast<double>(windowHeight) / 2);
        } else {
            CursorControlCam = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    if (key == GLFW_KEY_F6 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        showBlockCoord = !showBlockCoord;
    }
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) toggleFullScreen(window);
    if (key == GLFW_KEY_F12 && action == GLFW_PRESS) {
        world.generate();
        worldVertCount = 0;
        clearVerticesBuffer(verticesBuff, indicesBuff, VERTICES_BUFFER_SIZE, INDICES_BUFFER_SIZE);
        clearVerticesBuffer(verticesWaterBuff, indicesWaterBuff, VERTICES_BUFFER_SIZE, INDICES_BUFFER_SIZE);
        genWorldVertices();
        setupBuffers(terrainVAO, terrainVBO, terrainEBO, verticesBuff, indicesBuff);
        setupBuffers(terrWaterVAO, terrWaterVBO, terrWaterEBO, verticesWaterBuff, indicesWaterBuff);
        if (player_ptr) player_ptr->generatePlayerSpawn();
    }
}

inline void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouseButtonLeftPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouseButtonLeftPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        mouseButtonRightPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        mouseButtonRightPressed = false;
    }
    if (!isFirstPersonView) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            CursorControlCam = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            glfwSetCursorPos(window, static_cast<double>(windowWidth) / 2, static_cast<double>(windowHeight) / 2);
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            CursorControlCam = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

inline void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    if (yOffset > 0 && CamValDistance > 1 || yOffset < 0 && CamValDistance < 3)
        CamValDistance -= (CamValDistance < 2 ? .05f : 0.1f) * static_cast<float>(yOffset);
}

inline void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
    if (!CursorControlCam) return;
    const float halfWidth = static_cast<float>(windowWidth) / 2;
    const float halfHeight = static_cast<float>(windowHeight) / 2;
    CursorDeltaX = static_cast<float>(xPos) - halfWidth;
    CursorDeltaY = static_cast<float>(yPos) - halfHeight;
    glfwSetCursorPos(window, (double)halfWidth, (double)halfHeight);
}

inline void windowSizeCallback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    int x, y;
    glfwGetFramebufferSize(window, &x, &y);
    glViewport(0, 0, x, y);
}

inline void toggleFullScreen(GLFWwindow* window) {
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
