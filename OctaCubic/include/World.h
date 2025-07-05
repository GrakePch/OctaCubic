#pragma once
#include <unordered_map>
#include <glm/vec3.hpp>

#include "Chunk.h"
#include "Quad.h"

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
        int altitudeSeaSurface = 23;
        float worldDimMax = 256.0f;

        World();
        static void randomizeSeed();
        int generateSeed();

        bool isOutOfBound(const glm::ivec3& coordWorld) const;
        int getBlockId(const glm::ivec3& coordWorld);
        int setBlockId(const glm::ivec3& coordWorld, const uint8_t blockId);

        static bool isBlockOpaque(const int blockId);
        bool isBlockOpaqueAtCoord(const glm::ivec3& coordWorld);

        static glm::ivec3 insideBlockCoordinates(const glm::vec3 pos);

        CoordinatesAndFace lineTraceToFace(const glm::vec3 start, const glm::vec3 dir, const float len);

        static glm::ivec3 getCoordLocalToChunk(const glm::ivec3 coordWorld);
        static glm::ivec3 getCoordLocalToChunk(const glm::vec3 coordWorld);
        static glm::ivec3 getCoordChunk(const glm::ivec3 coordWorld);
        static glm::ivec3 getCoordChunk(const glm::vec3 coordWorld);

        void smartRenderingPreprocess(const glm::ivec3 center, const int viewDistance);
        void smartRenderingPreprocess(const glm::vec3 center, const int viewDistance);
        void renderInQueueOpaque();
        void renderInQueueWater();

    private:
        int seed_;

        std::vector<Chunk*> renderWaitingQueue_;

        std::unordered_map<chunk_coord, Chunk, ChunkCoordHash> chunkMap_;

        bool isChunkCreated(const chunk_coord c);
        Chunk* getChunk(const chunk_coord c);
    };
}
