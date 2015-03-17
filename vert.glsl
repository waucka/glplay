#version 330 core

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 vcolor_in;
layout (location = 2) in vec2 texcoord_in;

out vec4 vcolor;
out vec2 texcoord;

void main() {
  gl_Position = projection * view * model * vec4(position.x, position.y, position.z, 1.0);
  vcolor = vcolor_in;
  texcoord = texcoord_in;
}
