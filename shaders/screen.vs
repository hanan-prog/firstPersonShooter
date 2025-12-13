#version 150 core

in vec3 position;
in vec2 inTexcoord;

out vec2 texcoord;

void main() {
  texcoord = inTexcoord;
  gl_Position = vec4(position.x, position.y, 0.0, 1.0);
 
}