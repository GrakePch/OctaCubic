#include "Shader.h"

namespace MinecraftAlter
{
    const std::string readFile(const char* path) {
        std::string line;
        std::string code = "";
        std::ifstream file;
        file.open(path);
        std::cout << "===start print code===" << std::endl;

        while (std::getline(file, line)) {
            code += line + "\n";
        }
        file.close();
        return code;
    }

    void Shader::compile() {
        // Adapted from https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/2.2.hello_triangle_indexed/hello_triangle_indexed.cpp
        // Vertex Shader
        const std::string& vshStr = readFile("src/shaders/vsh.glsl");
        const char* vsh = vshStr.c_str();
        std::cout << vsh << std::endl;
        uint32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vsh, nullptr);
        glCompileShader(vertexShader);
        // check for shader compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n '%s'\n", infoLog);
            glDeleteShader(vertexShader);
        }

        // Fragment Shader
        const std::string& fshStr = readFile("src/shaders/fsh.glsl");
        const char* fsh = fshStr.c_str();
        uint32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fsh, nullptr);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n '%s'\n", infoLog);
            glDeleteShader(fragmentShader);
        }

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

    void Shader::bind() {
        glUseProgram(programId);
    }
}
