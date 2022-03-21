#ifndef GL_TEXTURES
#define GL_TEXTURES

#include "common.h"

GLuint load_texture(string path);
GLuint load_array(vector<string>& paths, int size);

#endif
