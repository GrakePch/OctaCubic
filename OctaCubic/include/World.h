#pragma once
#include <array>
#include <glm/vec3.hpp>

#include "Quad.h"

#define DIM 128

namespace OctaCubic
{
    struct CoordinatesAndFace {
        bool isHit = false;
        int x;
        int y;
        int z;
        face f;
        CoordinatesAndFace(): x(0), y(0), z(0), f(xPos) {}
        CoordinatesAndFace(const int x, const int y, const int z, const face face, const bool hit = false)
            : isHit(hit), x(x), y(y), z(z), f(face) {}
    };
    
    class World {
    public:
        std::array<std::array<std::array<int, DIM>, DIM>, DIM> world{};
        int worldDimX;
        int worldDimZ;
        int worldDimY;
        int worldDimMax;
        glm::vec3 worldCenter;

        int altitudeSeaSurface = 23;
    
        World();
        static void randomizeSeed();
        void generate();
        
        bool isOutOfBound(const int x, const int y, const int z) const;

        int getBlockId(const int x, const int y, const int z) const;

        int setBlockId(const int x, const int y, const int z, const int blockId);

        static bool isBlockOpaque(const int blockId);

        bool isBlockOpaqueAtCoord(const int x, const int y, const int z) const;

        static glm::ivec3 insideBlockCoordinates(const glm::vec3 pos);

        CoordinatesAndFace lineTraceToFace(const glm::vec3 start, const glm::vec3 dir, const float len) const;
    };
}
