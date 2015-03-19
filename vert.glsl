#version 330 core

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord_in;
layout (location = 2) in vec3 normal_in;

out vec2 texcoord;
out vec3 normal;
out vec3 frag_pos;

void main() {
  gl_Position = projection * view * model * vec4(position, 1.0);
  texcoord = texcoord_in;
  normal = normalize(vec3(model * vec4(normal_in, 1.0f)));
  frag_pos = vec3(model * vec4(position, 1.0f));
}
