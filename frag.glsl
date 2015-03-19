#version 330 core

uniform uint time;
uniform sampler2D tex;
uniform vec3 ambient_light;
uniform vec3 diffuse_light_pos;
uniform vec3 diffuse_light_color;
uniform bool enable_lighting;

in vec2 texcoord;
in vec3 normal;
in vec3 frag_pos;

out vec4 color;

void main() {
  vec4 texcolor = texture(tex, vec2(texcoord.s, 1.0f - texcoord.t));
  if(enable_lighting) {
    vec3 norm = normal;
    vec3 light_dir = normalize(diffuse_light_pos - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0f);
    vec3 diffuse = diff * diffuse_light_color;
    color = texcolor * vec4(ambient_light + diffuse, 1.0f);
  } else {
    color = texcolor;
  }
}
