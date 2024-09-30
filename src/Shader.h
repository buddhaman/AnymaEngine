#pragma once 

#include "AnymUtil.h"

inline const char* default_vertex_shader_source = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 a_position;
    layout (location = 1) in vec2 a_texture;
    layout (location = 2) in vec4 a_color;

    uniform mat3 u_transform;

    out vec4 v_color;

    void main() 
    {
        vec3 pos = u_transform * vec3(a_position, 1.0);
        v_color = a_color;
        gl_Position = vec4(pos.xy, 0.0, 1.0);
    }
)glsl";

inline const char* default_fragment_shader_source = R"glsl(
    #version 330 core

    in vec4 v_color;

    out vec4 frag_color;
    
    void main() 
    {
        vec3 gammaCorrectedColor = pow(v_color.abg, vec3(1.0/2.2));
        frag_color = vec4(gammaCorrectedColor, v_color.r);
    }
)glsl";

struct Shader
{
    U32 program_handle;
    U32 transform_loc;
};

Shader 
CreateShader(const char* vertex_shader_src = default_vertex_shader_source, const char* fragment_shader_src = default_fragment_shader_source);

void
SetTransform(Shader* shader, R32* transform_matrix);

void
UseShader(Shader* shader);

