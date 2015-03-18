#include "matrix_ops.h"

#include <string.h>
#include <math.h>

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
    memcpy(rowcopy, current_row, rowsize);
    for(int j = 0; j < 4; j++) {
      current_row[j] = 0;
      for(int k = 0; k < 4; k++) {
	current_row[j] += rowcopy[k] * mat2[k * 4 + j];
      }
    }
  }
}

void set_identity4(GLfloat* mat) {
  memset(mat, 0, sizeof(GLfloat) * 16);
  mat[0] = 1.0f;
  mat[5] = 1.0f;
  mat[10] = 1.0f;
  mat[15] = 1.0f;
}

void set_lookat(GLfloat* lookat,
		GLfloat* camera_location,
		GLfloat* camera_right,
		GLfloat* camera_up,
		GLfloat* camera_look) {
  set_identity4(lookat);
  memcpy(lookat, camera_right, sizeof(GLfloat) * 3);
  memcpy(lookat + 4, camera_up, sizeof(GLfloat) * 3);
  memcpy(lookat + 8, camera_look, sizeof(GLfloat) * 3);

  GLfloat mat[16];
  set_identity4(mat);
  mat[3] = -camera_location[0];
  mat[7] = -camera_location[1];
  mat[11] = -camera_location[2];

  mat_mul4(lookat, mat);
}
