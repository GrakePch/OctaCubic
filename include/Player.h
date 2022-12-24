#pragma once
#include <glm/vec3.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "World.h"

#define PI 3.14159265

namespace MinecraftAlter
{
    class Player {
    public:
        glm::vec3 dimensions{.6f, 1.8f, .6f};
        float eyeHeight = 1.62f;
        glm::vec3 location{0, 72, 0};
        float pitch = 0;
        float yaw = 0; // yaw = 0 facing x+
        float speedWalk = 4.317f;
        World* world_ptr = nullptr;

        Player() = default;

        static float clamp(float x, float min, float max) {
            return x > max ? max : x < min ? min : x;
        }

        bool blockHasCollision(int x, int y, int z) const {
            if (x < 0 || y < 0 || z < 0
                || x >= world_ptr->worldDimX
                || y >= world_ptr->worldDimY
                || z >= world_ptr->worldDimZ)
                return false;
            const int blockId = world_ptr->world[x][z][y];
            return blockId != 0 && blockId != 10;
        }

        void clampToCollision(glm::vec3 deltaLocation) {
            if (!world_ptr) {
                location.x += deltaLocation.x;
                location.y += deltaLocation.y;
                location.z += deltaLocation.z;
                return;
            }
            int locXI = (int)floor(location.x);
            int locYI = (int)floor(location.y);
            int locZI = (int)floor(location.z);
            float newLocX = location.x + deltaLocation.x;
            float newLocY = location.y + deltaLocation.y;
            float newLocZ = location.z + deltaLocation.z;
            location.x = clamp(newLocX,
                               blockHasCollision(locXI - 1, locYI, locZI)
                                   ? (float)locXI + dimensions.x / 2
                                   : newLocX,
                               blockHasCollision(locXI + 1, locYI, locZI)
                                   ? (float)locXI - dimensions.x / 2 + 1
                                   : newLocX
            );
            location.y = clamp(newLocY,
                               blockHasCollision(locXI, locYI - 1, locZI)
                                   ? (float)locYI
                                   : newLocY,
                               blockHasCollision(locXI, locYI + 2, locZI)
                                   ? (float)locYI - dimensions.y + 2
                                   : newLocY
            );
            location.z = clamp(newLocZ,
                               blockHasCollision(locXI, locYI, locZI - 1)
                                   ? (float)locZI + dimensions.z / 2
                                   : newLocZ,
                               blockHasCollision(locXI, locYI, locZI + 1)
                                   ? (float)locZI - dimensions.z / 2 + 1
                                   : newLocZ
            );
        }

        void updateLocation(float interval, int inputForward, int inputRight, int inputUp) {
            const glm::vec2 moveXZ = glm::normalize(glm::vec2(inputForward, inputRight));
            if (inputForward) {
                const float moveDist = speedWalk * interval * moveXZ.x;
                clampToCollision(glm::vec3(
                    sin(static_cast<double>(yaw) * PI / 180),
                    0,
                    -cos(static_cast<double>(yaw) * PI / 180)
                ) * moveDist);
            }
            if (inputRight) {
                const float moveDist = speedWalk * interval * moveXZ.y;
                clampToCollision(glm::vec3(
                    cos(static_cast<double>(yaw) * PI / 180),
                    0,
                    sin(static_cast<double>(yaw) * PI / 180)
                ) * moveDist);
            }
            if (inputUp) {
                const float moveDist = speedWalk * interval * static_cast<float>(inputUp);
                clampToCollision({0, moveDist, 0});
            }
        }

        glm::mat4 rotateFacing(glm::mat4 CamView, float deltaX, float deltaY, float sensitivity) {
            pitch = glm::clamp<float>(pitch + deltaY * sensitivity * .5f, -90, 90);
            yaw += deltaX * sensitivity * .5f;
            if (yaw >= 180) yaw -= 360;
            if (yaw < -180) yaw += 360;
            CamView = glm::rotate(CamView, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
            CamView = glm::rotate(CamView, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
            CamView = glm::translate(CamView, -location - glm::vec3{0, eyeHeight, 0});
            return CamView;
        }

        bool generatePlayerSpawn() {
            if (!world_ptr) {
                printf("Error: Player is not linked with world!");
                return false;
            }
            location.x = floor(world_ptr->worldCenter.x) + .5f;
            location.z = floor(world_ptr->worldCenter.z) + .5f;
            int surfaceY;
            for (surfaceY = 0; surfaceY < world_ptr->worldDimY; ++surfaceY)
                if (!world_ptr->world
                    [static_cast<int>(location.x)]
                    [static_cast<int>(location.z)]
                    [surfaceY])
                    break;
            location.y = static_cast<float>(surfaceY);
            return true;
        }
    };
}
