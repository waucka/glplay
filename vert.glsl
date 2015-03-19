#version 330 core

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord_in;

out vec2 texcoord;

void main() {
  gl_Position = projection * view * model * vec4(position.x, position.y, position.z, 1.0);
  texcoord = texcoord_in;
}
