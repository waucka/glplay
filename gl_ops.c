#include "gl_ops.h"

#include <SDL.h>
#include <SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <math.h>

char* slurp_file(const char* filename) {
  int fd = open(filename, O_RDONLY);
  if(fd < 0) {
    printf("[ERROR] Unable to open file %s: %s\n", filename, strerror(errno));
    return NULL;
  }

  struct stat file_stat;
  fstat(fd, &file_stat);
  if(file_stat.st_size > MAX_SHADER_SIZE) {
    printf("[ERROR] Shader file %s too big: %ld bytes\n", filename, file_stat.st_size);
    return NULL;
  }

  char* file_bytes = malloc(file_stat.st_size + 1);
  memset(file_bytes, 0, file_stat.st_size + 1);
  char* current = file_bytes;
  while(current - file_bytes < file_stat.st_size) {
    ssize_t bytes_read = read(fd, current, file_stat.st_size - (current - file_bytes));
    if(bytes_read < 0) {
      printf("[ERROR] Failed reading shader file %s: %s\n", filename, strerror(errno));
      free(file_bytes);
      return NULL;
    }
    if(bytes_read == 0) {
      printf("[WARNING] Unexpected EOF reading shader file %s: %ld bytes erad\n",
	     filename, current - file_bytes);
    }
    current += bytes_read;
  }
  close(fd);
  return file_bytes;
}

int load_shader(const char* filename, GLint shader_type) {
  char* shader_source = slurp_file(filename);
  if(shader_source == NULL) {
    return -1;
  }
  printf("Read %s shader source:\n########\n%s\n########\n",
	 shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment",
	 shader_source);

  GLuint shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, (const char**)&shader_source, NULL);
  glCompileShader(shader);
  GLint success;
  GLchar info_log[4096];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(shader, 4096, NULL, info_log);
    printf("[ERROR] Failed to compile shader:\n%s\n", info_log);
    return -1;
  }
  free(shader_source);
  return shader;
}

GLuint upload_texture(const char* filename) {
  SDL_Surface* img_surface = IMG_Load(filename);
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  GLint texmode = GL_RGB;
  if(img_surface->format->BytesPerPixel == 4) {
    texmode = GL_RGBA;
    printf("[INFO] RGBA texture\n");
  } else {
    printf("[INFO] RGB texture\n");
  }
  SDL_LockSurface(img_surface);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
	       img_surface->w, img_surface->h,
	       0,
	       texmode, GL_UNSIGNED_BYTE, img_surface->pixels);
  SDL_UnlockSurface(img_surface);
  SDL_FreeSurface(img_surface);
  img_surface = NULL;

  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);
  return tex;
}
