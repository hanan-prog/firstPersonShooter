#version 150 core

in vec2 texcoord;

out vec4 outColor;

uniform sampler2D hudTexure;

void main() { outColor = vec4(texture(hudTexure, texcoord).rgb, 1.0); }