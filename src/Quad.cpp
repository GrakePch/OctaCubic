﻿#include "Quad.h"

namespace MinecraftAlter
{
    float quadXPos[] = {
        // positions         // normals         // texture Coords
        +1.0f, -1.0f, +1.0f, +1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        +1.0f, +1.0f, +1.0f, +1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        +1.0f, -1.0f, -1.0f, +1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        +1.0f, +1.0f, -1.0f, +1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };
    float quadXNeg[] = {
        // positions         // normals         // texture Coords
        -1.0f, -1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, +1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, +1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };
    float quadYPos[] = {
        // positions         // normals         // texture Coords
        -1.0f, +1.0f, +1.0f, 0.0f, +1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, +1.0f, -1.0f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, 0.0f, +1.0f, 0.0f, 1.0f, 1.0f,
        +1.0f, +1.0f, -1.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f,
    };
    float quadYNeg[] = {
        // positions        // normals          // texture Coords
        -1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        +1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        +1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    float quadZPos[] = {
        // positions         // normals         // texture Coords
        -1.0f, +1.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f,
        +1.0f, +1.0f, +1.0f, 0.0f, 0.0f, +1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f,
        +1.0f, -1.0f, +1.0f, 0.0f, 0.0f, +1.0f, 1.0f, 0.0f,
    };
    float quadZNeg[] = {
        // positions         // normals         // texture Coords
        -1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        +1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        +1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
    };

    Quad::Quad(unsigned int& quadVAO, unsigned int& quadVBO, face face): vao(quadVAO), vbo(quadVBO) {
        switch (face) {
        case xPos: vertCopy(vertices, quadXPos, 32);
            break;
        case xNeg: vertCopy(vertices, quadXNeg, 32);
            break;
        case yPos: vertCopy(vertices, quadYPos, 32);
            break;
        case yNeg: vertCopy(vertices, quadYNeg, 32);
            break;
        case zPos: vertCopy(vertices, quadZPos, 32);
            break;
        case zNeg: vertCopy(vertices, quadZNeg, 32);
            break;
        }
    }

    void Quad::vertCopy(float* to, const float* from, int length) {
        for (int i = 0; i < length; ++i) {
            to[i] = from[i];
        }
    }

    void Quad::scale(float scale) {
        for (int i = 0; i < 4; ++i) {
            vertices[i * 8 + 0] *= scale;
            vertices[i * 8 + 1] *= scale;
            vertices[i * 8 + 2] *= scale;
        }
    }

    void Quad::renderQuad() {
        if (vao == 0) {
            // setup plane's Vertex Array Object
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            // Copy the vertices array in a vertex buffer for OpenGL to use
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
            // Set the vertex attributes pointers
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        }
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
}