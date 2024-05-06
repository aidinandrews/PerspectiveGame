#version 430 core // We need 430 for SSBOs.

layout(location = 0) in vec2 fragWorldPos;
layout(location = 1) in vec2 povPos;

uniform int numFrames;
uniform int initialTileIndex;
uniform int initialSideIndex;
uniform int initialTexCoordIndex;
uniform int initialSideOffset;
uniform sampler2D inTexture;
uniform mat4 inPovRelativePositions;

struct tileInfo {
	int neighborIndices[4];
	int neighborMirrored[4]; // 1x4 bits
	int neighborSideIndex[4]; // 2x4 bits
	
	vec4 color; // could be vec3 
	vec2 texCoords[4];

	int basisType;
	int basisOrientation; // 2 bits

	int hasForce; // 1 bit
	int forceDirection; // 2 bits

	int entityType;
	float entityOffset; // 2 bits (+ interpolation with delta time)
	int entityDirection; // 2 bits
	int entityOrientation; // 2 bits
	
	// alignas is a bastard and scales the struct size by the largest alignas(n) n value for some reason.  
	// This means we need a buffer in here so the stride length is consistent between OpenGl's definition 
	// and the shader's struct.
	// float bufferSpace[]; 
};

layout (std430, binding = 1) buffer tileInfosBuffer { 
	tileInfo tileInfos[]; 
};

#define PI 3.1415926535897932384626433832795

// Entity IDs:
#define NONE			0
#define OMNI			1
#define MATERIAL_A		2
#define MATERIAL_B		3

#define COMPRESSOR		4
#define FORCE_BLOCK		5
#define DISPERSER		6
#define FORCE_MIRROR	7

// Basis IDs:
#define EMPTY			0
#define PRODUCER		1
#define CONSUMER		2
#define FORCE			3
#define DISPERCER		4

#define TRUE 1
#define FALSE 0
// Protect against infinite loops.
#define MAX_STEPS 1000 

const int VERT_INFO_OFFSETS[] = { 2,1,0,3 };
const vec2 DRAW_TILE_COORDS[] = { vec2(1, 1), vec2(1, 0), vec2(0, 0), vec2(0, 1) };
const bool IS_OBJECT[] = { false, true, true, true, false, false, false, false, };
const vec2 ADJACENT_ENTITY_OFFSETS[] = { vec2(1, 0), vec2(0, -1), vec2(-1, 0), vec2(0, 1) };
const vec2 povPosToPixelPos = fragWorldPos - povPos;
const float totalDist = length(povPosToPixelPos);
const bool goingRight = povPosToPixelPos.x > 0.0f;
const bool goingUp = povPosToPixelPos.y > 0.0f;
const vec2 relativePositions[5] = {
	vec2(inPovRelativePositions[0][0], inPovRelativePositions[0][1]),
	vec2(inPovRelativePositions[1][0], inPovRelativePositions[1][1]),
	vec2(inPovRelativePositions[2][0], inPovRelativePositions[2][1]),
	vec2(inPovRelativePositions[3][0], inPovRelativePositions[3][1]),
	vec2(inPovRelativePositions[0][3], inPovRelativePositions[1][3])
};
const int relativePosTileIndices[5] = {
	int(inPovRelativePositions[0][2]), 
	int(inPovRelativePositions[1][2]), 
	int(inPovRelativePositions[2][2]), 
	int(inPovRelativePositions[3][2]),
	int(inPovRelativePositions[2][3])
};
const vec4 BUILDING_COLORS[] = {
	vec4(0, 0, 0, 0), // NONE
	vec4(0, 1, 0, 1), // PRODUCER
	vec4(1, 0, 0, 1), // CONSUMER
};

// Used for navigating throught the tile map:
int currentTileIndex			= initialTileIndex;
int currentInitialSideIndex		= initialSideIndex;
int currentInitialVertIndex = initialTexCoordIndex;
int currentSideOffset			= initialSideOffset;
int connectionIndex;
int drawSideIndex;
// Used for incrementing the ray:
vec2 runningDist;
vec2 stepDist;
float currentDist = 0;
// Coord relative to bottom left of the fragWorldPos's drawTile:
vec2 fragDrawTilePos; // Lies in domain { (x, y) | 0 <= x <= 1, 0 <= y <= 1 }.
vec2 vertOrigin;
vec2 vertA;
vec2 vertB;

