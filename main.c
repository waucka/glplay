#include <epoxy/gl.h>
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
#include <assert.h>

#define MAX_SHADER_SIZE (16 * 1024)

void sdl_bailout(const char* msg) {
  printf("[ERROR] %s: %s\n", SDL_GetError(), msg);
  SDL_Quit();
  exit(1);
}

#ifndef NDEBUG
void check_sdl_error_func(const char* file, int line) {
  const char* error = SDL_GetError();
  //Seriously, SDL?
  if(*error != '\0') {
    printf("[%s:%d] SDL Error: %s\n", file, line, error);
    SDL_ClearError();
  }
}
#define CHECK_SDL_ERROR check_sdl_error_func(__FILE__, __LINE__)
#else
#define CHECK_SDL_ERROR
#endif

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
    sdl_bailout("Failed to load shader");
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
    sdl_bailout("Failed to compile shader");
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

void set_projection_matrix(GLfloat* mat,
			   GLfloat width, GLfloat height, GLfloat fov,
			   GLfloat near_clip, GLfloat far_clip) {
  GLfloat aspect_ratio = width / height;
  GLfloat y_scale = 1.0f / tanf(fov / 2.0f);
  GLfloat x_scale = y_scale / aspect_ratio;
  GLfloat frustum_length = far_clip - near_clip;
  GLfloat m33 = -(far_clip + near_clip) / frustum_length;
  GLfloat m34 = -(2.0f * near_clip * far_clip) / frustum_length;

  memset(mat, 0, sizeof(GLfloat) * 16);
  mat[0] = x_scale;
  mat[5] = y_scale;
  mat[10] = m33;
  mat[11] = m34;
  mat[14] = -1;
}

/*
 * Sets mat1 to mat1 * mat2
 */
void mat_mul4(GLfloat* mat1, GLfloat* mat2) {
  GLfloat rowcopy[4];
  size_t rowsize = sizeof(GLfloat) * 4;
  for(int i = 0; i < 4; i++) {
    GLfloat* current_row = mat1 + i * 4;
    //assert(mat2[0] == 4);
    //assert(mat1[4] == 1 || mat1[4] == 40);
    memcpy(rowcopy, current_row, rowsize);
    //assert(rowcopy[0] == 1);
    //assert(mat2[0] == 4);
    //assert(mat1[4] == 1 || mat1[4] == 40);
    for(int j = 0; j < 4; j++) {
      current_row[j] = 0;
      //assert(mat2[0] == 4);
      for(int k = 0; k < 4; k++) {
	current_row[j] += rowcopy[k] * mat2[k * 4 + j];
	//assert(mat2[0] == 4);
      }
    }
    //assert(mat1[4] == 1 || mat1[4] == 40);
  }
}

int float_eq(GLfloat f1, GLfloat f2) {
  return fabs(f1 - f2) < 0.00001f;
}

