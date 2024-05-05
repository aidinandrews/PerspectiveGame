#version 330 core

in vec3 fragPos;

out vec4 outColor;

uniform vec3 inColor;

void main() {
	outColor = vec4(inColor, 1.0f);
	//outColor = vec4(0, 0, 0, 0);
};