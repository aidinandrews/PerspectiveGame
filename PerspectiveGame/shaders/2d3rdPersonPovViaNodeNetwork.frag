#version 430 core // We need 430 for SSBOs.

// INPUTS:

layout(location = 0) in vec2 pixelWorldPos;
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

vec2 localPixelPosition;
const vec2  povToPixelPos = pixelWorldPos - povWorldPos;
const float totalDist = length(povToPixelPos);

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

int getLocalEast()  { return MAP_DIRECTION[currentMapIndex][LOCAL_DIRECTION_0]; }
int getLocalSouth() { return MAP_DIRECTION[currentMapIndex][LOCAL_DIRECTION_1]; }
int getLocalWest()  { return MAP_DIRECTION[currentMapIndex][LOCAL_DIRECTION_2]; }
int getLocalNorth() { return MAP_DIRECTION[currentMapIndex][LOCAL_DIRECTION_3]; }

// ALIGNMENT_MAP_COMBINATIONS[initial mapping][next mapping]
const int COMBINE_MAP_INDICES[8][8] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 1, 2, 3, 0, 7, 4, 5, 6 },
	{ 2, 3, 0, 1, 6, 7, 4, 5 },
	{ 3, 0, 1, 2, 5, 6, 7, 4 },
	{ 4, 5, 6, 7, 0, 1, 2, 3 },
	{ 5, 6, 7, 4, 3, 0, 1, 2 },
	{ 6, 7, 4, 5, 2, 3, 0, 1 },
	{ 7, 4, 5, 6, 1, 2, 3, 0 }
};

// FUNCTIONS

// Returns the coordinates of the fragment local to its tile.
// Domain is 0-1 for x and y and is relative to the screen.  (x+ == right, y+ == up).
void getFragDrawTilePos() {
	localPixelPosition = (pixelWorldPos) - floor(pixelWorldPos);
}

// Given a local draw tile coordinate, will return the color of the base texture of a tile.
vec4 tileTexColor() {
	// catch mirrored local directions:
	int s = getLocalSouth(), w = getLocalWest(), n = getLocalNorth();
	if (currentMapIndex > 3) { s = ++s % 4; w = ++w % 4; n = ++n % 4; }

	vec2 southEastUV = CurrentTileInfo.texCoords[s];
	vec2 southWestUV = CurrentTileInfo.texCoords[w];
	vec2 northWestUV = CurrentTileInfo.texCoords[n];

	vec2 xDir = southEastUV - southWestUV;
	vec2 yDir = northWestUV - southWestUV;
	vec2 texCoord = southWestUV + (localPixelPosition.x * xDir) + (localPixelPosition.y * yDir);
	
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
	int M = nodeInfos[currentNodeIndex].neighborMapIndices[d];
	currentNodeIndex = nodeInfos[currentNodeIndex].neighborIndices[d];

	// Adjust the window space -> tile space mappings:
	currentMapIndex = COMBINE_MAP_INDICES[currentMapIndex][m];
	currentMapIndex = COMBINE_MAP_INDICES[currentMapIndex][M];
}

void findTile() {
	vec2 runningDist;
	vec2 stepDist = totalDist / abs(povToPixelPos);
	int stepCount = 0;
	const bool GO_WINDOW_EAST  = povToPixelPos.x > 0.0f;
	const bool GO_WINDOW_NORTH = povToPixelPos.y > 0.0f;

	// Setup:
	if (GO_WINDOW_EAST) { runningDist.x = (1.0f - povWorldPos.x) * stepDist.x; } 
	else { runningDist.x = povWorldPos.x * stepDist.x; }
	
	if (GO_WINDOW_NORTH) { runningDist.y = (1.0f - povWorldPos.y) * stepDist.y; } 
	else { runningDist.y = povWorldPos.y * stepDist.y; }

	// Raycast:
	while (stepCount++ < MAX_STEPS) {
		if (runningDist.x > totalDist && runningDist.y > totalDist) { break; } // We have arrived!

		if (runningDist.x < runningDist.y) {
			if (GO_WINDOW_EAST) { shiftCurrentTile(getLocalEast()); } else { shiftCurrentTile(getLocalWest()); }
			runningDist.x += stepDist.x;
		} 
		else { // runningDist.x > runningDist.y
			if (GO_WINDOW_NORTH) { shiftCurrentTile(getLocalNorth()); } else { shiftCurrentTile(getLocalSouth()); }
			runningDist.y += stepDist.y;
		}
	}
}

void main() {
	// Quick check to see if we are in the initial tile.  If so, we don't have to bother raycasting:
	if (pixelWorldPos.x > 0 && pixelWorldPos.x < 1 && pixelWorldPos.y > 0 && pixelWorldPos.y < 1) {
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