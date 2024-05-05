#version 330 core

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in float fragAlpha;
layout (location = 3) in float fragColorAlpha;
layout (location = 4) in vec4 fragGlPos;
layout (location = 5) in vec3 fragWorldPos;
layout (location = 6) flat in int fragTileIndex;

uniform sampler2D inTexture;
uniform vec3 inPlayerPos;
uniform mat4 inPlayerPosInfo;

const float MAX_Z_DIF = 0.5f;
int playerPosTileIndices[4] = {
    int(inPlayerPosInfo[0][3]),
    int(inPlayerPosInfo[1][3]),
    int(inPlayerPosInfo[2][3]),
    int(inPlayerPosInfo[3][3])
};
vec3 playerPositions[4] = {
    vec3(inPlayerPosInfo[0][0], inPlayerPosInfo[0][1], inPlayerPosInfo[0][2]),
    vec3(inPlayerPosInfo[1][0], inPlayerPosInfo[1][1], inPlayerPosInfo[1][2]),
    vec3(inPlayerPosInfo[2][0], inPlayerPosInfo[2][1], inPlayerPosInfo[2][2]),
    vec3(inPlayerPosInfo[3][0], inPlayerPosInfo[3][1], inPlayerPosInfo[3][2])
};

void main() {
    if (fragGlPos.z < inPlayerPos.z - MAX_Z_DIF) {
        discard;
        return;
    }
    for (int i = 0; i < 4; i++) {
        if (fragTileIndex == playerPosTileIndices[i]
            && length(fragWorldPos - playerPositions[i]) < 0.25f) {
//            vec4 clr = vec4(fragColor, 1);
//            clr -= vec4(1, 1, 1, 1);
//            clr *= -1.0f;
//            clr.a = 1.0f;
//            gl_FragColor = clr;
            gl_FragColor = vec4(1, 1, 1, 1);
            return;
        }
    }

   gl_FragColor = mix(texture(inTexture, fragTexCoord), vec4(fragColor, 1), fragColorAlpha);
}