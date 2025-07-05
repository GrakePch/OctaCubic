#include "World.h"

#include <ctime>
#include <vector>

extern size_t worldVertCount;

using namespace OctaCubic;

World::World() = default;

void World::randomizeSeed() {
    srand(static_cast<int>(time(nullptr)));
}

int World::generateSeed() {
    seed_ = rand();
    return seed_;
}

bool World::isOutOfBound(const glm::ivec3& coordWorld) const {
    if (coordWorld.y < 0 || coordWorld.y >= Chunk::height)
        return true; // Out of bound in Y-axis
    return false; // TODO: what if run out of chunk in x and z directions?
}

int World::getBlockId(const glm::ivec3& coordWorld) {
    if (isOutOfBound(coordWorld)) {
        // printf("Out of bound: %d %d %d\n", coordWorld.x, coordWorld.y, coordWorld.z);
        return -1; // Out of bound
    }
    const chunk_coord cc = getCoordChunk(coordWorld);
    const Chunk* ptr_chunk = getChunk(cc);
    if (!ptr_chunk) {
        // printf("Chunk not found for chunk coord: %d %d %d\n", cc.x, cc.y, cc.z);
        return -1; // Chunk not found
    }
    const glm::ivec3 coordLocal = getCoordLocalToChunk(coordWorld);
    return ptr_chunk->getBlockId(coordLocal);
}

int World::setBlockId(const glm::ivec3& coordWorld, const uint8_t blockId) {
    if (isOutOfBound(coordWorld))
        return -1; // Out of bound
    const chunk_coord cc = getCoordChunk(coordWorld);
    Chunk* ptr_chunk = getChunk(cc);
    if (!ptr_chunk) {
        return -1; // Chunk not found
    }
    const glm::ivec3 coordLocal = getCoordLocalToChunk(coordWorld);
    return ptr_chunk->setBlockId(coordLocal, blockId);
}

bool World::isBlockOpaque(const int blockId) {
    return blockId != 0 && blockId != 10;
}

bool World::isBlockOpaqueAtCoord(const glm::ivec3& coordWorld) {
    return isBlockOpaque(getBlockId(coordWorld));
}

glm::ivec3 World::insideBlockCoordinates(const glm::vec3 pos) {
    const int coordX = static_cast<int>(floor(pos.x));
    const int coordY = static_cast<int>(floor(pos.y));
    const int coordZ = static_cast<int>(floor(pos.z));
    return glm::ivec3{coordX, coordY, coordZ};
}

