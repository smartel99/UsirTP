#pragma once

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "GL/glew.h"

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))

#define RENDER_COLOR_BLACK 0.0f, 0.0f, 0.0f, 1.0f

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);
const char* GetGLErrorName(GLenum errorno);