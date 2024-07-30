#version 330 core

layout(location = 0) in vec2 inVertPos;

layout(location = 0) out vec2 fragWorldPos;
layout(location = 1) out vec2 povPos;
layout(location = 2) out mat4 povRelativePos;

uniform mat4 inWindowToWorldSpace;
uniform vec2 inPovPosWindowSpace;

void main() {
	gl_Position = vec4(inVertPos, 0, 1);

	// y is inverted to translate from CPU side coords to OpenGL coords.
	fragWorldPos = vec2(inWindowToWorldSpace * vec4(inVertPos.x, -inVertPos.y, 0, 1));
	povPos = vec2(inWindowToWorldSpace * vec4(inPovPosWindowSpace, 0, 1));
	//povRelativePos = inPovRelativePosWindowSpace;
}