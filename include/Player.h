#pragma once
#include <glm/vec3.hpp>
#include <glm/ext/matrix_transform.hpp>

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

        Player() = default;

        void move(float interval, int inputForward, int inputRight, int inputUp) {
            const glm::vec2 moveXZ = glm::normalize(glm::vec2(inputForward, inputRight));
            if (inputForward) {
                const float moveDist = speedWalk * interval * moveXZ.x;
                location += glm::vec3(
                    sin(static_cast<double>(yaw) * PI / 180),
                    0,
                    -cos(static_cast<double>(yaw) * PI / 180)
                ) * moveDist;
            }
            if (inputRight) {
                const float moveDist = speedWalk * interval * moveXZ.y;
                location += glm::vec3(
                    cos(static_cast<double>(yaw) * PI / 180),
                    0,
                    sin(static_cast<double>(yaw) * PI / 180)
                ) * moveDist;
            }
            if (inputUp) {
                const float moveDist = speedWalk * interval * static_cast<float>(inputUp);
                location += glm::vec3(0, moveDist, 0);
            }
        }

        glm::mat4 rotateFacing(glm::mat4 CamView, float deltaX, float deltaY, float sensitivity) {
            pitch = glm::clamp<float>(pitch + deltaY * sensitivity * .5f, -90, 90);
            yaw += deltaX * sensitivity * .5f;
            CamView = glm::rotate(CamView, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
            CamView = glm::rotate(CamView, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
            CamView = glm::translate(CamView, -location - glm::vec3{0.0f, eyeHeight + .5f, 0.0f});
            return CamView;
        }
    };
}
