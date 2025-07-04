#include "Chunk.h"

#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>

#include "perlin.h"
#include "Quad.h"

using namespace OctaCubic;

std::unordered_set<chunk_coord, ChunkCoordHash> Chunk::chunkInGPUSet;

Chunk::Chunk(): chunkCoord_({0, 0, 0}) {}

Chunk::Chunk(const int cX, const int cZ): chunkCoord_({cX, 0, cZ}) {
    for (int x = 0; x < width; ++x)
        for (int z = 0; z < width; ++z)
            for (int y = 0; y < height; ++y)
                blocks_[x][y][z] = (y < 23) ? 2 : 0;
}

Chunk::~Chunk() {
    freeGPU();
}

void Chunk::buildMesh() {
    printf("Chunk %d %d: Building Mesh\n", chunkCoord_.x, chunkCoord_.z);
    genMeshData();
    isDirty = false;
    chunkInGPUSet.erase(chunkCoord_);
}

void Chunk::sendToGPU() {
    printf("Chunk %d %d: Sending to GPU\n", chunkCoord_.x, chunkCoord_.z);
    sendToGPUHelper(&vaoOpaque_, &vboOpaque_, meshDataOpaque_);
    sendToGPUHelper(&vaoWater_, &vboWater_, meshDataWater_);
    chunkInGPUSet.insert(chunkCoord_);
}

void Chunk::renderOpaque() const {
    if (vaoOpaque_ == 0 && meshDataOpaque_.empty()) return;
    glBindVertexArray(vaoOpaque_);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)meshDataOpaque_.size());
    glBindVertexArray(0);
}

void Chunk::renderWater() const {
    if (vaoWater_ == 0 && meshDataWater_.empty()) return;
    glBindVertexArray(vaoWater_);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)meshDataWater_.size());
    glBindVertexArray(0);
}

void Chunk::freeGPU() {
    freeGPUHelper(&vaoOpaque_, &vboOpaque_);
    freeGPUHelper(&vaoWater_, &vboWater_);
    chunkInGPUSet.erase(chunkCoord_);
}

bool Chunk::isInGPU() const {
    return chunkInGPUSet.find(chunkCoord_) != chunkInGPUSet.end();
}

bool Chunk::isCoordValid(const glm::ivec3& c) {
    return c.x >= 0 && c.x < width && c.y >= 0 && c.y < height && c.z >= 0 && c.z < width;
}

int16_t Chunk::getBlockId(const glm::ivec3& c) const {
    if (!isCoordValid(c)) {
        printf("Error: Get invalid in-chunk coordinates (%d, %d, %d)\n", c.x, c.y, c.z);
        return -1;
    }
    return blocks_[c.x][c.y][c.z];
}

int16_t Chunk::setBlockId(const glm::ivec3& c, const uint8_t blockId) {
    if (!isCoordValid(c)) {
        printf("Error: Set invalid in-chunk coordinates (%d, %d, %d)\n", c.x, c.y, c.z);
        return -1;
    }
    if (blocks_[c.x][c.y][c.z] == blockId) {
        return -2; // No change
    }
    blocks_[c.x][c.y][c.z] = blockId;
    isDirty = true; // Mark chunk as dirty to rebuild mesh
    return blockId;
}

void Chunk::genTerrain(const int seed) {
    const float altitudeSeaSurfaceF = 23.0f;
    for (int x = 0; x < width; x++)
        for (int z = 0; z < width; z++) {
            const float wX = static_cast<float>(x + chunkCoord_.x * width); // World coordinate X
            const float wZ = static_cast<float>(z + chunkCoord_.z * width); // World coordinate Z
            float surfaceHeightF = 0
                + (perlin(seed, wX / 32, wZ / 32) * .5f + .5f) * 32
                + (perlin(seed, wX / 16, wZ / 16) * .5f) * 12
                + 10;
            // Higher mountains
            if (surfaceHeightF > altitudeSeaSurfaceF + 5) {
                surfaceHeightF = altitudeSeaSurfaceF + 5 + (surfaceHeightF - altitudeSeaSurfaceF - 5) * 2;
            }
            // Deeper water
            if (surfaceHeightF < altitudeSeaSurfaceF - 3) {
                surfaceHeightF = altitudeSeaSurfaceF - 3 - (altitudeSeaSurfaceF - 3 - surfaceHeightF) * 2;
            }
            for (int y = 0; y < height; y++) {
                // General Terrain
                const auto yF = static_cast<float>(y + chunkCoord_.y * height); // World coordinate Y
                const uint8_t bid = y == 0
                                        ? 1 // Bedrock @ y = 0
                                        : yF < surfaceHeightF - 4
                                        ? 2 // Stone
                                        : yF < surfaceHeightF - 1
                                        ? 3 // Dirt
                                        : yF < surfaceHeightF
                                        // For the solid surface: below sea+2 -> Sand; below sea+12 -> Grass; above -> Snow
                                        ? surfaceHeightF > altitudeSeaSurfaceF + 2
                                              ? surfaceHeightF > altitudeSeaSurfaceF + 12
                                                    ? 6
                                                    : 4
                                              : 5
                                        // Above the solid surface: below sea -> Water; above -> Air
                                        : yF > altitudeSeaSurfaceF
                                        ? 0
                                        : 10;
                setBlockId(glm::ivec3(x, y, z), bid);
            }
        }
}

size_t Chunk::getNumOfChunksInGPU() {
    return chunkInGPUSet.size();
}

size_t Chunk::getNumVertices() const {
    return numVertices_;
}

