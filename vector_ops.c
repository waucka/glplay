#include "vector_ops.h"

#include <string.h>
#include <math.h>

void cross_product3(GLfloat* u, GLfloat* v) {
  GLfloat uxv[3];
  uxv[0] = u[1] * v[2] - u[2] * v[1];
  uxv[1] = u[2] * v[0] - u[0] * v[2];
  uxv[2] = u[0] * v[1] - u[1] * v[0];
  memcpy(u, uxv, sizeof(uxv));
}

void mul_vector3(GLfloat* v, GLfloat scalar) {
  v[0] *= scalar;
  v[1] *= scalar;
  v[2] *= scalar;
}

void add_vector3(GLfloat* u, GLfloat* v) {
  u[0] += v[0];
  u[1] += v[1];
  u[2] += v[2];
}

void normalize3(GLfloat* v) {
  GLfloat len = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  v[0] = v[0] / len;
  v[1] = v[1] / len;
  v[2] = v[2] / len;
}

