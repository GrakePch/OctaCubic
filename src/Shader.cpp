#include "Shader.h"

#include <iostream>

using namespace MinecraftAlter;

Shader* Shader::activeShader = nullptr;

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath) {
    compile(vertexShaderPath, fragmentShaderPath);
    programId = 0;
}

const std::string Shader::readFile(const char* path) {
    std::string line;
    std::string code;
    std::ifstream file;
    file.open(path);
    std::cout << "Loading: " << path;

    while (std::getline(file, line)) {
        code += line + "\n";
    }
    file.close();
    return code;
}

void Shader::compile(const char* vertexShaderPath, const char* fragmentShaderPath) {
    // Adapted from https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/2.2.hello_triangle_indexed/hello_triangle_indexed.cpp
    // Vertex Shader
    const std::string& vshStr = readFile(vertexShaderPath);
    const char* vsh = vshStr.c_str();
    uint32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vsh, nullptr);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        printf("\nERROR::SHADER::VERTEX::COMPILATION_FAILED\n '%s'\n", infoLog);
        glDeleteShader(vertexShader);
    }
    printf("  Success!\n");

    // Fragment Shader
    const std::string& fshStr = readFile(fragmentShaderPath);
    const char* fsh = fshStr.c_str();
    uint32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fsh, nullptr);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        printf("\nERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n '%s'\n", infoLog);
        glDeleteShader(fragmentShader);
    }
    printf("  Success!\n");

    // link shaders
    programId = glCreateProgram();
    glAttachShader(programId, vertexShader);
    glAttachShader(programId, fragmentShader);
    glLinkProgram(programId);
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programId, 512, nullptr, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n '%s'\n", infoLog);
        glDeleteProgram(programId);
    }

    // After successful link, detach shaders and destroy when not needed.
    glDetachShader(programId, vertexShader);
    glDetachShader(programId, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::use() {
    glUseProgram(programId);
    activeShader = this;
}

// Link Uniforms
void Shader::setBool(const char* uniformName, bool val) {
    setInt(uniformName, val);
}

void Shader::setInt(const char* uniformName, int val) {
    glUniform1i(glGetUniformLocation(programId, uniformName), val);
}

void Shader::setFloat(const char* uniformName, float val) {
    glUniform1f(glGetUniformLocation(programId, uniformName), val);
}

void Shader::setVec3(const char* uniformName, const glm::vec3 &vec) {
    glUniform3fv(glGetUniformLocation(programId, uniformName), 1, glm::value_ptr(vec));
}

void Shader::setVec4(const char* uniformName, const glm::vec4 &vec) {
    glUniform4fv(glGetUniformLocation(programId, uniformName), 1, glm::value_ptr(vec));
}

void Shader::setMat4(const char* uniformName, const glm::mat4 &mat) {
    glUniformMatrix4fv(glGetUniformLocation(programId, uniformName), 1, GL_FALSE, glm::value_ptr(mat));
}
