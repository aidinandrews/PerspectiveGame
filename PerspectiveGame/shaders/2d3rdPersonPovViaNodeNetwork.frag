#version 430 core // We need 430 for SSBOs.

// INPUTS:

layout(location = 0) in vec2 fragWorldPos;
layout(location = 1) in vec2 povWorldPos;

uniform float     deltaTime;
uniform float     updateProgress;
uniform sampler2D inTexture;
//uniform mat4      inPovRelativePositions;
uniform int initialNodeIndex;
uniform int initialMapIndex;

struct NodeInfo {
	int neighborIndices[8];
	int neighborMapIndices[8];
	
	int index;
	int tileInfoIndex;
	int padding[6];
};
struct TileInfo {
	vec2 texCoords[4];

	vec4 color;
	int nodeIndex;
	int padding[3];
};

layout (std430, binding = 1) buffer positionNodeInfosBuffer { NodeInfo nodeInfos[]; };
layout (std430, binding = 2) buffer tileInfosBuffer { TileInfo tileInfos[]; };

// GLOBAL VARIABLES:

#define LOCAL_DIRECTION_0 0
#define LOCAL_DIRECTION_1 1
#define LOCAL_DIRECTION_2 2
#define LOCAL_DIRECTION_3 3
#define LOCAL_DIRECTION_0_1 4
#define LOCAL_DIRECTION_1_0 4
#define LOCAL_DIRECTION_1_2 5
#define LOCAL_DIRECTION_2_1 5
#define LOCAL_DIRECTION_2_3 6
#define LOCAL_DIRECTION_3_2 6
#define LOCAL_DIRECTION_3_0 7
#define LOCAL_DIRECTION_0_3 7

#define MAX_STEPS 1000

#define CurrentNode nodeInfos[currentNodeIndex]
#define CurrentTileInfo tileInfos[CurrentNode.tileInfoIndex]

int currentNodeIndex = initialNodeIndex;
int currentMapIndex = initialMapIndex;

vec2 fragDrawTilePos;
const vec2  povPosToPixelPos = fragWorldPos - povWorldPos;
const float totalDist = length(povPosToPixelPos);
const bool  goingRight = povPosToPixelPos.x > 0.0f;
const bool  goingUp = povPosToPixelPos.y > 0.0f;

// CONST FUNCTIONS:

// MAP_DIRECTION[map index][direction to map]
const int MAP_DIRECTION[8][8] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 1, 2, 3, 0, 5, 6, 7, 4 },
	{ 2, 3, 0, 1, 6, 7, 4, 5 },
	{ 3, 0, 1, 2, 7, 4, 5, 6 },
	{ 0, 3, 2, 1, 7, 6, 5, 4 },
	{ 1, 0, 3, 2, 4, 7, 6, 5 },
	{ 2, 1, 0, 3, 5, 4, 7, 6 },
	{ 3, 2, 1, 0, 6, 5, 4, 7 }
};

int NORTH    = MAP_DIRECTION[initialMapIndex][3];
int EAST = MAP_DIRECTION[initialMapIndex][0];
int SOUTH  = MAP_DIRECTION[initialMapIndex][1];
int WEST  = MAP_DIRECTION[initialMapIndex][2];

// ALIGNMENT_MAP_COMBINATIONS[initial mapping][next mapping]
const int COMBINE_MAP_INDICES[8][8] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 1, 2, 3, 0, 6, 7, 4, 5 },
	{ 2, 3, 0, 1, 5, 6, 7, 4 },
	{ 3, 0, 1, 2, 4, 5, 6, 7 },
	{ 4, 5, 6, 7, 0, 1, 2, 3 },
	{ 5, 6, 7, 4, 3, 0, 1, 2 },
	{ 6, 7, 4, 5, 2, 3, 0, 1 },
	{ 7, 4, 5, 6, 1, 2, 3, 0 }
};

// FUNCTIONS

// Returns the coordinates of the fragment local to its tile.
// Domain is 0-1 for x and y and is relative to the screen.  (x+ == right, y+ == up).
void getFragDrawTilePos() {
	fragDrawTilePos = (fragWorldPos) - floor(fragWorldPos);
	// We flipped (abs) the coordinates to get in a domain of [0, 1] for negative fragWorldPositions, so now we flip them back:
	//if (fragWorldPos.x < 0) { fragDrawTilePos.x = 1 - fragDrawTilePos.x; }
	//if (fragWorldPos.y < 0) { fragDrawTilePos.y = 1 - fragDrawTilePos.y; }
}