CoordinatesAndFace World::lineTraceToFace(const glm::vec3 start, const glm::vec3 dir, const float len) {
    const glm::vec3 end = start + dir * len;
    glm::vec3 hitPlainX;
    glm::vec3 hitPlainY;
    glm::vec3 hitPlainZ;
    CoordinatesAndFace hitBlockInX;
    CoordinatesAndFace hitBlockInY;
    CoordinatesAndFace hitBlockInZ;
    {
        // Compute hit block in X-axis face
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
            if (getBlockId(currentBlock) > 0) {
                hitPlainX = currentPos;
                hitBlockInX = CoordinatesAndFace(currentBlock.x, currentBlock.y, currentBlock.z,
                                                 dir.x < 0 ? xPos : xNeg, true);
                break;
            }
        }
    }
    {
        // Compute hit block in Y-axis face
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
            if (getBlockId(currentBlock) > 0) {
                hitPlainY = currentPos;
                hitBlockInY = CoordinatesAndFace(currentBlock.x, currentBlock.y, currentBlock.z,
                                                 dir.y < 0 ? yPos : yNeg, true);
                break;
            }
        }
    }
    {
        // Compute hit block in Z-axis face
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
            if (getBlockId(currentBlock) > 0) {
                hitPlainZ = currentPos;
                hitBlockInZ = CoordinatesAndFace(currentBlock.x, currentBlock.y, currentBlock.z,
                                                 dir.z < 0 ? zPos : zNeg, true);
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

glm::ivec3 World::getCoordLocalToChunk(const glm::ivec3 coordWorld) {
    return glm::ivec3{
        (coordWorld.x % Chunk::width + Chunk::width) % Chunk::width,
        coordWorld.y,
        (coordWorld.z % Chunk::width + Chunk::width) % Chunk::width
    };
}

glm::ivec3 World::getCoordLocalToChunk(const glm::vec3 coordWorld) {
    return getCoordLocalToChunk(insideBlockCoordinates(coordWorld));
}

glm::ivec3 World::getCoordChunk(const glm::ivec3 coordWorld) {
    return glm::ivec3{
        coordWorld.x < 0 ? (coordWorld.x - Chunk::width + 1) / Chunk::width : coordWorld.x / Chunk::width,
        0,
        coordWorld.z < 0 ? (coordWorld.z - Chunk::width + 1) / Chunk::width : coordWorld.z / Chunk::width
    };
}

glm::ivec3 World::getCoordChunk(const glm::vec3 coordWorld) {
    return getCoordChunk(insideBlockCoordinates(coordWorld));
}

void World::smartRenderingPreprocess(const glm::ivec3 center, const int viewDistance) {
    const glm::ivec3 centerChunk = getCoordChunk(center);
    const int minX = centerChunk.x - viewDistance;
    const int maxX = centerChunk.x + viewDistance;
    const int minZ = centerChunk.z - viewDistance;
    const int maxZ = centerChunk.z + viewDistance;
    // Free GPU memory of chunks out of view
    std::vector<Chunk*> chunksToFree;
    for (auto c : Chunk::chunkInGPUSet) {
        if (c.x < minX || c.x > maxX || c.z < minZ || c.z > maxZ) {
            chunksToFree.push_back(getChunk(c));
        }
    }
    for (Chunk* chunk : chunksToFree) {
        if (chunk != nullptr) {
            chunk->freeGPU();
        }
    }
    // Render chunks in view
    renderWaitingQueue_.clear();
    worldVertCount = 0;
    //// Generate chunks in view if it never generated
    for (int x = minX; x <= maxX; ++x) {
        for (int z = minZ; z <= maxZ; ++z) {
            const chunk_coord chunkCoord{x, 0, z};
            Chunk* ptr_chunk = getChunk(chunkCoord);
            if (ptr_chunk == nullptr) {
                Chunk newChunk(x, z);
                newChunk.bindWorld(this);
                newChunk.genTerrain(seed_);
                chunkMap_.emplace(chunkCoord, std::move(newChunk));
                ptr_chunk = &chunkMap_[chunkCoord];
            }
        }
    }
    //// After all chunks in view generated, build mesh and send to GPU
    for (int x = minX; x <= maxX; ++x) {
        for (int z = minZ; z <= maxZ; ++z) {
            Chunk* ptr_chunk = getChunk({x, 0, z});
            if (ptr_chunk->isDirty) ptr_chunk->buildMesh();
            if (!ptr_chunk->isInGPU()) ptr_chunk->sendToGPU();
            renderWaitingQueue_.push_back(ptr_chunk);
            worldVertCount += ptr_chunk->getNumVertices();
        }
    }
}

void World::smartRenderingPreprocess(const glm::vec3 center, const int viewDistance) {
    smartRenderingPreprocess(insideBlockCoordinates(center), viewDistance);
}

void World::renderInQueueOpaque() {
    for (const auto ptr_chunk : renderWaitingQueue_) {
        ptr_chunk->renderOpaque();
    }
}

void World::renderInQueueWater() {
    for (const auto ptr_chunk : renderWaitingQueue_) {
        ptr_chunk->renderWater();
    }
}

bool World::isChunkCreated(const chunk_coord c) {
    return chunkMap_.find(c) != chunkMap_.end();
}

Chunk* World::getChunk(const chunk_coord c) {
    const auto it = chunkMap_.find(c);
    if (it != chunkMap_.end()) {
        return &it->second;
    }
    return nullptr;
}