// connectionIndex is the side index of the side of the current tile we are passing over.
// drawTileSideIndex is the side index of that same side but from the perspective of screen space.
void adjustCurrentTileIndex(int connectionIndex, int drawTileSideIndex) {

	int sideIndexOfConnectedTile = tileInfos[currentTileIndex].neighborSideIndex[connectionIndex];
	if (tileInfos[currentTileIndex].neighborMirrored[connectionIndex] == TRUE) {
		currentSideOffset = (currentSideOffset + 2) % 4;
	}
	currentInitialSideIndex = (sideIndexOfConnectedTile + currentSideOffset * VERT_INFO_OFFSETS[drawTileSideIndex]) % 4;
	currentTileIndex = tileInfos[currentTileIndex].neighborIndices[connectionIndex];

	currentInitialVertIndex = currentInitialSideIndex;
	// Side indices and vertex indices are not the same if the winding of the tile is counterclockwise.
	// When that is the case, the vertex index will have to be offset a little:
	if (currentSideOffset == 3) {
		currentInitialVertIndex = (currentInitialVertIndex+ 1) % 4;
	}
}

void getFragDrawTilePos() {
	// scaleUV will tell us how much to alias between the bottom left uv coord the the top left/bottom left uv coords.
	// As each tile is 1 x 1, the difference between them will always act also as a way to scale between coord information.
	fragDrawTilePos = abs(fragWorldPos) - floor(abs(fragWorldPos));
	// We flipped (abs) the coordinates to get in a domain of [0, 1] for negative fragWorldPositions, so now we flip them back:
	if (fragWorldPos.x < 0) { fragDrawTilePos.x = 1 - fragDrawTilePos.x; }
	if (fragWorldPos.y < 0) { fragDrawTilePos.y = 1 - fragDrawTilePos.y; }
}

float pointToLineSegDist(vec2 A, vec2 B, vec2 E) {
	vec2 AB = B - A;
	vec2 AE = E - A;
	vec2 BE = E - B;

    if (dot(AB, BE) > 0) { 
		return length(BE); 
	}
    else if (dot(AB, AE) < 0) {
		return length(AE); 
	}
	return abs(AB.x * AE.y - AB.y * AE.x) / length(AB);
}

vec4 tileTexColor() {
	vec2 bottomRightUV, bottomLeftUV, topLeftUV, texCoord;

	// currentInitialVertIndex takes us to the side info of the top right vertex as seen from screen space.
	bottomRightUV = tileInfos[currentTileIndex].texCoords[(currentInitialVertIndex + currentSideOffset) % 4];
	bottomLeftUV  = tileInfos[currentTileIndex].texCoords[(currentInitialVertIndex + currentSideOffset * 2) % 4];
	topLeftUV     = tileInfos[currentTileIndex].texCoords[(currentInitialVertIndex + currentSideOffset * 3) % 4];


	texCoord = bottomLeftUV 
			 + fragDrawTilePos.x * (bottomRightUV - bottomLeftUV)
			 + fragDrawTilePos.y * (topLeftUV - bottomLeftUV);
	
	return texture(inTexture, texCoord);
}

float WEIGHT = 0.5;

void getRelativeVertPositions() {
	int inverseOffset = (currentSideOffset + 2) % 4; // 1 -> 3 and 3 -> 1

	vertOrigin = DRAW_TILE_COORDS[((inverseOffset * currentInitialVertIndex) + (2 * currentSideOffset)) % 4];
	vertA = (DRAW_TILE_COORDS[((inverseOffset * currentInitialVertIndex) + currentSideOffset) % 4]) - vertOrigin;					
	vertB = (DRAW_TILE_COORDS[((inverseOffset * currentInitialVertIndex) + (3 * currentSideOffset)) % 4]) - vertOrigin;
}

