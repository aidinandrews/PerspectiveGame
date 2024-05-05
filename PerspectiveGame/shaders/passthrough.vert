#version 330 core

layout(location = 0) in vec2 inVertPos;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inColor;

out vec3 outColor;

void main() {
	gl_Position = vec4(inVertPos, 0, 1);
	outColor = inColor;
}
