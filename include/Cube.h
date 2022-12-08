#ifndef MINECRAFTALTER_CUBE_H
#define MINECRAFTALTER_CUBE_H
#include "MinecraftAlter.h"

namespace MinecraftAlter
{
    class Cube {
    public:
        int cubeId = 1; // Bedrock
        glm::vec3 position{0};
        std::array<Vertex, 36> vertices{};
        int verticesFilled = 0;
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
        Cube() = default;
        std::array<Vertex, 36> updateVerticesFacesShown(const int* showFaces) {
            // [-Y, +Y, -Z, +Z, -X, +X]
            verticesFilled = 0;
            for (int face = 0; face < 6; ++face) {
                // Loop over showFaces[6]
                if (showFaces[face]) {
                    for (int v = 0; v < 6; ++v) {
                        // Add following 6 cubeVertIdx to cubeVertices{}
                        vertices[verticesFilled].position = cubeCornersRaw[cubeVertIdx[face * 6 + v]].position;
                        vertices[verticesFilled].normal = face == 0
                                                              ? std::array<float, 3>{.0f, -1.0f, .0f}
                                                              : face == 1
                                                              ? std::array<float, 3>{.0f, +1.0f, .0f}
                                                              : face == 2
                                                              ? std::array<float, 3>{.0f, .0f, -1.0f}
                                                              : face == 3
                                                              ? std::array<float, 3>{.0f, .0f, +1.0f}
                                                              : face == 4
                                                              ? std::array<float, 3>{-1.0f, .0f, .0f}
                                                              : std::array<float, 3>{+1.0f, .0f, .0f};
                        ++verticesFilled;
                    }
                }
            }
            return vertices;
        }
    };
}

#endif
