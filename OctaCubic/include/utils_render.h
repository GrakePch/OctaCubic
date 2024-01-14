#pragma once

inline void overwriteVertexBuff(int numOfAttributesPerVertex, float* verticesBuffer, unsigned int* indicesBuffer,
                         int* currQuadIdx, const float* arr, int blockId, int x, int y, int z, unsigned int & worldVertCount) {
    
    for (int i = 0; i < 4; ++i) {
        /// vec3 vertex's Position
        verticesBuffer[*currQuadIdx * numOfAttributesPerVertex * 4 + i * numOfAttributesPerVertex + 0] = arr[i * 8 + 0] + (float)x;
        verticesBuffer[*currQuadIdx * numOfAttributesPerVertex * 4 + i * numOfAttributesPerVertex + 1] = arr[i * 8 + 1] + (float)y;
        verticesBuffer[*currQuadIdx * numOfAttributesPerVertex * 4 + i * numOfAttributesPerVertex + 2] = arr[i * 8 + 2] + (float)z;
        // printf("coords: %f %f %f\n",
        // verticesBuff[*currQuadIdx * 36 + i * 9 + 0],
        // verticesBuff[*currQuadIdx * 36 + i * 9 + 1],
        // verticesBuff[*currQuadIdx * 36 + i * 9 + 2]);
        
        /// vec3 vertex's Normal & vec2 vertex's Texture Coordinates
        for (int val = 3; val < 8; ++val) {
            verticesBuffer[*currQuadIdx * numOfAttributesPerVertex * 4 + i * numOfAttributesPerVertex + val] = arr[i * 8 + val];
        }
        /// float vertex's Block Id
        verticesBuffer[*currQuadIdx * numOfAttributesPerVertex * 4 + i * numOfAttributesPerVertex + 8] = (float)blockId;
        /// vec3 vertex's Block Coordinates
        verticesBuffer[*currQuadIdx * numOfAttributesPerVertex * 4 + i * numOfAttributesPerVertex + 9] = (float)x;
        verticesBuffer[*currQuadIdx * numOfAttributesPerVertex * 4 + i * numOfAttributesPerVertex + 10] = (float)y;
        verticesBuffer[*currQuadIdx * numOfAttributesPerVertex * 4 + i * numOfAttributesPerVertex + 11] = (float)z;
        //printf("id: %d\n", blockId);
        worldVertCount++;
    }
    const int startIdx = *currQuadIdx * 6;
    const int startIdxOfVertex = *currQuadIdx * 4;
    indicesBuffer[startIdx + 0] = startIdxOfVertex + 0;
    indicesBuffer[startIdx + 1] = startIdxOfVertex + 1;
    indicesBuffer[startIdx + 2] = startIdxOfVertex + 2;
    indicesBuffer[startIdx + 3] = startIdxOfVertex + 2;
    indicesBuffer[startIdx + 4] = startIdxOfVertex + 1;
    indicesBuffer[startIdx + 5] = startIdxOfVertex + 3;
    //printf("%d %d %d %d %d %d\n", indicesBuff[startIdx + 0], indicesBuff[startIdx + 1], indicesBuff[startIdx + 2],
    // indicesBuff[startIdx + 3], indicesBuff[startIdx + 4], indicesBuff[startIdx + 5]);
    ++*currQuadIdx;
}

inline void clearVerticesBuffer(float* verticesBuffer, unsigned int* indicesBuffer, int lenOfVerticesBuffer, int lenOfIndicesBuffer) {
    for (int i = 0; i < lenOfVerticesBuffer; ++i) verticesBuffer[i] = 0;
    for (int i = 0; i < lenOfIndicesBuffer; ++i) indicesBuffer[i] = 0;
}