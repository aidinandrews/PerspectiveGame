#version 430 core // We need 430 for SSBOs.

// INPUTS:

layout(location = 0) in vec2 pixelWorldPos;
layout(location = 1) in vec2 povWorldPos;

uniform float     deltaTime;
uniform float     updateProgress;
uniform sampler2D inTexture;
//uniform mat4      inPovRelativePositions;
uniform int initialTileIndex;
uniform int initialMapIndex;

struct Tile {
	int neighborIndices[4];
	int maps[4];

	int entityPositions[4];
	int entityDirections[4];

	vec2 texCoords[4];
	
	vec4 color;
	int numEntities;
	int padding[3];
};

layout (std430, binding = 1) buffer tilesBuffer { Tile tiles[]; };

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

#define NUM_ORTHO_DIRS 4

#define LOCAL_POSITION_CENTER 8

const int ORTHOGONAL_DIRECTION_SET[4] = { 
	LOCAL_DIRECTION_0,
	LOCAL_DIRECTION_1,
	LOCAL_DIRECTION_2,
	LOCAL_DIRECTION_3,
};

const vec2 LOCAL_POS_TO_COORD[9] = {
	vec2(1.0f, 0.5f),
	vec2(0.5f, 0.0f),
	vec2(0.0f, 0.5f),
	vec2(0.5f, 1.0f),
	vec2(1.0f, 0.0f),
	vec2(0.0f, 0.0f),
	vec2(0.0f, 1.0f),
	vec2(1.0f, 1.0f),
	vec2(0.5f, 0.5f), // center
};

const vec2 LOCAL_DIR_TO_VEC[9] = {
	vec2(1.0f, 0.0f),
	vec2(0.0f, -1.0f),
	vec2(-1.0f, 0.0f),
	vec2(0.0f, 1.0f),
	vec2(1.0f, -1.0f),
	vec2(-1.0f, -1.0f),
	vec2(-1.0f, 1.0f),
	vec2(1.0f, 1.0f),
	vec2(0.0f, 0.0f), // static
};

#define MAX_STEPS 500

int currentTileIndex = initialTileIndex;
int currentMapIndex = initialMapIndex;

#define CurrentTile tiles[currentTileIndex]

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

vec2 getPixelPos() {
	// catch mirrored local directions:
	int s = getLocalSouth(), w = getLocalWest(), n = getLocalNorth();
	if (currentMapIndex > 3) { s = ++s % 4; w = ++w % 4; n = ++n % 4; }

	vec2 southEastUV = CurrentTile.texCoords[s];
	vec2 southWestUV = CurrentTile.texCoords[w];
	vec2 northWestUV = CurrentTile.texCoords[n];

	vec2 xDir = southEastUV - southWestUV;
	vec2 yDir = northWestUV - southWestUV;
	return southWestUV + (localPixelPosition.x * xDir) + (localPixelPosition.y * yDir);
}

bool colorPixelInsideEntity(vec2 pixelPos) {
	for (int i = 0; i < CurrentTile.numEntities; i++) {
		vec2 entityPos = LOCAL_POS_TO_COORD[CurrentTile.entityPositions[i]];
		vec2 dir = LOCAL_DIR_TO_VEC[CurrentTile.entityDirections[i]] / 2.0f;
		vec2 offset = dir * updateProgress;
		entityPos += offset;

		if (abs(entityPos.x - pixelPos.x) < 0.5f && 
			abs(entityPos.y - pixelPos.y) < 0.5f) {
			gl_FragColor = vec4(0, 1, 0, 1);
			return true;
		}
	}

	for (int dir = LOCAL_DIRECTION_0; dir < NUM_ORTHO_DIRS; dir++) {
		int ni = CurrentTile.neighborIndices[dir];
		int mappedDir = MAP_DIRECTION[CurrentTile.maps[dir]][(dir + 2) % 4];
		#define neighborTile tiles[ni]

		if (neighborTile.numEntities != 1 || // only center-positioned tiles can spill over to neighbors
			neighborTile.entityPositions[0] != LOCAL_POSITION_CENTER ||
			neighborTile.entityDirections[0] != mappedDir) 
				continue; 

		vec2 entityPos = LOCAL_POS_TO_COORD[dir] + (LOCAL_DIR_TO_VEC[dir] / 2.0f);
		entityPos -= (LOCAL_DIR_TO_VEC[dir] / 2.0f) * updateProgress;

		if (abs(entityPos.x - pixelPos.x) < 0.5f && 
			abs(entityPos.y - pixelPos.y) < 0.5f) {
			gl_FragColor = vec4(0, 1, 0, 1);
			return true;
		}
	}

	return false;
}

void colorPixel() {
	vec2 pixelPos = getPixelPos();

	if (!colorPixelInsideEntity(pixelPos)) {
		gl_FragColor = mix(texture(inTexture, pixelPos), CurrentTile.color, 0.5);
	}
}

// transitions currentTileIndex and currentMapIndex 1 tile over in the given direction (d).
void shiftCurrentTile(int d) {
	currentMapIndex = COMBINE_MAP_INDICES[currentMapIndex][CurrentTile.maps[d]];
	currentTileIndex = CurrentTile.neighborIndices[d];
}

bool findTile() {
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
			if (GO_WINDOW_EAST) { shiftCurrentTile(getLocalEast()); } 
			else { shiftCurrentTile(getLocalWest()); }
			runningDist.x += stepDist.x;
		} 
		else { // runningDist.x > runningDist.y
			if (GO_WINDOW_NORTH) { shiftCurrentTile(getLocalNorth()); } 
			else { shiftCurrentTile(getLocalSouth()); }
			runningDist.y += stepDist.y;
		}
	}
	return stepCount < MAX_STEPS;
}

void main() {
	// Quick check to see if we are in the initial tile.  If so, we don't have to bother raycasting:
	if (pixelWorldPos.x > 0 && pixelWorldPos.x < 1 && pixelWorldPos.y > 0 && pixelWorldPos.y < 1) {
		getFragDrawTilePos();
		//getRelativeVertPositions();
		colorPixel();
		return;
	}

	bool foundTile = findTile();
	if (foundTile) {
		getFragDrawTilePos();
		//getRelativeVertPositions();
		colorPixel();
	}
	else {
		gl_FragColor = vec4(0, 0, 0, 1);
	}
};