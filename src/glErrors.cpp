#include "glErrors.h"
#include <iostream>

void GLClearError()
{
    while ( glGetError() != GL_NO_ERROR )
    {
    }
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while ( GLenum error = glGetError() )
    {
        std::cout << "[OpenGL Error]: \n\t" <<
            "(" << error << ") " << GetGLErrorName(error) << "\n\t" <<
            "Function: " << function <<
            "\n\tLine: " << line <<
            "\n\tFile: " << file << std::endl;
        return false;
    }
    return true;
}

const char* GetGLErrorName(GLenum errorno)
{
    switch ( errorno )
    {
        case GL_INVALID_ENUM:
            return "Invalid Enum";
        case GL_INVALID_VALUE:
            return "Invalid Value";
        case GL_INVALID_OPERATION:
            return "Invalid Operation";
        case GL_STACK_OVERFLOW:
            return "Stack Overflow";
        case GL_STACK_UNDERFLOW:
            return "Stack Underflow";
        case GL_OUT_OF_MEMORY:
            return "Out of Memory";
        default:
            return "Unknown";
    }
}