bool isInsidePlayer() {
	bool inside = false;
	int numRelativeInfoIndices = 0;
	vec2 scalars[4];

	// figure out if the tile we are drawing is even one of the tiles the player is in:
	for (int i = 0; i < 4; i++) {
		if (currentTileIndex == relativePosTileIndices[i]) {
			inside = true;
			scalars[numRelativeInfoIndices++] = relativePositions[i];
		}
	}
	if (!inside) { return false; }

	vec2 a1 = vertA * scalars[0].x;
	vec2 b1 = vertB * scalars[0].y;

	vec2 drawTilePlayerPos =  vertOrigin + a1 + b1;

	if (length(fragDrawTilePos - drawTilePlayerPos) < 0.25) {
		return true;
	} 
	// There are cases where the tile has more than one copy of player data.
	// It is possible to 'see' the same tile from 2 edges, or the tile you are on in a corner!
	if (numRelativeInfoIndices == 2) {
		a1 = vertA * scalars[1].x;
		b1 = vertB * scalars[1].y;

		drawTilePlayerPos =  vertOrigin + a1 + b1;

		if (length(fragDrawTilePos - drawTilePlayerPos) < 0.25) {
			return true;
		}
	}
	return false;
}

bool isInsideEntity() {
	// check current tile for entity:
	if (IS_OBJECT[tileInfos[currentTileIndex].entityType]) {
		int inverseOffset = (currentSideOffset + 2) % 4; // 1 -> 3 and 3 -> 1
		vec2 entityVert = DRAW_TILE_COORDS[((inverseOffset * currentInitialVertIndex) 
						+ (tileInfos[currentTileIndex].entityDirection * currentSideOffset)) % 4];
		vec2 entityVertOther = DRAW_TILE_COORDS[((inverseOffset * currentInitialVertIndex) 
						+ ((tileInfos[currentTileIndex].entityDirection + 3) * currentSideOffset)) % 4];

		vec2 direction = entityVert - entityVertOther;
		vec2 entityPos = vec2(0.5f, 0.5f) + direction * tileInfos[currentTileIndex].entityOffset;

		if (length(fragDrawTilePos - entityPos) < 0.25f) {
			return true;
		}
	}

	// check adjacent tiles for entities:
	for (int i = 0; i < 4; i++) {
		int ithIndex = (currentInitialSideIndex + i * currentSideOffset) % 4;
		int neighborTileIndex = tileInfos[currentTileIndex].neighborIndices[ithIndex];
		int neighborSideIndex = tileInfos[currentTileIndex].neighborSideIndex[ithIndex];

		if (tileInfos[neighborTileIndex].entityType == NONE ||
			tileInfos[neighborTileIndex].entityDirection != neighborSideIndex) {
			continue;
		}

		vec2 entityPos = vec2(0.5f, 0.5f) + ADJACENT_ENTITY_OFFSETS[i] * -(tileInfos[neighborTileIndex].entityOffset - 1.0f);
		if (length(fragDrawTilePos - entityPos) < 0.5f) {
			return true;
		}
	}

	return false;
}

void setup() {
	stepDist = totalDist / abs(povPosToPixelPos);

	if (goingRight) { runningDist.x = (1.0f - povPos.x) * stepDist.x; } 
	else			{ runningDist.x = povPos.x * stepDist.x; }
	
	if (goingUp) { runningDist.y = (1.0f - povPos.y) * stepDist.y; } 
	else		 { runningDist.y = povPos.y * stepDist.y; }
}

void findTile() {

	int stepCount = 0;
	while (stepCount < MAX_STEPS) {

		if (runningDist.x < runningDist.y) {

			currentDist = runningDist.x;
			if (currentDist > totalDist) {
				break; // We have arrived!
			}
			runningDist.x += stepDist.x;

			// Shift tile target left/right depending on goingRight:
			if (goingRight) {
				connectionIndex = currentInitialSideIndex;
				drawSideIndex = 0;
			} 
			else { // Going left:
				connectionIndex = (currentInitialSideIndex + currentSideOffset * 2) % 4;
				drawSideIndex = 2;
			}
		} 
		else { // runningDist.x > runningDist.y

			currentDist = runningDist.y;
			if (currentDist > totalDist) {
				break; // We have arrived!
			}
			runningDist.y += stepDist.y;

			// Shift tile target up/down depending on goingUp:
			if (goingUp) {
				connectionIndex = (currentInitialSideIndex + currentSideOffset * 3) % 4;
				drawSideIndex = 3;
			} 
			else { // Going down:
				connectionIndex = (currentInitialSideIndex + currentSideOffset * 1) % 4;
				drawSideIndex = 1;
			}
		}
		adjustCurrentTileIndex(connectionIndex, drawSideIndex);
		stepCount++;
	}
}

