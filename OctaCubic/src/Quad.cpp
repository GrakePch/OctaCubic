#include "Quad.h"

#include <glad/glad.h>

namespace OctaCubic
{
    float quadXPos[] = {
        // positions         // normals         // texture Coords
        +1.0f, -1.0f, +1.0f, +1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        +1.0f, +1.0f, +1.0f, +1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        +1.0f, -1.0f, -1.0f, +1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        +1.0f, +1.0f, -1.0f, +1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };
    float quadXNeg[] = {
        // positions         // normals         // texture Coords
        -1.0f, -1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, +1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, +1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    };
    float quadYPos[] = {
        // positions         // normals         // texture Coords
        -1.0f, +1.0f, +1.0f, 0.0f, +1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, +1.0f, -1.0f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, 0.0f, +1.0f, 0.0f, 1.0f, 1.0f,
        +1.0f, +1.0f, -1.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f,
    };
    float quadYNeg[] = {
        // positions         // normals         // texture Coords
        -1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        +1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        +1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    float quadZPos[] = {
        // positions         // normals         // texture Coords
        -1.0f, +1.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, 0.0f, 0.0f, +1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f,
        +1.0f, -1.0f, +1.0f, 0.0f, 0.0f, +1.0f, 1.0f, 1.0f,
    };
    float quadZNeg[] = {
        // positions         // normals         // texture Coords
        -1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        +1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        +1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
    };

    unsigned int Quad::vertRenderCount = 0;

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
        for (int i = 0; i < 4; ++i)
            for (int idx = 0; idx < 3; ++idx)
                vertices[i * 8 + idx] *= scale;
    }

    void Quad::removeNegPosition() {
        for (int i = 0; i < 4; ++i)
            for (int idx = 0; idx < 3; ++idx)
                if (vertices[i * 8 + idx] < 0) vertices[i * 8 + idx] = 0;
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
        vertRenderCount += 4;
        glBindVertexArray(0);
    }

    float* Quad::getVertices() {
        return vertices;
    }

    float* Quad::getVerticesWithTrans(glm::mat4 transform) const {
        static float result[32] = {0};
        for (int v = 0; v < 4; ++v) {
            glm::vec4 vertex(vertices[v * 8 + 0], vertices[v * 8 + 1], vertices[v * 8 + 2], 1);
            vertex = transform * vertex;
            result[v * 8 + 0] = vertex.x;
            result[v * 8 + 1] = vertex.y;
            result[v * 8 + 2] = vertex.z;
            for (int i = 3; i < 8; ++i) result[v * 8 + i] = vertices[v * 8 + i];
        }
        return result;
    }
}
