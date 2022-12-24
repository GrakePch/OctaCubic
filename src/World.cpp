#include "World.h"

#include <ctime>

#include "perlin.h"

using namespace MinecraftAlter;

World::World() {
    worldDimX = static_cast<int>(world.size());
    worldDimZ = static_cast<int>(world[0].size());
    worldDimY = static_cast<int>(world[0][0].size());
    worldDimMax = std::max(worldDimX, std::max(worldDimY, worldDimZ));
}

void World::randomizeSeed() {
    srand(static_cast<int>(time(nullptr)));
}


void World::generate() {
    const int perlinSeed = rand();
    for (int x = 0; x < worldDimX; ++x) {
        for (int z = 0; z < worldDimZ; ++z) {
            float surfaceHeightF =
                (perlin(perlinSeed, static_cast<float>(x) / 32, static_cast<float>(z) / 32) * .5f + .5f) * 32
                + (perlin(perlinSeed, static_cast<float>(x) / 16, static_cast<float>(z) / 16) * .5f) * 12
                + 10;
            // Higher mountains
            if (surfaceHeightF > AltitudeSeaSurface + 5) {
                surfaceHeightF = AltitudeSeaSurface + 5 + (surfaceHeightF - AltitudeSeaSurface - 5) * 2;
            }
            // Deeper water
            if (surfaceHeightF < AltitudeSeaSurface - 3) {
                surfaceHeightF = AltitudeSeaSurface - 3 - (AltitudeSeaSurface - 3 - surfaceHeightF) * 2;
            }
            // General Terrain
            for (int y = 0; y < worldDimY; ++y) {
                world[x][z][y] = y == 0
                                     ? 1 // Bedrock @ y = 0
                                     : y < surfaceHeightF - 4
                                     ? 2 // Stone
                                     : y < surfaceHeightF - 1
                                     ? 3 // Dirt
                                     : y < surfaceHeightF
                                     // For the solid surface: below sea+2 -> Sand; below sea+12 -> Grass; above -> Snow
                                     ? surfaceHeightF > AltitudeSeaSurface + 2
                                           ? surfaceHeightF > AltitudeSeaSurface + 12
                                                 ? 6
                                                 : 4
                                           : 5
                                     // Above the solid surface: below sea -> Water; above -> Air
                                     : y > AltitudeSeaSurface
                                     ? 0
                                     : 10;
            }
        }
    }
}
