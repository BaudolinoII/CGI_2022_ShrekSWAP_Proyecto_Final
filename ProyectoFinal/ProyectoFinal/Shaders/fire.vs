#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

const float amplitude = 0.125;
const float frequency = 4.0;
const float PI = 3.14159;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform float aleatorio;

void main(){
  float distance = length(aPos);
  float effect = amplitude*sin(-PI*frequency + time*aleatorio);
  gl_Position = projection*view*model*vec4(aPos.x,aPos.y-effect*0.5, aPos.z,1);
  TexCoords=vec2(aTexCoords.x + effect*0.001 ,aTexCoords.y + effect*0.001);
}