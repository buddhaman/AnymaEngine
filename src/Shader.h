#pragma once 

#include "AnymUtil.h"

struct Shader
{
    U32 program_handle;
};

Shader 
CreateShader(const char* vertex_shader_src, const char* fragment_shader_src);

