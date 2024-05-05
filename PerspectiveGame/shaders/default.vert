#version 330 core

in vec3 inPos;
in vec3 inNormal;
in vec3 inColor;
in vec2 inTexCoord;

out vec3 fragPos;
out vec3 fragColor;
out vec3 fragNormal;
out vec2 fragTexCoord;
out float shininess;
out float Ks;
out float Kn;
out vec3 lightPos;
out vec3 camPos;

uniform mat4 inTransfMatrix;
uniform mat4 inModelMatrix;
uniform vec3 inLightPos;
uniform vec3 inCamPos;
	
void main() {
	fragPos = vec3(inModelMatrix * vec4(inPos, 1.0f));
	gl_Position = inTransfMatrix * vec4(fragPos, 1.0f);

	fragNormal = inNormal;

	fragColor = inColor;
	fragTexCoord = inTexCoord;
	
	camPos = inCamPos;	
	lightPos = inLightPos;
	
	shininess = 2;
	Ks = 0.3;
	Kn = 0.6;
};