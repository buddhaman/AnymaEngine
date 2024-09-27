#include "Shader.h"

static inline U32
CreateAndCompileShaderSource(const char *source, GLenum shaderType)
{
    const char *const* src = (const char *const *)&source;
    U32 shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, src, NULL);
    glCompileShader(shader);
    I32 succes;
    char infolog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &succes);
    if(!succes)
    {
        glGetShaderInfoLog(shader, 512, NULL, infolog);
        printf("%s\n", infolog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static inline U32 
CreateAndLinkShaderProgram(U32 vertexShader, U32 fragmentShader)
{
    U32 shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    I32 succes;
    char infolog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &succes);
    if(!succes)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infolog);
        printf("%s", infolog);
        glDetachShader(shaderProgram, vertexShader);
        glDetachShader(shaderProgram, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);
    }
    else
    {
        printf("Shader program %u linked succesfully. Cleaning up.", shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    return shaderProgram;
}

Shader
CreateShader(const char* vertex_shader_src, const char* fragment_shader_src)
{
    Shader shader = {};

    U32 vertex_shader = CreateAndCompileShaderSource(vertex_shader_src, GL_VERTEX_SHADER);
    U32 fragment_shader = CreateAndCompileShaderSource(fragment_shader_src, GL_FRAGMENT_SHADER);
    shader.program_handle = CreateAndLinkShaderProgram(vertex_shader, fragment_shader);

    return shader;
}
