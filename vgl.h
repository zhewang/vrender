#ifndef __VGL_H__
#define __VGL_H__

#ifdef USE_GL3W

#include <GL3/gl3.h>
#include <GL3/gl3w.h>

#else

#define GLEW_STATIC

#include <GL/glew.h>

#endif

#define FREEGLUT_STATIC
#define FREEGLUT_LIB_PRAGMAS 0
#include <GL/freeglut.h>

#define BUFFER_OFFSET(x)  ((const void*) (x))

#endif // __VGL_H__
