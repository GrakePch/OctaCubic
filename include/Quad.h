﻿#ifndef MINECRAFTALTER_QUAD_H
#define MINECRAFTALTER_QUAD_H
#include <glad/glad.h>

namespace MinecraftAlter
{
    enum face { xPos, xNeg, yPos, yNeg, zPos, zNeg };

    class Quad {
        unsigned int vao = 0;
        unsigned int vbo;
        float vertices[32] = {};

    public:
        Quad() = default;
        Quad(unsigned int& quadVAO, unsigned int& quadVBO, face face);
        void vertCopy(float* to, const float* from, int length);
        void scale(float scale);
        void renderQuad();
    };
}

#endif