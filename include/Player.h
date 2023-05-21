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
        glm::vec3 AimingAtBlockCoord{0, 0, 0};
        bool isAimingAtSomeBlock = false;
        float pitch = 0;
        float yaw = 0; // yaw = 0 facing x+
        bool isSprinting = false;
        bool isFloating = false;
        float speedWalk = 4.317f;
        float speedSprint = 5.612f;
        float speedJump = 10.0f;
        float gravityAc = -32.0f;
        float currSpeedUp = 0;
        float lastSpeedUp = 0;
        float tickSecond = 0; // updated in updateLocation();
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

        bool isInAir() const {
            return currSpeedUp == 0.0f && lastSpeedUp == 0.0f;
        }

        void applyNewLocation(glm::vec3 deltaLocation) {
            if (!world_ptr) {
                location.x += deltaLocation.x;
                location.y += deltaLocation.y;
                location.z += deltaLocation.z;
                return;
            }
            const int locXI = static_cast<int>(floor(location.x));
            const int locYI = static_cast<int>(floor(location.y));
            const int locZI = static_cast<int>(floor(location.z));
            const float newLocX = location.x + deltaLocation.x;
            const float newLocY = location.y + deltaLocation.y;
            const float newLocZ = location.z + deltaLocation.z;

            const float bias = 0.01f;
            // ^ if not use it, player will stuck in an edge of a block if walking closely along a wall

            ////// Update location X //////
            location.x = clamp(newLocX,
                               blockHasCollision(locXI - 1, locYI, locZI)
                                   ? static_cast<float>(locXI) + dimensions.x / 2
                                   : newLocX,
                               blockHasCollision(locXI + 1, locYI, locZI)
                                   ? static_cast<float>(locXI) - dimensions.x / 2 + 1
                                   : newLocX
            );
            if (location.z - locZI < dimensions.z / 2 - bias) {
                // player's z- side is out of current square
                location.x = clamp(location.x,
                                   blockHasCollision(locXI - 1, locYI, locZI - 1)
                                       ? static_cast<float>(locXI) + dimensions.x / 2
                                       : location.x,
                                   blockHasCollision(locXI + 1, locYI, locZI - 1)
                                       ? static_cast<float>(locXI) - dimensions.x / 2 + 1
                                       : location.x
                );
            }
            if (location.z - locZI > 1 - dimensions.z / 2 + bias) {
                // player's z+ side is out of current square
                location.x = clamp(location.x,
                                   blockHasCollision(locXI - 1, locYI, locZI + 1)
                                       ? static_cast<float>(locXI) + dimensions.x / 2
                                       : location.x,
                                   blockHasCollision(locXI + 1, locYI, locZI + 1)
                                       ? static_cast<float>(locXI) - dimensions.x / 2 + 1
                                       : location.x
                );
            }

            ////// Update Location Z //////
            location.z = clamp(newLocZ,
                               blockHasCollision(locXI, locYI, locZI - 1)
                                   ? static_cast<float>(locZI) + dimensions.z / 2
                                   : newLocZ,
                               blockHasCollision(locXI, locYI, locZI + 1)
                                   ? static_cast<float>(locZI) - dimensions.z / 2 + 1
                                   : newLocZ
            );
            if (location.x - locXI < dimensions.x / 2 - bias) {
                // player's x- side is out of current square
                location.z = clamp(location.z,
                                   blockHasCollision(locXI - 1, locYI, locZI - 1)
                                       ? static_cast<float>(locZI) + dimensions.z / 2
                                       : location.z,
                                   blockHasCollision(locXI - 1, locYI, locZI + 1)
                                       ? static_cast<float>(locZI) - dimensions.z / 2 + 1
                                       : location.z
                );
            }
            if (location.x - locXI > 1 - dimensions.x / 2 + bias) {
                // player's x+ side is out of current square
                location.z = clamp(location.z,
                                   blockHasCollision(locXI + 1, locYI, locZI - 1)
                                       ? static_cast<float>(locZI) + dimensions.z / 2
                                       : location.z,
                                   blockHasCollision(locXI + 1, locYI, locZI + 1)
                                       ? static_cast<float>(locZI) - dimensions.z / 2 + 1
                                       : location.z
                );
            }

            ////// Update Location Y //////
            const float locYOld = location.y;
            location.y = clamp(newLocY,
                               blockHasCollision(locXI, locYI - 1, locZI)
                                   ? static_cast<float>(locYI)
                                   : newLocY,
                               blockHasCollision(locXI, locYI + 2, locZI)
                                   ? static_cast<float>(locYI) - dimensions.y + 2
                                   : newLocY
            );
            if (location.x - locXI < dimensions.x / 2 - bias) {
                // player's x- side is out of current square
                location.y = clamp(location.y,
                                   blockHasCollision(locXI - 1, locYI - 1, locZI)
                                       ? static_cast<float>(locYI)
                                       : location.y,
                                   blockHasCollision(locXI - 1, locYI + 2, locZI)
                                       ? static_cast<float>(locYI) - dimensions.y + 2
                                       : location.y
                );
            }
            if (location.x - locXI > 1 - dimensions.x / 2 + bias) {
                // player's x+ side is out of current square
                location.y = clamp(location.y,
                                   blockHasCollision(locXI + 1, locYI - 1, locZI)
                                       ? static_cast<float>(locYI)
                                       : location.y,
                                   blockHasCollision(locXI + 1, locYI + 2, locZI)
                                       ? static_cast<float>(locYI) - dimensions.y + 2
                                       : location.y
                );
            }
            if (location.z - locZI < dimensions.z / 2 - bias) {
                // player's z- side is out of current square
                location.y = clamp(location.y,
                                   blockHasCollision(locXI, locYI - 1, locZI - 1)
                                       ? static_cast<float>(locYI)
                                       : location.y,
                                   blockHasCollision(locXI, locYI + 2, locZI - 1)
                                       ? static_cast<float>(locYI) - dimensions.y + 2
                                       : location.y
                );
            }
            if (location.z - locZI > 1 - dimensions.z / 2 + bias) {
                // player's z+ side is out of current square
                location.y = clamp(location.y,
                                   blockHasCollision(locXI, locYI - 1, locZI + 1)
                                       ? static_cast<float>(locYI)
                                       : location.y,
                                   blockHasCollision(locXI, locYI + 2, locZI + 1)
                                       ? static_cast<float>(locYI) - dimensions.y + 2
                                       : location.y
                );
            }
            currSpeedUp = (location.y - locYOld) / tickSecond; // Actual Speed Y
        }

        void updateLocation(float interval, int inputForward, int inputRight, int inputUp) {
            tickSecond = interval;
            lastSpeedUp = currSpeedUp;
            // Calculate horizontal movement
            const glm::vec2 moveXZ = glm::normalize(glm::vec2(inputForward, inputRight));
            float moveDistForward = 0, moveDistRight = 0, moveDistUp = 0;
            if (inputForward) moveDistForward = (isSprinting ? speedSprint : speedWalk) * interval * moveXZ.x;
            if (inputRight) moveDistRight = (isSprinting ? speedSprint : speedWalk) * interval * moveXZ.y;
            if (isFloating) {
                if (inputUp)
                    moveDistUp = (isSprinting ? speedSprint : speedWalk) * interval * static_cast<float>(inputUp);
            } else {
                // Calculate gravity
                currSpeedUp += gravityAc * interval; // Intended Speed Up
                moveDistUp = currSpeedUp * interval; // Intended Distance Up
            }

            const glm::vec3 moveDistXYZ =
                moveDistForward * glm::vec3(
                    sin(static_cast<double>(yaw) * PI / 180),
                    0,
                    -cos(static_cast<double>(yaw) * PI / 180)
                )
                + moveDistRight * glm::vec3(
                    cos(static_cast<double>(yaw) * PI / 180),
                    0,
                    sin(static_cast<double>(yaw) * PI / 180)
                )
                + glm::vec3(0, moveDistUp, 0);

            applyNewLocation(moveDistXYZ);
            // printf("speed up: %f\n", currSpeedUp);
        }

        void jump() {
            if (isInAir()) currSpeedUp = speedJump;
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

        glm::ivec3 getSteppingBlock() const {
            return glm::ivec3{
                static_cast<int>(floor(location.x)),
                static_cast<int>(floor(location.y)),
                static_cast<int>(floor(location.z))
            };
        }
    };
}
