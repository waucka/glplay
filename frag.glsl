#version 330 core

uniform uint time;
uniform sampler2D tex;
uniform vec3 ambient_light;

in vec2 texcoord;

out vec4 color;

void main() {
  vec4 texcolor = texture(tex, vec2(texcoord.s, 1.0f - texcoord.t));
  color = texcolor * vec4(ambient_light, 1.0f);
}
