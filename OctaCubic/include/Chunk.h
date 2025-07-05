#pragma once
#include <cstdint>
#include <unordered_set>
#include <vector>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

namespace OctaCubic
{
    using chunk_coord = glm::ivec3;

    struct ChunkCoordHash {
        std::size_t operator()(const chunk_coord& c) const {
            return std::hash<int>()(c.x) ^ std::hash<int>()(c.z << 16);
        }
    };

    class World; // Forward declaration

    class Chunk {
    public:
        static constexpr int width = 16;
        static constexpr int height = 256;
        static std::unordered_set<chunk_coord, ChunkCoordHash> chunkInGPUSet;

        Chunk();
        Chunk(int cX, int cZ);
        ~Chunk();

        void buildMesh();
        void sendToGPU();
        void renderOpaque() const;
        void renderWater() const;
        void freeGPU();

        bool isDirty = true;
        bool isInGPU() const;

        static bool isCoordValid(const glm::ivec3& c);
        int16_t getBlockId(const glm::ivec3& c) const;
        int16_t setBlockId(const glm::ivec3& c, const uint8_t blockId);

        void genTerrain(const int seed);

        static size_t getNumOfChunksInGPU();
        size_t getNumVertices() const;

        World* getWorld() const;
        void bindWorld(World* ptr_world);

        glm::ivec3 getCoordWorld(const glm::ivec3 coordLocal) const;

    private:
        uint8_t blocks_[width][height][width]{};
        chunk_coord chunkCoord_;
        glm::uint vaoOpaque_ = 0;
        glm::uint vboOpaque_ = 0;
        glm::uint vaoWater_ = 0;
        glm::uint vboWater_ = 0;
        size_t numVertices_ = 0;
        World* ptr_world_ = nullptr;

        struct Vertex {
            float x, y, z;
            float nx, ny, nz;
            float u, v;
            float id;

            Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v, uint8_t id)
                : x(x), y(y), z(z), nx(nx), ny(ny), nz(nz), u(u), v(v), id(id) {}
        };

        std::vector<Vertex> meshDataOpaque_;
        std::vector<Vertex> meshDataWater_;

        // Helper functions
        void genMeshData();
        void genQuadData(std::vector<Vertex>& meshData, const float* vertices, const uint8_t blockId, const int x,
                         const int y, const int z);
        static bool isBlockOpaque(const int blockId);
        bool isBlockOpaque(const int x, const int y, const int z) const;
        void sendToGPUHelper(glm::uint* vao, glm::uint* vbo, const std::vector<Vertex>& meshData);
        void freeGPUHelper(glm::uint* vao, glm::uint* vbo);
    };
}
