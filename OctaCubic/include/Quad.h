﻿#pragma once

#include <glm/glm.hpp>

namespace OctaCubic
{
    enum face { xPos, xNeg, yPos, yNeg, zPos, zNeg };

    class Quad {
        unsigned int vao = 0;
        unsigned int vbo;
        float vertices[32] = {};

    public:
        static unsigned int vertRenderCount;
        Quad() = default;
        Quad(unsigned int& quadVAO, unsigned int& quadVBO, face face);
        void vertCopy(float* to, const float* from, int length);
        void scale(float scale);
        void removeNegPosition();
        void renderQuad();
        float* getVertices();
        float* getVerticesWithTrans(const glm::mat4& transform) const;
    };
}