bool isInsideBuilding() {
	switch(tileInfos[currentTileIndex].entityType) {
	case(FORCE_BLOCK):
		// Find the vertices of the arrow:
		vec2 A, B, C;
		switch(tileInfos[currentTileIndex].entityOrientation) {
		case(0):
			A = vertOrigin + vertB;
			B = vertOrigin + vertA + vertB / 2.0f;
			C = vertOrigin;
			break;
		case(1):
			A = vertOrigin + vertA + vertB;
			B = vertOrigin + vertA / 2.0f;
			C = vertOrigin + vertB;
			break;
		case(2):
			A = vertOrigin + vertA;
			B = vertOrigin + vertB / 2.0f;
			C = vertOrigin + vertA + vertB;
			break;
		case(3):
			A = vertOrigin;
			B = vertOrigin + vertA / 2.0f + vertB;
			C = vertOrigin + vertA;
			break;
		}

		if (pointToLineSegDist(A, B, fragDrawTilePos) < 0.1f ||
			pointToLineSegDist(B, C, fragDrawTilePos) < 0.1f) {
			gl_FragColor = vec4(0.937, 0.737, 0.608, 1);
			return true;
		} 
		break;
	}

	switch(tileInfos[currentTileIndex].basisType) {
		case(PRODUCER):
			if (pointToLineSegDist(vec2(0.5, 0), vec2(0.5, 1), fragDrawTilePos) < 0.1f ||
				pointToLineSegDist(vec2(0, 0.5), vec2(1, 0.5), fragDrawTilePos) < 0.1f) {
				gl_FragColor = vec4(0.69, 0.773, 0.643, 1);
				return true;
			} 
			break;
		case(CONSUMER):
			if (pointToLineSegDist(vec2(0, 0), vec2(1, 1), fragDrawTilePos) < 0.1f ||
				pointToLineSegDist(vec2(0, 1), vec2(1, 0), fragDrawTilePos) < 0.1f) {
				gl_FragColor = vec4(0.827, 0.463, 0.463, 1);
				return true;
			}
			break;
		}

		return false;
}

void colorPixel() {
	// Rendering heierarchy (first prio to last prio): 
	//     player > entity > basis > force
	
	bool insideBasis    = false;
	bool insidePlayer   = false;
	bool insideEntity   = false;
	bool insideBuilding = false;

	if (isInsidePlayer()) {
		insidePlayer = true;
		gl_FragColor = vec4(0.984, 0.953, 0.835, 1);
	} 
	else if (isInsideEntity()) {
		insideEntity = true;
		gl_FragColor = vec4(0.612, 0.686, 0.667, 1);
	} 
	else {
		insideBuilding = isInsideBuilding();
	}

	// Texture of the underlying pixel:
	if (!insidePlayer && !insideEntity && !insideBuilding) {
		gl_FragColor = mix(tileTexColor(), tileInfos[currentTileIndex].color, 0.5);
	}

	if (tileInfos[currentTileIndex].hasForce == TRUE) {
		gl_FragColor.b = (cos(2 * PI * fragDrawTilePos.y - float(numFrames) / 10.0f) + 1) / 2.0f;
	}
}

void main() {
	// Quick check to see if we are in the initial tile.  If so we dont have to bother raycasting:
	if (fragWorldPos.x > 0 && fragWorldPos.x < 1 && fragWorldPos.y > 0 && fragWorldPos.y < 1) {
		getFragDrawTilePos();
		getRelativeVertPositions();
		colorPixel();
		return;
	}

	setup();
	findTile();
	getFragDrawTilePos();
	getRelativeVertPositions();
	colorPixel();
};