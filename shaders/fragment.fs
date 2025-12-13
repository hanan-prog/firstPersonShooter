#version 150 core

in vec3 Color;
in vec3 vertNormal;
in vec3 pos;
// in vec3 lightDir;
in vec2 texcoord;

out vec4 outColor;

uniform sampler2D tex0;
// uniform sampler2D tex1;




uniform samplerCube skybox;

void main() {
    // vec3 normal = normalize(vertNormal);
    // vec3 viewDir = normalize(-pos);
    // vec3 reflectDir = reflect(viewDir, normal);
    outColor = vec4(texture(tex0, texcoord).rgb, 1.0);
}