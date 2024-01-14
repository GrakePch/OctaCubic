#pragma once
#include <array>
#include <glm/vec3.hpp>

#define DIM 32

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
    };
}
