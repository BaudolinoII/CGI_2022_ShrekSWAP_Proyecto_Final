#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D texture1;
uniform float activeAlpha;

void main(){
  vec4 texColor= texture(texture1,TexCoords);
    if(texColor.a < 0.1 && activeAlpha > 0.0)
        discard;
    FragColor = texColor;
}