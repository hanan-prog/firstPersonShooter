#version 150 core

in vec3 texcoord;

out vec4 outColor;

uniform samplerCube skybox;

void main() {
     outColor = texture(skybox, texcoord); 
     }