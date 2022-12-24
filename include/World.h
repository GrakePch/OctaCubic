#pragma once
#include <array>
#include <glm/vec3.hpp>

#include "Player.h"

namespace MinecraftAlter
{
    class World {
    public:
        std::array<std::array<std::array<int, 72>, 72>, 72> world{};
        int worldDimX;
        int worldDimZ;
        int worldDimY;
        int worldDimMax;
        glm::vec3 worldCenter;

        int AltitudeSeaSurface = 23;
        Player* CurrentPlayer = nullptr;
    
        World();
        static void randomizeSeed();
        void generate();
        
        void generatePlayerSpawn() const;
    };
}
