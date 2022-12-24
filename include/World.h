#pragma once
#include <array>

namespace MinecraftAlter
{
    class World {
    public:
        int worldDimX;
        int worldDimZ;
        int worldDimY;
        int worldDimMax;
        std::array<std::array<std::array<int, 72>, 72>, 72> world{};

        float AltitudeSeaSurface = 23;
    
        World();
        static void randomizeSeed();
        void generate();
    };
}
