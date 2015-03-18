#ifndef MATRIX_OPS_H
#define MATRIX_OPS_H

#include <epoxy/gl.h>

void set_projection_matrix(GLfloat* mat,
			   GLfloat width, GLfloat height, GLfloat fov,
			   GLfloat near_clip, GLfloat far_clip);
/*
 * Sets mat1 to mat1 * mat2
 */
void mat_mul4(GLfloat* mat1, GLfloat* mat2);
void set_identity4(GLfloat* mat);
void set_lookat(GLfloat* lookat,
		GLfloat* camera_location,
		GLfloat* camera_right,
		GLfloat* camera_up,
		GLfloat* camera_look);
void set_camera_vectors(GLfloat* camera_location,
			GLfloat* camera_target,
			GLfloat* camera_look,
			GLfloat* camera_right,
			GLfloat* camera_up);
#endif
