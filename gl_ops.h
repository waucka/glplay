#ifndef GL_OPS_H
#define GL_OPS_H

#include <epoxy/gl.h>

#define MAX_SHADER_SIZE (16 * 1024)

char* slurp_file(const char* filename);
int load_shader(const char* filename, GLint shader_type);
GLuint upload_texture(const char* filename);

typedef struct {
  GLfloat x;
  GLfloat y;
  GLfloat z;

  GLfloat s;
  GLfloat t;
} vertex_data_t;

#endif
