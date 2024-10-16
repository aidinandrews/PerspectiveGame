#version 330 core

layout(location = 0) in vec2 inVertPos; // Set via setVertAttribVec2Pos() in guiManager.

layout(location = 0) out vec2 fragWorldPos;
layout(location = 1) out vec2 povWorldPos;

uniform mat4 inWindowToWorldSpace;

void main() {
	gl_Position = vec4(inVertPos, 0, 1);

	// y is inverted to translate from CPU side coords to OpenGL coords.
	fragWorldPos = vec2(inWindowToWorldSpace * vec4(inVertPos.x, -inVertPos.y, 0, 1));
	povWorldPos = vec2(inWindowToWorldSpace * vec4(0, 0, 0, 1));
}