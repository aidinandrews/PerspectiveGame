#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inTexCoord;
layout (location = 4) in float inTileIndex;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexCoord;
layout (location = 2) out float fragAlpha;
layout (location = 3) out float fragColorAlpha;
layout (location = 4) out vec4 fragGlPos;
layout (location = 5) out vec3 fragWorldPos;
layout (location = 6) flat out int fragTileIndex;

uniform mat4 inTransfMatrix;
uniform float inAlpha;
uniform float inColorAlpha;

void main() {
   vec4 pos = inTransfMatrix * vec4(inPos, 1); 
   pos.y *= -1; // OpenGL expects inversed y.
   gl_Position = pos;
   fragGlPos = gl_Position;
   fragWorldPos = inPos;

   fragColor = inColor;
   fragTexCoord = inTexCoord;
   fragAlpha = inAlpha;
   fragColorAlpha = inColorAlpha;
   fragTileIndex = int(inTileIndex);
}