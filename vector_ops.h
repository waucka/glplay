#ifndef VECTOR_OPS_H
#define VECTOR_OPS_H

#include <epoxy/gl.h>

void cross_product3(GLfloat* u, GLfloat* v);

void mul_vector3(GLfloat* v, GLfloat scalar);

void add_vector3(GLfloat* u, GLfloat* v);

void normalize3(GLfloat* v);

#endif
