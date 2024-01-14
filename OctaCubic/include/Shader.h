﻿#pragma once

#ifndef GLAD_INCLUDED
#define GLAD_INCLUDED
#include <glad/glad.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <string>

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

namespace OctaCubic
{
    class Shader {
    public:
        static Shader* activeShader;
        uint32 programId;
        Shader() = default;
        Shader(const char* vertexShaderPath, const char* fragmentShaderPath);
        void compile(const char* vertexShaderPath, const char* fragmentShaderPath);
        void use();
        const std::string readFile(const char* path);

        // Link Uniforms
        void setBool(const char* uniformName, bool val);
        void setInt(const char* uniformName, int val);
        void setFloat(const char* uniformName, float val);
        void setVec3(const char* uniformName, const glm::vec3 &vec);
        void setVec4(const char* uniformName, const glm::vec4 &vec);
        void setMat4(const char* uniformName, const glm::mat4 &mat);
    };
}