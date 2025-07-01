#include "World.h"

#include <ctime>
#include <vector>

#include "perlin.h"

using namespace OctaCubic;

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
    const auto AltitudeSeaSurfaceF = static_cast<float>(altitudeSeaSurface);
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

bool World::isOutOfBound(const int x, const int y, const int z) const {
    return x < 0 || y < 0 || z < 0 || x >= worldDimX || y >= worldDimY || z >= worldDimZ;
}

int World::getBlockId(const int x, const int y, const int z) const {
    if (isOutOfBound(x, y, z))
        return -1; // Out of bound
    return world[x][z][y];
}

int World::setBlockId(const int x, const int y, const int z, const int blockId) {
    if (isOutOfBound(x, y, z))
        return -1; // Out of bound
    if (world[x][z][y] == blockId) return -2; // No change
    world[x][z][y] = blockId;
    return blockId;
}

bool World::isBlockOpaque(const int blockId) {
    return blockId != 0 && blockId != 10;
}

bool World::isBlockOpaqueAtCoord(const int x, const int y, const int z) const {
    return isBlockOpaque(getBlockId(x, y, z));
}

glm::ivec3 World::insideBlockCoordinates(const glm::vec3 pos) {
    const int coordX = static_cast<int>(floor(pos.x));
    const int coordY = static_cast<int>(floor(pos.y));
    const int coordZ = static_cast<int>(floor(pos.z));
    return glm::ivec3{coordX, coordY, coordZ};
}

CoordinatesAndFace World::lineTraceToFace(const glm::vec3 start, const glm::vec3 dir, const float len) const {
    const glm::vec3 end = start + dir * len;
    glm::vec3 hitPlainX;
    glm::vec3 hitPlainY;
    glm::vec3 hitPlainZ;
    CoordinatesAndFace hitBlockInX;
    CoordinatesAndFace hitBlockInY;
    CoordinatesAndFace hitBlockInZ;
    { // Compute hit block in X-axis face
        const int deltaX = end.x > start.x ? 1 : -1;
        const int startX = (int)ceil(start.x);
        const int endX = (int)floor(end.x);
        for (int x = startX; deltaX > 0 ? x <= endX : x >= endX; x += deltaX) {
            const float t = ((float)x - start.x) / dir.x;
            if (t < 0) continue;
            const float y = start.y + t * dir.y;
            const float z = start.z + t * dir.z;
            const glm::vec3 currentPos{x, y, z};
            glm::ivec3 currentBlock = insideBlockCoordinates(currentPos);
            if (dir.x < 0) currentBlock.x -= 1;
            if (getBlockId(currentBlock.x, currentBlock.y, currentBlock.z) > 0) {
                hitPlainX = currentPos;
                hitBlockInX = CoordinatesAndFace(currentBlock.x, currentBlock.y, currentBlock.z, dir.x < 0 ? xPos : xNeg, true);
                break;
            }
        }
    }
    { // Compute hit block in Y-axis face
        const int deltaY = end.y > start.y ? 1 : -1;
        const int startY = (int)ceil(start.y);
        const int endY = (int)floor(end.y);
        for (int y = startY; deltaY > 0 ? y <= endY : y >= endY; y += deltaY) {
            const float t = ((float)y - start.y) / dir.y;
            if (t < 0) continue;
            const float x = start.x + t * dir.x;
            const float z = start.z + t * dir.z;
            const glm::vec3 currentPos{x, y, z};
            glm::ivec3 currentBlock = insideBlockCoordinates(currentPos);
            if (dir.y < 0) currentBlock.y -= 1;
            if (getBlockId(currentBlock.x, currentBlock.y, currentBlock.z) > 0) {
                hitPlainY = currentPos;
                hitBlockInY = CoordinatesAndFace(currentBlock.x, currentBlock.y, currentBlock.z, dir.y < 0 ? yPos : yNeg, true);
                break;
            }
        }
    }
    { // Compute hit block in Z-axis face
        const int deltaZ = end.z > start.z ? 1 : -1;
        const int startZ = (int)ceil(start.z);
        const int endZ = (int)floor(end.z);
        for (int z = startZ; deltaZ > 0 ? z <= endZ : z >= endZ; z += deltaZ) {
            const float t = ((float)z - start.z) / dir.z;
            if (t < 0) continue;
            const float x = start.x + t * dir.x;
            const float y = start.y + t * dir.y;
            const glm::vec3 currentPos{x, y, z};
            glm::ivec3 currentBlock = insideBlockCoordinates(currentPos);
            if (dir.z < 0) currentBlock.z -= 1;
            if (getBlockId(currentBlock.x, currentBlock.y, currentBlock.z) > 0) {
                hitPlainZ = currentPos;
                hitBlockInZ = CoordinatesAndFace(currentBlock.x, currentBlock.y, currentBlock.z, dir.z < 0 ? zPos : zNeg, true);
                break;
            }
        }
    }
    // Compare hit blocks and return the closest one
    if (hitBlockInX.isHit && hitBlockInY.isHit && hitBlockInZ.isHit) {
        const float distX = glm::length(hitPlainX - start);
        const float distY = glm::length(hitPlainY - start);
        const float distZ = glm::length(hitPlainZ - start);
        if (distX < distY && distX < distZ) return hitBlockInX;
        if (distY < distX && distY < distZ) return hitBlockInY;
        return hitBlockInZ;
    }
    if (hitBlockInX.isHit && hitBlockInY.isHit) {
        const float distX = glm::length(hitPlainX - start);
        const float distY = glm::length(hitPlainY - start);
        if (distX < distY) return hitBlockInX;
        return hitBlockInY;
    }
    if (hitBlockInX.isHit && hitBlockInZ.isHit) {
        const float distX = glm::length(hitPlainX - start);
        const float distZ = glm::length(hitPlainZ - start);
        if (distX < distZ) return hitBlockInX;
        return hitBlockInZ;
    }
    if (hitBlockInY.isHit && hitBlockInZ.isHit) {
        const float distY = glm::length(hitPlainY - start);
        const float distZ = glm::length(hitPlainZ - start);
        if (distY < distZ) return hitBlockInY;
        return hitBlockInZ;
    }
    if (hitBlockInX.isHit) return hitBlockInX;
    if (hitBlockInY.isHit) return hitBlockInY;
    if (hitBlockInZ.isHit) return hitBlockInZ;
    return hitBlockInX;
}