int main(int argc, char* argv[]) {
  GLfloat mat1[] = {
    1.0f, 2.0f, 3.0f, 4.0f,
    1.0f, 2.0f, 3.0f, 4.0f,
    1.0f, 2.0f, 3.0f, 4.0f,
    1.0f, 2.0f, 3.0f, 4.0f
  };

  /*GLfloat mat2[] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };*/

  GLfloat mat2[] = {
    4.0f, 3.0f, 2.0f, 1.0f,
    4.0f, 3.0f, 2.0f, 1.0f,
    4.0f, 3.0f, 2.0f, 1.0f,
    4.0f, 3.0f, 2.0f, 1.0f
  };

  mat_mul4(mat1, mat2);
  assert(float_eq(mat1[0], 40.0f));
  assert(float_eq(mat1[1], 30.0f));
  assert(float_eq(mat1[2], 20.0f));
  assert(float_eq(mat1[3], 10.0f));
  assert(float_eq(mat1[4], 40.0f));
  assert(float_eq(mat1[5], 30.0f));
  assert(float_eq(mat1[6], 20.0f));
  assert(float_eq(mat1[7], 10.0f));
  assert(float_eq(mat1[8], 40.0f));
  assert(float_eq(mat1[9], 30.0f));
  assert(float_eq(mat1[10], 20.0f));
  assert(float_eq(mat1[11], 10.0f));
  assert(float_eq(mat1[12], 40.0f));
  assert(float_eq(mat1[13], 30.0f));
  assert(float_eq(mat1[14], 20.0f));
  assert(float_eq(mat1[15], 10.0f));

  SDL_Window* main_window;
  SDL_GLContext main_context;

  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    sdl_bailout("Unable to initialize SDL video/events");
  }

  if(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) < 0) {
    sdl_bailout("Unable to initialize SDL_image");
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  main_window = SDL_CreateWindow("glplay",
				 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
				 640, 480,
				 SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  main_context = SDL_GL_CreateContext(main_window);
  CHECK_SDL_ERROR;

  printf("[INFO] libepoxy says GL version is %d\n", epoxy_gl_version());

  GLint gl_major_version;
  GLint gl_minor_version;
  glGetIntegerv(GL_MAJOR_VERSION, &gl_major_version);
  glGetIntegerv(GL_MINOR_VERSION, &gl_minor_version);
  printf("[INFO] Using GL %d.%d\n", gl_major_version, gl_minor_version);
  printf("[INFO] GL version string: %s\n", glGetString(GL_VERSION));

  GLint max_vertex_attrs;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attrs);
  printf("[INFO] Max vertex attributes: %d\n", max_vertex_attrs);

  SDL_GL_SetSwapInterval(1);
  glClearColor(0.1, 0.2, 0.2, 1.0);
  glViewport(0, 0, 640, 480);

  GLuint vertex_shader = load_shader("vert.glsl", GL_VERTEX_SHADER);
  GLuint fragment_shader = load_shader("frag.glsl", GL_FRAGMENT_SHADER);

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  {
    GLint success;
    GLchar info_log[4096];
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success) {
      glGetProgramInfoLog(shader_program, 4096, NULL, info_log);
      printf("[ERROR] Failed to link shader program:\n%s\n", info_log);
      sdl_bailout("Failed to link shader program");
    }
  }
  glDeleteShader(vertex_shader);
  vertex_shader = 0;
  glDeleteShader(fragment_shader);
  fragment_shader = 0;

  glUseProgram(shader_program);

  GLfloat vertices[] = {
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f,   1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0f,   0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 1.0f
  };

  GLuint indices[] = {
    0, 1, 3,
    1, 2, 3
  };

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  GLuint tex = upload_texture("me.jpg");

  GLuint vbo;
  glGenBuffers(1, &vbo);

  GLuint ebo;
  glGenBuffers(1, &ebo);

  GLuint vao;
  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(7 * sizeof(GLfloat)));
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glBindVertexArray(0);


  GLfloat projection[16];

  GLfloat view[] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };

  GLfloat model_base[] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, -2,
    0, 0, 0, 1
    };

  GLfloat model_rotation[] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };

  GLfloat model[16];

  int done = 0;
  SDL_Event ev;
  while(!done) {
    if(SDL_PollEvent(&ev)) {
      switch(ev.type) {
      case SDL_KEYUP:
	if(ev.key.keysym.sym == SDLK_q) {
	  done = 1;
	}
	break;
      default:
	break;
      }
    }
    GLint time_uniform_location = glGetUniformLocation(shader_program, "time");
    GLint sampler_uniform_location = glGetUniformLocation(shader_program, "tex");
    GLint projection_uniform_location = glGetUniformLocation(shader_program, "projection");
    GLint view_uniform_location = glGetUniformLocation(shader_program, "view");
    GLint model_uniform_location = glGetUniformLocation(shader_program, "model");

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);

    glUniform1ui(time_uniform_location, SDL_GetTicks());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(sampler_uniform_location, 0);

    set_projection_matrix(projection,
			  640.0f, 480.0f, M_PI_2,
			  1.0f, 10.0f + 5.0f * (sin(SDL_GetTicks() / 500.0f) + 1));
    GLfloat angle_deg = SDL_GetTicks() / 20.0f;
    GLfloat angle_rad = angle_deg / 180.0f * M_PI;
    memcpy(model, model_base, sizeof(model_base));
    model_rotation[5] = cos(-angle_rad);
    model_rotation[6] = -sin(-angle_rad);
    model_rotation[9] = sin(-angle_rad);
    model_rotation[10] = cos(-angle_rad);
    mat_mul4(model, model_rotation);
    //model[11] = -2.0f - (2.0f * (sin(SDL_GetTicks() / 500.0f) + 1));
    glUniformMatrix4fv(projection_uniform_location,
		       1,
		       GL_TRUE,
		       projection);
    glUniformMatrix4fv(view_uniform_location,
		       1,
		       GL_TRUE,
		       view);
    glUniformMatrix4fv(model_uniform_location,
		       1,
		       GL_TRUE,
		       model);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    SDL_GL_SwapWindow(main_window);
  }

  SDL_GL_DeleteContext(main_context);
  SDL_DestroyWindow(main_window);
  IMG_Quit();
  SDL_Quit();
  return 0;
}
