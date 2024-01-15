#pragma once

/**
 * \brief Write vertices with attributes of a quad of the unit cube into vertex buffer. \n
 * [ local_x, y, z, normal_x, y, z, tex_u, v ] --> \n
 * [ global_x, y, z, normal_x, y, z, tex_u, v, blockId, block_x, y, z ]
 */
inline void overwriteVertexBuff(int numOfAttributesPerVertex, float* verticesBuffer, unsigned int* indicesBuffer,
                                int* currQuadIdx, const float* verticesOfUnitQuad, int blockId, int x, int y, int z,
                                unsigned int& worldVertCount) {
    /// 4 vertices per Quad
    const int startIdxOfVertex = *currQuadIdx * 4;
    for (int i = 0; i < 4; ++i) {
        const int startPosOfCurrVertex = (startIdxOfVertex + i) * numOfAttributesPerVertex;

        /// Position
        verticesBuffer[startPosOfCurrVertex + 0] = verticesOfUnitQuad[i * 8 + 0] + (float)x;
        verticesBuffer[startPosOfCurrVertex + 1] = verticesOfUnitQuad[i * 8 + 1] + (float)y;
        verticesBuffer[startPosOfCurrVertex + 2] = verticesOfUnitQuad[i * 8 + 2] + (float)z;

        /// Normal
        verticesBuffer[startPosOfCurrVertex + 3] = verticesOfUnitQuad[i * 8 + 3];
        verticesBuffer[startPosOfCurrVertex + 4] = verticesOfUnitQuad[i * 8 + 4];
        verticesBuffer[startPosOfCurrVertex + 5] = verticesOfUnitQuad[i * 8 + 5];

        /// Texture Coordinates
        verticesBuffer[startPosOfCurrVertex + 6] = verticesOfUnitQuad[i * 8 + 6];
        verticesBuffer[startPosOfCurrVertex + 7] = verticesOfUnitQuad[i * 8 + 7];

        /// Block Id
        verticesBuffer[startPosOfCurrVertex + 8] = (float)blockId;
        
        /// Block Coordinates
        verticesBuffer[startPosOfCurrVertex + 9] = (float)x;
        verticesBuffer[startPosOfCurrVertex + 10] = (float)y;
        verticesBuffer[startPosOfCurrVertex + 11] = (float)z;

        worldVertCount++;
    }

    /// 2 triangles per Quad
    const int startIdx = *currQuadIdx * 6;
    indicesBuffer[startIdx + 0] = startIdxOfVertex + 0;
    indicesBuffer[startIdx + 1] = startIdxOfVertex + 1;
    indicesBuffer[startIdx + 2] = startIdxOfVertex + 2;
    indicesBuffer[startIdx + 3] = startIdxOfVertex + 2;
    indicesBuffer[startIdx + 4] = startIdxOfVertex + 1;
    indicesBuffer[startIdx + 5] = startIdxOfVertex + 3;

    ++*currQuadIdx;
}

inline void clearVerticesBuffer(float* verticesBuffer, unsigned int* indicesBuffer, int lenOfVerticesBuffer,
                                int lenOfIndicesBuffer) {
    for (int i = 0; i < lenOfVerticesBuffer; ++i) verticesBuffer[i] = 0;
    for (int i = 0; i < lenOfIndicesBuffer; ++i) indicesBuffer[i] = 0;
}
