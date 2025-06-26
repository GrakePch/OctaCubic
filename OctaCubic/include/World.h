#pragma once
#include <array>
#include <glm/vec3.hpp>

#define DIM 200

namespace OctaCubic
{
    class World {
    public:
        std::array<std::array<std::array<int, DIM>, DIM>, DIM> world{};
        int worldDimX;
        int worldDimZ;
        int worldDimY;
        int worldDimMax;
        glm::vec3 worldCenter;

        int AltitudeSeaSurface = 23;
    
        World();
        static void randomizeSeed();
        void generate();
        
        bool isOutOfBound(const int x, const int y, const int z) const;

        int getBlockId(const int x, const int y, const int z) const;

        int setBlockId(const int x, const int y, const int z, const int blockId);

        static bool isBlockOpaque(const int blockId);

        bool isBlockOpaqueAtCoord(const int x, const int y, const int z) const;
    };
}
