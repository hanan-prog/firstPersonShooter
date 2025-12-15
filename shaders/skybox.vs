#version 150 core

in vec3 position;
out vec3 texcoord;

uniform mat4 view;
uniform mat4 proj;


void main() {
    texcoord = vec3(position.xy, -position.z);
    vec4 pos = proj * view * vec4(position, 1.0);
    gl_Position = pos.xyww;
}