/* Private members */

void Chunk::genMeshData() {
    meshDataOpaque_.clear();
    meshDataWater_.clear();

    for (int x = 0; x < width; ++x)
        for (int z = 0; z < width; ++z)
            for (int y = 0; y < height; ++y) {
                const uint8_t blockId = blocks_[x][y][z];
                if (blockId == 0) continue; // Skip air blocks
                if (blockId == 10) {
                    // Water block
                    // TODO: If it is water surface, shrink height to pre-set value
                    if (x == width - 1 || getBlockId({x + 1, y, z}) != 10)
                        genQuadData(meshDataWater_, Quad::unit_x_pos, blockId, x, y, z); // X+ face exposed
                    if (y == height - 1 || getBlockId({x, y + 1, z}) != 10)
                        genQuadData(meshDataWater_, Quad::unit_y_pos, blockId, x, y, z); // Y+ face exposed
                    if (z == width - 1 || getBlockId({x, y, z + 1}) != 10)
                        genQuadData(meshDataWater_, Quad::unit_z_pos, blockId, x, y, z); // Z+ face exposed
                    if (x == 0 || getBlockId({x - 1, y, z}) != 10)
                        genQuadData(meshDataWater_, Quad::unit_x_neg, blockId, x, y, z); // X- face exposed
                    if (y == 0 || getBlockId({x, y - 1, z}) != 10)
                        genQuadData(meshDataWater_, Quad::unit_y_neg, blockId, x, y, z); // Y- face exposed
                    if (z == 0 || getBlockId({x, y, z - 1}) != 10)
                        genQuadData(meshDataWater_, Quad::unit_z_neg, blockId, x, y, z); // Z- face exposed
                }
                else {
                    // Opaque block
                    if (x != 0 && x != width - 1 &&
                        y != 0 && y != height - 1 &&
                        z != 0 && z != width - 1 &&
                        isBlockOpaque(x + 1, y, z) && isBlockOpaque(x - 1, y, z) &&
                        isBlockOpaque(x, y, z + 1) && isBlockOpaque(x, y, z - 1) &&
                        isBlockOpaque(x, y + 1, z) && isBlockOpaque(x, y - 1, z))
                        continue; // Skip fully covered blocks
                    if (x == width - 1 || !isBlockOpaque(x + 1, y, z))
                        genQuadData(meshDataOpaque_, Quad::unit_x_pos, blockId, x, y, z); // X+ face exposed
                    if (y == height - 1 || !isBlockOpaque(x, y + 1, z))
                        genQuadData(meshDataOpaque_, Quad::unit_y_pos, blockId, x, y, z); // Y+ face exposed
                    if (z == width - 1 || !isBlockOpaque(x, y, z + 1))
                        genQuadData(meshDataOpaque_, Quad::unit_z_pos, blockId, x, y, z); // Z+ face exposed
                    if (x == 0 || !isBlockOpaque(x - 1, y, z))
                        genQuadData(meshDataOpaque_, Quad::unit_x_neg, blockId, x, y, z); // X- face exposed
                    if (y == 0 || !isBlockOpaque(x, y - 1, z))
                        genQuadData(meshDataOpaque_, Quad::unit_y_neg, blockId, x, y, z); // Y- face exposed
                    if (z == 0 || !isBlockOpaque(x, y, z - 1))
                        genQuadData(meshDataOpaque_, Quad::unit_z_neg, blockId, x, y, z); // Z- face exposed}
                }
            }

    numVertices_ = meshDataOpaque_.size() + meshDataWater_.size();
}

void Chunk::genQuadData(std::vector<Vertex>& meshData, const float* vertices, const uint8_t blockId,
                        const int x, const int y, const int z) {
    static const size_t indices[6] = {0, 1, 2, 2, 1, 3}; // Indices for two triangles forming a quad
    for (const size_t idx : indices) {
        meshData.emplace_back(
            vertices[idx * 8 + 0] + (float)x + (float)chunkCoord_.x * width,
            vertices[idx * 8 + 1] + (float)y,
            vertices[idx * 8 + 2] + (float)z + (float)chunkCoord_.z * width,
            vertices[idx * 8 + 3],
            vertices[idx * 8 + 4],
            vertices[idx * 8 + 5],
            vertices[idx * 8 + 6],
            vertices[idx * 8 + 7],
            blockId
        );
    }
}

bool Chunk::isBlockOpaque(const int x, const int y, const int z) const {
    const uint8_t blockId = blocks_[x][y][z];
    return blockId != 0 && blockId != 10; // Not air or water
}

void Chunk::sendToGPUHelper(glm::uint* vao, glm::uint* vbo, const std::vector<Vertex>& meshData) {
    if (*vao == 0)
        glGenVertexArrays(1, vao);
    if (*vbo == 0)
        glGenBuffers(1, vbo);

    glBindVertexArray(*vao);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);

    glBufferData(GL_ARRAY_BUFFER, meshData.size() * sizeof(Vertex), meshData.data(), GL_STATIC_DRAW);

    // Set the vertex attributes pointers
    /// vec3 vertex's Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    /// vec3 vertex's Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));
    /// vec2 vertex's Texture Coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    /// float vertex's Block Id
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, id));

    glBindVertexArray(0);
}

void Chunk::freeGPUHelper(glm::uint* vao, glm::uint* vbo) {
    if (*vbo != 0) {
        glDeleteBuffers(1, vbo);
        *vbo = 0;
    }
    if (*vao != 0) {
        glDeleteVertexArrays(1, vao);
        *vao = 0;
    }
}
