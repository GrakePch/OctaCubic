#ifndef MINECRAFTALTER_SHADER_H
#define MINECRAFTALTER_SHADER_H
#include "MinecraftAlter.h"
#include <fstream>
#include <string>

namespace MinecraftAlter
{
    struct Shader {
        uint32 programId;

        void bind();
        void compile();
    };
}

#endif
