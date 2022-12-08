#ifndef MINECRAFTALTER_CUBE_H
#define MINECRAFTALTER_CUBE_H
#include "MinecraftAlter.h"

namespace MinecraftAlter
{
    class Cube {
    public:
        std::array<Vertex, 36> vertices{};
    private:
        std::array<Vertex, 8> cubeCornersRaw = {
            Vertex{{-.5f, -.5f, -.5f}},
            Vertex{{-.5f, -.5f, 0.5f}},
            Vertex{{0.5f, -.5f, 0.5f}},
            Vertex{{0.5f, -.5f, -.5f}},
            Vertex{{-.5f, 0.5f, -.5f}},
            Vertex{{-.5f, 0.5f, 0.5f}},
            Vertex{{0.5f, 0.5f, 0.5f}},
            Vertex{{0.5f, 0.5f, -.5f}}
        };
        uint32 cubeVertIdx[36] = {
            // Face -Y
            0, 1, 2,
            0, 2, 3,
            // Face +Y
            6, 5, 4,
            7, 6, 4,
            // Face -Z
            7, 4, 0,
            3, 7, 0,
            // Face +Z
            1, 5, 6,
            1, 6, 2,
            // Face -X
            5, 1, 0,
            4, 5, 0,
            // Face +X
            3, 2, 6,
            3, 6, 7
        }; // Define the triangle in the order of Counter-clockwise for face culling
    public:
        Cube() {
            for (int i = 0; i < 36; ++i) {
                vertices[i].position = cubeCornersRaw[cubeVertIdx[i]].position;
                vertices[i].normal = i < 6
                                         ? std::array<float, 3>{.0f, -1.0f, .0f}
                                         : i < 6 * 2
                                         ? std::array<float, 3>{.0f, +1.0f, .0f}
                                         : i < 6 * 3
                                         ? std::array<float, 3>{.0f, .0f, -1.0f}
                                         : i < 6 * 4
                                         ? std::array<float, 3>{.0f, .0f, +1.0f}
                                         : i < 6 * 5
                                         ? std::array<float, 3>{-1.0f, .0f, .0f}
                                         : std::array<float, 3>{+1.0f, .0f, .0f};
            }
        }
    };
}

#endif
