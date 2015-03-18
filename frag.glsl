#version 330 core

uniform uint time;
uniform sampler2D tex;
uniform bool pulsate;

in vec4 vcolor;
in vec2 texcoord;

out vec4 color;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
  vec3 mixedcolor = vcolor.rgb;
  //color = vec4(1.0f, 0.5f, 0.25f, 1.0f);
  if(pulsate) {
    float factor = ((cos(time / 500.0f) + 1.0f) / 2.0f);
    vec3 hsv = rgb2hsv(vcolor.rgb);
    //hsv.r = clamp(hsv.r + factor, 0.0f, 1.0f);
    //hsv.g *= factor;
    hsv.b *= factor;
    mixedcolor = hsv2rgb(hsv);
  }

  vec4 texcolor = texture(tex, vec2(texcoord.s, 1.0f - texcoord.t));
  //color = texcolor;
  color = vec4(texcolor.r * mixedcolor.r, texcolor.g * mixedcolor.g, texcolor.b * mixedcolor.b, texcolor.a);
  //color = vec4(texcoord.st, 0.0f, 1.0f);
}
