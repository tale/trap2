#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <stdio.h>
#include <stdlib.h>

GLuint load_shaders(const char *vert, const char *frag);
void attach_shaders(GLuint program, const char *vert, const char *frag);