// Given a local draw tile coordinate, will return the color of the base texture of a tile.
vec4 tileTexColor() {
	// catch mirrored local directions:
	int s = SOUTH, w = WEST, n = NORTH;
	if (currentMapIndex > 3) { s = ++s % 4; w = ++w % 4; n = ++n % 4; }

	vec2 bottomRightUV = CurrentTileInfo.texCoords[s];
	vec2 bottomLeftUV  = CurrentTileInfo.texCoords[w];
	vec2 topLeftUV     = CurrentTileInfo.texCoords[n];

	vec2 xDir = bottomRightUV - bottomLeftUV, yDir = topLeftUV - bottomLeftUV;
	vec2 texCoord = bottomLeftUV + (fragDrawTilePos.x * xDir) + (fragDrawTilePos.y * yDir);
	
	return texture(inTexture, texCoord);
}

void colorPixel() {
	gl_FragColor = mix(tileTexColor(), CurrentTileInfo.color, 0.5);
}

// transitions currentNodeIndex and currentMapIndex 1 tile over in the given direction (d).
void shiftCurrentTile(int d) {
	int D = d;
	int m = nodeInfos[currentNodeIndex].neighborMapIndices[d];

	// Transition to a side node:
	d = MAP_DIRECTION[nodeInfos[currentNodeIndex].neighborMapIndices[d]][d];
	currentNodeIndex = nodeInfos[currentNodeIndex].neighborIndices[D];

	// Transistion to the neighbor tile (a center node).
	currentMapIndex = nodeInfos[currentNodeIndex].neighborMapIndices[d];
	currentNodeIndex = nodeInfos[currentNodeIndex].neighborIndices[d];

	// Adjust the window space -> tile space mappings:
	m = COMBINE_MAP_INDICES[m][currentMapIndex];
	WEST = MAP_DIRECTION[m][WEST];
	EAST = MAP_DIRECTION[m][EAST];
	NORTH = MAP_DIRECTION[m][NORTH];
	SOUTH = MAP_DIRECTION[m][SOUTH];
}

void findTile() {
	vec2 runningDist;
	vec2 stepDist = totalDist / abs(povPosToPixelPos);
	int stepCount = 0;
	int connectionIndex;
	int drawSideIndex;
	float currentDist = 0; // Used for incrementing the ray:

	// Setup:
	if (goingRight) { runningDist.x = (1.0f - povWorldPos.x) * stepDist.x; } 
	else { runningDist.x = povWorldPos.x * stepDist.x; }
	
	if (goingUp) { runningDist.y = (1.0f - povWorldPos.y) * stepDist.y; } 
	else { runningDist.y = povWorldPos.y * stepDist.y; }

	// Raycast:
	while (stepCount < MAX_STEPS) {
		if (runningDist.x < runningDist.y) {
			if ((currentDist = runningDist.x) > totalDist) { break; } // We have arrived!
			
			if (goingRight) { shiftCurrentTile(EAST); } 
			else { shiftCurrentTile(WEST); } // Going left
			runningDist.x += stepDist.x;
		} 
		else { // runningDist.x > runningDist.y
			if ((currentDist = runningDist.y) > totalDist) { break; } // We have arrived!
			
			if (goingUp) { shiftCurrentTile(NORTH); } 
			else { shiftCurrentTile(SOUTH);} // Going down
			runningDist.y += stepDist.y;
		}
		stepCount++;
	}
}

void main() {
	// Quick check to see if we are in the initial tile.  If so, we don't have to bother raycasting:
	if (fragWorldPos.x > 0 && fragWorldPos.x < 1 && fragWorldPos.y > 0 && fragWorldPos.y < 1) {
		getFragDrawTilePos();
		//getRelativeVertPositions();
		colorPixel();
		return;
	}

	findTile();
	getFragDrawTilePos();
	//getRelativeVertPositions();
	colorPixel();

	//gl_FragColor = CurrentTileInfo.color;
};