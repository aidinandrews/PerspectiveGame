#version 330 core

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in float fragAlpha;
layout (location = 3) in float fragColorAlpha;

uniform sampler2D inTexture;
uniform vec3 inPlayerPos;

void main() {
   gl_FragColor = mix(texture(inTexture, fragTexCoord), vec4(fragColor, 1), fragColorAlpha);
   gl_FragColor.a = fragAlpha;
}