#ifndef MINECRAFTALTER_SHADER_H
#define MINECRAFTALTER_SHADER_H
#include "MinecraftAlter.h"
#include <fstream>
#include <string>

namespace MinecraftAlter
{
    class Shader {
    public:
        uint32 programId;
        Shader() = default;
        Shader(const char* vertexShaderPath, const char* fragmentShaderPath);
        void compile(const char* vertexShaderPath, const char* fragmentShaderPath);
        void use();
        const std::string readFile(const char* path);

        // Link Uniforms
        void setFloat(const char* uniformName, float val);
        void setVec3(const char* uniformName, const glm::vec3 &vec);
        void setVec4(const char* uniformName, const glm::vec4 &vec);
        void setMat4(const char* uniformName, const glm::mat4 &mat);
    };
}

#endif
