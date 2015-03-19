#include "vector_ops.h"
#include "matrix_ops.h"
#include "gl_ops.h"

#include <epoxy/gl.h>
#include <SDL.h>
#include <SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define CLAMP(val, minval, maxval) val = val > maxval ? maxval : (val < minval ? minval : val)

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
				 1024, 768,
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
  glViewport(0, 0, 1024, 768);
  glEnable(GL_DEPTH_TEST);

  GLint vertex_shader = load_shader("vert.glsl", GL_VERTEX_SHADER);
  GLint fragment_shader = load_shader("frag.glsl", GL_FRAGMENT_SHADER);

  if(vertex_shader < 0) {
    sdl_bailout("Failed to load vertex shader");
  }
  if(fragment_shader < 0) {
    sdl_bailout("Failed to load fragment shader");
  }

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

  vertex_data_t vertices[] = {
    { 0.5f,  0.5f, -0.5f,   0.0f, 1.0f,    0.0f,  0.0f, -1.0f},
    { 0.5f, -0.5f, -0.5f,   0.0f, 0.0f,    0.0f,  0.0f, -1.0f},
    {-0.5f, -0.5f, -0.5f,   1.0f, 0.0f,    0.0f,  0.0f, -1.0f},
    {-0.5f,  0.5f, -0.5f,   1.0f, 1.0f,    0.0f,  0.0f, -1.0f},

    { 0.5f,  0.5f,  0.5f,   1.0f, 1.0f,    0.0f,  0.0f,  1.0f},
    { 0.5f, -0.5f,  0.5f,   1.0f, 0.0f,    0.0f,  0.0f,  1.0f},
    {-0.5f, -0.5f,  0.5f,   0.0f, 0.0f,    0.0f,  0.0f,  1.0f},
    {-0.5f,  0.5f,  0.5f,   0.0f, 1.0f,    0.0f,  0.0f,  1.0f},

    { 0.5f,  0.5f, -0.5f,   1.0f, 1.0f,    1.0f,  0.0f,  0.0f},
    { 0.5f, -0.5f, -0.5f,   1.0f, 0.0f,    1.0f,  0.0f,  0.0f},
    { 0.5f, -0.5f,  0.5f,   0.0f, 0.0f,    1.0f,  0.0f,  0.0f},
    { 0.5f,  0.5f,  0.5f,   0.0f, 1.0f,    1.0f,  0.0f,  0.0f},

    {-0.5f,  0.5f, -0.5f,   0.0f, 1.0f,   -1.0f,  0.0f,  0.0f},
    {-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   -1.0f,  0.0f,  0.0f},
    {-0.5f, -0.5f,  0.5f,   1.0f, 0.0f,   -1.0f,  0.0f,  0.0f},
    {-0.5f,  0.5f,  0.5f,   1.0f, 1.0f,   -1.0f,  0.0f,  0.0f},

    { 0.5f, -0.5f, -0.5f,   1.0f, 0.0f,    0.0f, -1.0f,  0.0f},
    {-0.5f, -0.5f, -0.5f,   0.0f, 0.0f,    0.0f, -1.0f,  0.0f},
    {-0.5f, -0.5f,  0.5f,   0.0f, 1.0f,    0.0f, -1.0f,  0.0f},
    { 0.5f, -0.5f,  0.5f,   1.0f, 1.0f,    0.0f, -1.0f,  0.0f},

    { 0.5f, 0.5f,  -0.5f,   0.0f, 0.0f,    0.0f,  1.0f,  0.0f},
    {-0.5f, 0.5f,  -0.5f,   1.0f, 0.0f,    0.0f,  1.0f,  0.0f},
    {-0.5f, 0.5f,   0.5f,   1.0f, 1.0f,    0.0f,  1.0f,  0.0f},
    { 0.5f, 0.5f,   0.5f,   0.0f, 1.0f,    0.0f,  1.0f,  0.0f}
  };

  GLuint indices[] = {
    0, 1, 3,
    1, 2, 3,

    4, 5, 7,
    5, 6, 7,

    8, 9, 10,
    8, 10, 11,

    12, 13, 14,
    12, 14, 15,

    16, 17, 18,
    16, 18, 19,

    20, 21, 22,
    20, 22, 23
  };

  vertex_data_t ground_vertices[] = {
    { 5.0f, -1.0f,  5.0f,     10.0f, 10.0f,    0.0f,  1.0f,  0.0f},
    { 5.0f, -1.0f, -5.0f,     10.0f,  0.0f,    0.0f,  1.0f,  0.0f},
    {-5.0f, -1.0f, -5.0f,      0.0f,  0.0f,    0.0f,  1.0f,  0.0f},
    {-5.0f, -1.0f,  5.0f,      0.0f, 10.0f,    0.0f,  1.0f,  0.0f}
  };

  GLuint ground_indices[] = {
    0, 1, 3,
    1, 2, 3
  };

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  GLuint tex = upload_texture("me.jpg");
  GLuint white_tex = upload_texture("pure_white.png");

  GLuint vbo;
  glGenBuffers(1, &vbo);

  GLuint ebo;
  glGenBuffers(1, &ebo);

  GLuint vao;
  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_data_t), (GLvoid*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_data_t), (GLvoid*)(3 *  sizeof(GLfloat)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_data_t), (GLvoid*)(5 *  sizeof(GLfloat)));
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glBindVertexArray(0);

  GLuint ground_tex = upload_texture("stone.png");

  GLuint ground_vbo;
  glGenBuffers(1, &ground_vbo);

  GLuint ground_ebo;
  glGenBuffers(1, &ground_ebo);

  GLuint ground_vao;
  glGenVertexArrays(1, &ground_vao);

  glBindVertexArray(ground_vao);
  glBindBuffer(GL_ARRAY_BUFFER, ground_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(ground_vertices), ground_vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_data_t), (GLvoid*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_data_t), (GLvoid*)(3 *  sizeof(GLfloat)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_data_t), (GLvoid*)(5 *  sizeof(GLfloat)));
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ground_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ground_indices), ground_indices, GL_STATIC_DRAW);
  glBindVertexArray(0);


  GLfloat camera_location[3] = {3.0f, 0.0f, 3.0f};
  GLfloat camera_target[3] = {0.0f, 0.0f, 0.0f};
  GLfloat camera_look[3];
  GLfloat camera_right[3];
  GLfloat camera_up[3];
  set_camera_vectors(camera_location,
		     camera_target,
		     camera_look,
		     camera_right,
		     camera_up);

  assert(float_eq(camera_location[0], 3.0f));
  assert(float_eq(camera_location[1], 0.0f));
  assert(float_eq(camera_location[2], 3.0f));

  assert(float_eq(camera_target[0], 0.0f));
  assert(float_eq(camera_target[1], 0.0f));
  assert(float_eq(camera_target[2], 0.0f));

  assert(float_eq(camera_look[0], 0.70711f));
  assert(float_eq(camera_look[1], 0.0f));
  assert(float_eq(camera_look[2], 0.70711f));

  assert(float_eq(camera_right[0], 0.70711f));
  assert(float_eq(camera_right[1], 0.0f));
  assert(float_eq(camera_right[2], -0.70711f));

  assert(float_eq(camera_up[0], 0.0f));
  assert(float_eq(camera_up[1], 1.0f));
  assert(float_eq(camera_up[2], 0.0f));

  GLfloat lookat[16];
  set_lookat(lookat, camera_location, camera_right, camera_up, camera_look);

  GLfloat projection[16];

  GLfloat* view = lookat;

  GLfloat model_base[] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
    };

  GLfloat model_rotation[] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };

  GLfloat model[16];

  GLfloat camera_speed = 0.05f;
  int moving_forward = 0;
  int moving_backward = 0;
  int moving_left = 0;
  int moving_right = 0;
  int moving_up = 0;
  int moving_down = 0;

  GLfloat yaw = 0;
  GLfloat pitch = 0;

  int input_grab = 0;

  int done = 0;
  SDL_Event ev;
  while(!done) {
    while(SDL_PollEvent(&ev)) {
      switch(ev.type) {
      case SDL_KEYUP:
	switch(ev.key.keysym.sym) {
	case SDLK_q:
	  done = 1;
	  break;
	case SDLK_e:
	  input_grab = !input_grab;
	  SDL_SetWindowGrab(main_window, input_grab);
	  SDL_ShowCursor(!input_grab);
	  SDL_SetRelativeMouseMode(input_grab);
	  break;
	case SDLK_SPACE:
	  moving_up = 0;
	  break;
	case SDLK_z:
	  moving_down = 0;
	  break;
	case SDLK_w:
	  moving_forward = 0;
	  break;
	case SDLK_a:
	  moving_left = 0;
	  break;
	case SDLK_s:
	  moving_backward = 0;
	  break;
	case SDLK_d:
	  moving_right = 0;
	  break;
	}
	break;
      case SDL_KEYDOWN:
	switch(ev.key.keysym.sym) {
	case SDLK_SPACE:
	  moving_up = 1;
	  break;
	case SDLK_z:
	  moving_down = 1;
	  break;
	case SDLK_w:
	  moving_forward = 1;
	  break;
	case SDLK_a:
	  moving_left = 1;
	  break;
	case SDLK_s:
	  moving_backward = 1;
	  break;
	case SDLK_d:
	  moving_right = 1;
	  break;
	}
	break;
      case SDL_MOUSEMOTION:
	{
	  GLfloat xoffset = ev.motion.xrel;
	  GLfloat yoffset = ev.motion.yrel;

	  GLfloat sensitivity = 0.15f;
	  xoffset *= sensitivity;
	  yoffset *= sensitivity;

	  yaw += xoffset;
	  pitch += yoffset;

	  CLAMP(pitch, -89, 89);
	  //printf("yaw: %f; pitch: %f\n", yaw, pitch);
	  camera_look[0] = cos(pitch * M_PI / 180.0f) * cos(yaw * M_PI / 180.0f);
	  camera_look[1] = sin(pitch * M_PI / 180.0f);
	  camera_look[2] = cos(pitch * M_PI / 180.0f) * sin(yaw * M_PI / 180.0f);
	}
      default:
	break;
      }
    }

    GLfloat velocity[3];
    if(moving_forward) {
      memcpy(velocity, camera_look, sizeof(velocity));
      //Negate because camera_look faces opposite of camera
      mul_vector3(velocity, -camera_speed);
      add_vector3(camera_location, velocity);
    }
    if(moving_backward) {
      memcpy(velocity, camera_look, sizeof(velocity));
      //Don't negate because camera_look faces opposite of camera
      mul_vector3(velocity, camera_speed);
      add_vector3(camera_location, velocity);
    }
    if(moving_right) {
      memcpy(velocity, camera_right, sizeof(velocity));
      mul_vector3(velocity, camera_speed);
      add_vector3(camera_location, velocity);
    }
    if(moving_left) {
      memcpy(velocity, camera_right, sizeof(velocity));
      mul_vector3(velocity, -camera_speed);
      add_vector3(camera_location, velocity);
    }
    if(moving_up) {
      memcpy(velocity, camera_up, sizeof(velocity));
      mul_vector3(velocity, camera_speed);
      add_vector3(camera_location, velocity);
    }
    if(moving_down) {
      memcpy(velocity, camera_up, sizeof(velocity));
      mul_vector3(velocity, -camera_speed);
      add_vector3(camera_location, velocity);
    }
    GLint time_uniform_location = glGetUniformLocation(shader_program, "time");
    GLint sampler_uniform_location = glGetUniformLocation(shader_program, "tex");
    GLint projection_uniform_location = glGetUniformLocation(shader_program, "projection");
    GLint view_uniform_location = glGetUniformLocation(shader_program, "view");
    GLint model_uniform_location = glGetUniformLocation(shader_program, "model");
    GLint ambient_uniform_location = glGetUniformLocation(shader_program, "ambient_light");
    GLint diffuse_uniform_location = glGetUniformLocation(shader_program, "diffuse_light_pos");
    GLint diffcolor_uniform_location = glGetUniformLocation(shader_program, "diffuse_light_color");
    GLint lighting_uniform_location = glGetUniformLocation(shader_program, "enable_lighting");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program);

    glUniform1ui(time_uniform_location, SDL_GetTicks());
    glUniform3f(ambient_uniform_location,
		0.1f,
		0.1f,
		0.1f);
    glUniform3f(diffuse_uniform_location,
		1.0f,
		1.0f,
		1.0f);
    glUniform3f(diffcolor_uniform_location,
		1.0f,
		1.0f,
		1.0f);

    set_projection_matrix(projection,
			  1024.0f, 768.0f, M_PI_2,
			  1.0f, 100.0f);
    int enable_rotation = 1;
    GLfloat angle_deg = SDL_GetTicks() / 20.0f;
    GLfloat angle_rad = angle_deg / 180.0f * M_PI;
    set_camera_vectors(NULL,
		       NULL,
		       camera_look,
		       camera_right,
		       camera_up);
    set_lookat(lookat, camera_location, camera_right, camera_up, camera_look);

    glUniformMatrix4fv(projection_uniform_location,
		       1,
		       GL_TRUE,
		       projection);
    glUniformMatrix4fv(view_uniform_location,
		       1,
		       GL_TRUE,
		       view);

    glUniform1ui(lighting_uniform_location, 1);
    //ground
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ground_tex);
    glUniform1i(sampler_uniform_location, 0);

    memcpy(model, model_base, sizeof(model_base));
    glUniformMatrix4fv(model_uniform_location,
		       1,
		       GL_TRUE,
		       model);

    glBindVertexArray(ground_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);



    memcpy(model, model_base, sizeof(model_base));
    if(enable_rotation) {
      model_rotation[5] = cos(-angle_rad);
      model_rotation[6] = -sin(-angle_rad);
      model_rotation[9] = sin(-angle_rad);
      model_rotation[10] = cos(-angle_rad);
      mat_mul4(model, model_rotation);
    }
    glUniformMatrix4fv(model_uniform_location,
		       1,
		       GL_TRUE,
		       model);

    glUniform1ui(lighting_uniform_location, 1);
    //me
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(sampler_uniform_location, 0);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLfloat), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);



    memcpy(model, model_base, sizeof(model_base));
    model[3] = 1.0f;
    model[7] = 1.0f;
    model[11] = 1.0f;
    model[0] = 0.1f;
    model[5] = 0.1f;
    model[10] = 0.1f;
    glUniformMatrix4fv(model_uniform_location,
		       1,
		       GL_TRUE,
		       model);
    glUniform1ui(lighting_uniform_location, 0);
    //light
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, white_tex);
    glUniform1i(sampler_uniform_location, 0);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLfloat), GL_UNSIGNED_INT, 0);
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
