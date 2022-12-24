#include "World.h"

#include <ctime>

#include "perlin.h"

using namespace MinecraftAlter;

World::World() {
    worldDimX = static_cast<int>(world.size());
    worldDimZ = static_cast<int>(world[0].size());
    worldDimY = static_cast<int>(world[0][0].size());
    worldDimMax = std::max(worldDimX, std::max(worldDimY, worldDimZ));
    worldCenter = glm::vec3{
        static_cast<float>(worldDimX) / 2,
        static_cast<float>(worldDimY) / 4,
        static_cast<float>(worldDimZ) / 2
    };
}

void World::randomizeSeed() {
    srand(static_cast<int>(time(nullptr)));
}


void World::generate() {
    const int perlinSeed = rand();
    const auto AltitudeSeaSurfaceF = static_cast<float>(AltitudeSeaSurface);
    for (int x = 0; x < worldDimX; ++x) {
        for (int z = 0; z < worldDimZ; ++z) {
            float surfaceHeightF =
                (perlin(perlinSeed, static_cast<float>(x) / 32, static_cast<float>(z) / 32) * .5f + .5f) * 32
                + (perlin(perlinSeed, static_cast<float>(x) / 16, static_cast<float>(z) / 16) * .5f) * 12
                + 10;
            // Higher mountains
            if (surfaceHeightF > AltitudeSeaSurfaceF + 5) {
                surfaceHeightF = AltitudeSeaSurfaceF + 5 + (surfaceHeightF - AltitudeSeaSurfaceF - 5) * 2;
            }
            // Deeper water
            if (surfaceHeightF < AltitudeSeaSurfaceF - 3) {
                surfaceHeightF = AltitudeSeaSurfaceF - 3 - (AltitudeSeaSurfaceF - 3 - surfaceHeightF) * 2;
            }
            // General Terrain
            for (int y = 0; y < worldDimY; ++y) {
                const auto yF = static_cast<float>(y);
                world[x][z][y] = y == 0
                                     ? 1 // Bedrock @ y = 0
                                     : yF < surfaceHeightF - 4
                                     ? 2 // Stone
                                     : yF < surfaceHeightF - 1
                                     ? 3 // Dirt
                                     : yF < surfaceHeightF
                                     // For the solid surface: below sea+2 -> Sand; below sea+12 -> Grass; above -> Snow
                                     ? surfaceHeightF > AltitudeSeaSurfaceF + 2
                                           ? surfaceHeightF > AltitudeSeaSurfaceF + 12
                                                 ? 6
                                                 : 4
                                           : 5
                                     // Above the solid surface: below sea -> Water; above -> Air
                                     : yF > AltitudeSeaSurfaceF
                                     ? 0
                                     : 10;
            }
        }
    }
}

void World::generatePlayerSpawn() const {
    CurrentPlayer->location.x = floor(worldCenter.x) + .5f;
    CurrentPlayer->location.z = floor(worldCenter.z) + .5f;
    int surfaceY;
    for (surfaceY = 0; surfaceY < worldDimY; ++surfaceY)
        if (!world
            [static_cast<int>(CurrentPlayer->location.x)]
            [static_cast<int>(CurrentPlayer->location.z)]
            [surfaceY])
            break;
    CurrentPlayer->location.y = static_cast<float>(surfaceY);
}
