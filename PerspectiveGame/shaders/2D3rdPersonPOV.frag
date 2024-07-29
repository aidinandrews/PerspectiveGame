#version 430 core // We need 430 for SSBOs.

#define PI				3.1415926535897932384626433832795

// Entity IDs:
#define NONE			0
#define OMNI			1
#define MATERIAL_A		2
#define MATERIAL_B		3
#define COMPRESSOR		4
#define FORCE_BLOCK		5
#define FORCE_MIRROR	6

// Basis IDs:
#define EMPTY			0
#define PRODUCER		1
#define CONSUMER		2
#define FORCE_SINK		3

#define TRUE			1
#define FALSE			0

#define RIGHT			0
#define DOWN			1
#define LEFT			2
#define UP				3

#define MAX_STEPS 1000 // <- Protects against infinite loops.

#define CurrentTileMirrored currentSideOffset==3

layout(location = 0) in vec2 fragWorldPos;
layout(location = 1) in vec2 povPos;

uniform float     deltaTime;
uniform float     updateProgress;
uniform int       initialTileIndex;
uniform int       initialSideIndex;
uniform int       initialTexCoordIndex;
uniform int       initialSideOffset;
uniform sampler2D inTexture;
uniform mat4      inPovRelativePositions;

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

	int entityEdge0Index;
	int entityEdge1Index;
	int entityEdge2Index;
	int entityEdge3Index;
	int entityEdge4Index;
	int entityEdge5Index;
	int entityEdge6Index;
	int entityEdge7Index;
	int entityEdge8Index;

	int type; // 3 bits (6 options)
	
	// alignas is a bastard and scales the struct size by the largest alignas(n) n value for some reason.  
	// This means we need a buffer in here so the stride length is consistent between OpenGl's definition 
	// and the shader's struct.
	float bufferSpace[2]; 
};

layout (std430, binding = 1) buffer tileInfosBuffer { 
	tileInfo tileInfos[]; 
};

const int  VERT_INFO_OFFSETS[]       = { 2,1,0,3 };
const vec2 DRAW_TILE_COORDS[]        = { vec2(1, 1), vec2(1, 0), vec2(0, 0), vec2(0, 1) };
const bool IS_OBJECT[]               = { false, true, true, true, false, false, false, false, };
const vec2 ADJACENT_ENTITY_OFFSETS[] = { vec2(1, 0), vec2(0, -1), vec2(-1, 0), vec2(0, 1) };
// Key: [Current Tile Type][Destination Tile Type][Exiting side][Current Orientation]
const int ORIENTATION_TO_ORIENTATION_MAP[6][6][4][4] = {
#define X -1
	{ // Current Tile: XYF
		{ { 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 1, 2, 3 } }, // Destination Tile: XYF
		{ { 2, 1, 0, 3 }, { 0, 3, 2, 1 }, { 2, 1, 0, 3 }, { 0, 3, 2, 1 } }, // Destination Tile: XYB
		{ { X, X, X, X }, { 3, 0, 1, 2 }, { X, X, X, X }, { 3, 0, 1, 2 } }, // Destination Tile: XZF
		{ { X, X, X, X }, { 3, 2, 1, 0 }, { X, X, X, X }, { 3, 2, 1, 0 } }, // Destination Tile: XZB
		{ { 1, 2, 3, 0 }, { X, X, X, X }, { 1, 2, 3, 0 }, { X, X, X, X } }, // Destination Tile: YZF
		{ { 3, 2, 1, 0 }, { X, X, X, X }, { 3, 2, 1, 0 }, { X, X, X, X } }  // Destination Tile: YZB
	},																						   
	{ // Current Tile: XYB																	   
		{ { 2, 1, 0, 3 }, { 0, 3, 2, 1 }, { 2, 1, 0, 3 }, { 0, 3, 2, 1 } }, // Destination Tile: XYF
		{ { 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 1, 2, 3 } },	// Destination Tile: XYB
		{ { X, X, X, X }, { 3, 2, 1, 0 }, { X, X, X, X }, { 3, 2, 1, 0 } },	// Destination Tile: XZF
		{ { X, X, X, X }, { 3, 0, 1, 2 }, { X, X, X, X }, { 3, 0, 1, 2 } },	// Destination Tile: XZB
		{ { 3, 2, 1, 0 }, { X, X, X, X }, { 3, 2, 1, 0 }, { X, X, X, X } },	// Destination Tile: YZF
		{ { 1, 2, 3, 0 }, { X, X, X, X }, { 1, 2, 3, 0 }, { X, X, X, X } }	// Destination Tile: YZB
	},						
	{ // Current Tile: XZF	
		{ { 1, 2, 3, 0 }, { X, X, X, X }, { 1, 2, 3, 0 }, { X, X, X, X } }, // Destination Tile: XYF
		{ { 3, 2, 1, 0 }, { X, X, X, X }, { 3, 2, 1, 0 }, { X, X, X, X } },	// Destination Tile: XYB
		{ { 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 1, 2, 3 } },	// Destination Tile: XZF
		{ { 2, 1, 0, 3 }, { 0, 3, 2, 1 }, { 2, 1, 0, 3 }, { 0, 3, 2, 1 } },	// Destination Tile: XZB
		{ { X, X, X, X }, { 3, 0, 1, 2 }, { X, X, X, X }, { 3, 0, 1, 2 } },	// Destination Tile: YZF
		{ { X, X, X, X }, { 3, 2, 1, 0 }, { X, X, X, X }, { 3, 2, 1, 0 } }	// Destination Tile: YZB
	},																						   
	{ // Current Tile: XZB																	   
		{ { 3, 2, 1, 0 }, { X, X, X, X }, { 3, 2, 1, 0 }, { X, X, X, X } }, // Destination Tile: XYF
		{ { 1, 2, 3, 0 }, { X, X, X, X }, { 1, 2, 3, 0 }, { X, X, X, X } },	// Destination Tile: XYB
		{ { 2, 1, 0, 3 }, { 0, 3, 2, 1 }, { 2, 1, 0, 3 }, { 0, 3, 2, 1 } },	// Destination Tile: XZF
		{ { 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 1, 2, 3 } },	// Destination Tile: XZB
		{ { X, X, X, X }, { 3, 2, 1, 0 }, { X, X, X, X }, { 3, 2, 1, 0 } },	// Destination Tile: YZF
		{ { X, X, X, X }, { 3, 0, 1, 2 }, { X, X, X, X }, { 3, 0, 1, 2 } }	// Destination Tile: YZB
	},																						   
	{ // Current Tile: YZF																	   
		{ { X, X, X, X }, { 3, 0, 1, 2 }, { X, X, X, X }, { 3, 0, 1, 2 } }, // Destination Tile: XYF
		{ { X, X, X, X }, { 3, 2, 1, 0 }, { X, X, X, X }, { 3, 2, 1, 0 } },	// Destination Tile: XYB
		{ { 1, 2, 3, 0 }, { X, X, X, X }, { 1, 2, 3, 0 }, { X, X, X, X } },	// Destination Tile: XZF
		{ { 3, 2, 1, 0 }, { X, X, X, X }, { 3, 2, 1, 0 }, { X, X, X, X } },	// Destination Tile: XZB
		{ { 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 1, 2, 3 } },	// Destination Tile: YZF
		{ { 2, 1, 0, 3 }, { 0, 3, 2, 1 }, { 2, 1, 0, 3 }, { 0, 3, 2, 1 } }	// Destination Tile: YZB
	},																						   
	{ // Current Tile: YZB																	   
		{ { X, X, X, X }, { 3, 2, 1, 0 }, { X, X, X, X }, { 3, 2, 1, 0 } }, // Destination Tile: XYF
		{ { X, X, X, X }, { 3, 0, 1, 2 }, { X, X, X, X }, { 3, 0, 1, 2 } },	// Destination Tile: XYB
		{ { 3, 2, 1, 0 }, { X, X, X, X }, { 3, 2, 1, 0 }, { X, X, X, X } },	// Destination Tile: XZF
		{ { 1, 2, 3, 0 }, { X, X, X, X }, { 1, 2, 3, 0 }, { X, X, X, X } },	// Destination Tile: XZB
		{ { 2, 1, 0, 3 }, { 0, 3, 2, 1 }, { 2, 1, 0, 3 }, { 0, 3, 2, 1 } },	// Destination Tile: YZF
		{ { 1, 2, 3, 4 }, { 1, 2, 3, 4 }, { 1, 2, 3, 4 }, { 1, 2, 3, 4 } }	// Destination Tile: YZB
	}
#undef X
};

const vec2  povPosToPixelPos = fragWorldPos - povPos;
const float totalDist = length(povPosToPixelPos);
const bool  goingRight = povPosToPixelPos.x > 0.0f;
const bool  goingUp = povPosToPixelPos.y > 0.0f;

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

// Used for navigating throught the tile map:
int currentTileIndex		= initialTileIndex;
int currentInitialSideIndex	= initialSideIndex;
int currentInitialVertIndex = initialTexCoordIndex;
int currentSideOffset		= initialSideOffset;
int connectionIndex;
int drawSideIndex;
// Used for incrementing the ray:
vec2 runningDist;
vec2 stepDist;
float currentDist = 0;
// Coord relative to bottom left of the fragWorldPos's drawTile:
vec2 fragDrawTilePos; // Lies in domain { (x, y) | 0 <= x <= 1, 0 <= y <= 1 }.
vec2 fragLocalBasisPos; // Gives position if side 0 was x+ and side 3 was y+. Domain 0->1 for x and y.
vec2 vertOrigin;
vec2 vertA;
vec2 vertB;

float entityOffset = updateProgress;

// Returns the adjusted relative orientation of an entity/basis when going from one tile to another.
int orientationToOrientationMap(int currentTileType, int neighborTileType, int exitingSide, int currentOrientation) {
	return ORIENTATION_TO_ORIENTATION_MAP[currentTileType][neighborTileType][exitingSide][currentOrientation];
}

float signedDistanceBox(vec2 point, vec2 box) {
    vec2 d = abs(point) - box;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

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

// gives a 2D rotation matrix given an angle:
mat2 rotation2D(float angle) {
    float s = sin(angle);
    float c = cos(angle);
    return mat2(c, -s, s, c);
}

// rotates a 2D vector by an angle:
vec2 rotate(vec2 v, float angle) {
    return rotation2D(angle) * v;
}

void getRelativeVertPositions() {
	int inverseOffset = (currentSideOffset + 2) % 4; // 1 -> 3 and 3 -> 1

	vertOrigin = DRAW_TILE_COORDS[((inverseOffset * currentInitialVertIndex) + 2) % 4];
	vertA = (DRAW_TILE_COORDS[((inverseOffset * currentInitialVertIndex) + currentSideOffset) % 4]) - vertOrigin;					
	vertB = (DRAW_TILE_COORDS[((inverseOffset * currentInitialVertIndex) + (3 * currentSideOffset)) % 4]) - vertOrigin;

	vec2 center = vec2(0.5,0.5);
	if (!(CurrentTileMirrored)) {
		switch(currentInitialSideIndex) {
		case 0: fragLocalBasisPos = fragDrawTilePos; break;
		case 1: fragLocalBasisPos = center + rotate(fragDrawTilePos - center, PI/2.0f); break;
		case 2: fragLocalBasisPos = center + rotate(fragDrawTilePos - center, PI);      break;
		case 3: fragLocalBasisPos = center + rotate(fragDrawTilePos - center,  -PI/2.0f); break;
		}
	}
	else {
		switch(currentInitialSideIndex) {
		case 0: fragLocalBasisPos = center + rotate(fragDrawTilePos.yx - center, PI/2.0f); break;
		case 1: fragLocalBasisPos = center + rotate(fragDrawTilePos.yx - center, PI); break;
		case 2: fragLocalBasisPos = center + rotate(fragDrawTilePos.yx - center, -PI/2.0f);      break;
		case 3: fragLocalBasisPos = fragDrawTilePos.yx; break;
		}
	}
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

bool tryDrawForceBlock(vec2 entityForward, int tileIndex, int tempOrientationOverride) {
	// Find the vertices of the arrow:
	vec2 A, B, C;
	//switch(tileInfos[tileIndex].entityOrientation) {
	switch(tempOrientationOverride) {
	case(0):
		A = vertOrigin + vertB;
		B = vertOrigin + vertA + vertB/2.0f;
		C = vertOrigin;
		break;
	case(1):
		A = vertOrigin + vertA + vertB;
		B = vertOrigin + vertA/2.0f;
		C = vertOrigin + vertB;
		break;
	case(2):
		A = vertOrigin + vertA;
		B = vertOrigin + vertB/2.0f;
		C = vertOrigin + vertA + vertB;
		break;
	case(3):
		A = vertOrigin;
		B = vertOrigin + vertA/2.0f + vertB;
		C = vertOrigin + vertA;
		break;
	}
	A += entityForward;
	B += entityForward;
	C += entityForward;

	if (pointToLineSegDist(A, B, fragDrawTilePos) < 0.1f ||
		pointToLineSegDist(B, C, fragDrawTilePos) < 0.1f) {
		gl_FragColor = vec4(0.937, 0.737, 0.608, 1);
		return true;
	} else {
		gl_FragColor = vec4(0.612, 0.686, 0.667, 1);
		return true;
	}
	return false;
}

// REFACTOR THIS BULLSHIT:
bool isInsideEntity() {
//	float thisEntityOffset = (entityOffset - 1) * tileInfos[currentTileIndex].entityMoving;
//
//	int inverseOffset = (currentSideOffset + 2) % 4; // 1 -> 3 and 3 -> 1
//	vec2 entityVert = DRAW_TILE_COORDS[((currentInitialVertIndex * inverseOffset) 
//					+ (tileInfos[currentTileIndex].entityDirection * currentSideOffset)) 
//					% 4];	
//
//	vec2 entityVertOther = DRAW_TILE_COORDS[((inverseOffset * currentInitialVertIndex) 
//							+ ((tileInfos[currentTileIndex].entityDirection + 3) * currentSideOffset)) 
//							% 4];
//
//	vec2 entityForward = entityVert - entityVertOther;
//	vec2 entityPos = vec2(0.5f, 0.5f) + entityForward * thisEntityOffset;
//
//	// Since the max size of an entity is the size of a tile, if we are too far away from the entityPos, we must be outside of the entity itself:
//	if(abs(fragDrawTilePos.x - entityPos.x) < 0.5f && abs(fragDrawTilePos.y - entityPos.y) < 0.5f){
//		
//		switch(tileInfos[currentTileIndex].entityType) {
//		case(MATERIAL_A):
//			if (abs(fragDrawTilePos.x - entityPos.x) < 0.25f && abs(fragDrawTilePos.y - entityPos.y) < 0.25f) {
//				gl_FragColor = vec4(0.945, 0.937, 0.6, 1);
//				return true;
//			}
//			break;
//
//		case(FORCE_BLOCK):
//			return tryDrawForceBlock(entityForward * thisEntityOffset, currentTileIndex, tileInfos[currentTileIndex].entityOrientation);
//		}
//
//	}
//
//	// check adjacent tiles for entities:
//	for (int i = 0; i < 4; i++) {
//		int ithIndex = (currentInitialSideIndex + i * currentSideOffset) % 4;
//		int neighborTileIndex = tileInfos[currentTileIndex].neighborIndices[ithIndex];
//		int neighborSideIndex = tileInfos[currentTileIndex].neighborSideIndex[ithIndex];
//		float neighborEntityOffset = (-entityOffset + 1) * tileInfos[neighborTileIndex].entityMoving;
//
//		entityPos = vec2(0.5f, 0.5f) + ADJACENT_ENTITY_OFFSETS[i] * -(neighborEntityOffset - 1.0f);
//		entityForward = ADJACENT_ENTITY_OFFSETS[i] * -(neighborEntityOffset - 1.0f);
//
//		// Check we should even try to draw anything:
//		bool neighborEmpty = tileInfos[neighborTileIndex].entityType == NONE;
//		bool neighborEntityGoingAway = ((tileInfos[neighborTileIndex].entityDirection + 2) % 4) != neighborSideIndex;
//		bool neighborEntityTooFarAway = abs(fragDrawTilePos.x - entityPos.x) > 0.5f 
//									 || abs(fragDrawTilePos.y - entityPos.y) > 0.5f;
//
//		if (neighborEmpty || neighborEntityGoingAway || neighborEntityTooFarAway) {
//			continue;
//		}
//		
//		switch(tileInfos[neighborTileIndex].entityType) {
//		case(MATERIAL_A):
//			if (abs(fragDrawTilePos.x - entityPos.x) < 0.25f && abs(fragDrawTilePos.y - entityPos.y) < 0.25f) {
//				gl_FragColor = vec4(0.945, 0.937, 0.6, 1);
//				return true;
//			}
//			return false;
//
//		case(FORCE_BLOCK):
//			int trueNeighborOrientation = ORIENTATION_TO_ORIENTATION_MAP
//				[tileInfos[neighborTileIndex].type]
//				[tileInfos[currentTileIndex].type]
//				[neighborSideIndex]
//				[tileInfos[neighborTileIndex].entityOrientation];
//
//			return tryDrawForceBlock(entityForward, neighborTileIndex, trueNeighborOrientation);
//		}
//	}

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

bool isInsideBasis() {

	switch(tileInfos[currentTileIndex].basisType) {
		case PRODUCER:
			if (pointToLineSegDist(vec2(0.5, 0), vec2(0.5, 1), fragDrawTilePos) < 0.1f ||
				pointToLineSegDist(vec2(0, 0.5), vec2(1, 0.5), fragDrawTilePos) < 0.1f) {
				gl_FragColor = vec4(0.69, 0.773, 0.643, 1);
				return true;
			} 
			break;
		case CONSUMER:
			if (pointToLineSegDist(vec2(0, 0), vec2(1, 1), fragDrawTilePos) < 0.1f ||
				pointToLineSegDist(vec2(0, 1), vec2(1, 0), fragDrawTilePos) < 0.1f) {
				gl_FragColor = vec4(0.827, 0.463, 0.463, 1);
				return true;
			}
			break;
		case FORCE_SINK:

			if (pointToLineSegDist(vec2(0.05, 0.05), vec2(0.05, 0.95), fragDrawTilePos) < 0.1f ||
				pointToLineSegDist(vec2(0.05, 0.05), vec2(0.95, 0.05), fragDrawTilePos) < 0.1f ||
				pointToLineSegDist(vec2(0.95, 0.95), vec2(0.05, 0.95), fragDrawTilePos) < 0.1f ||
				pointToLineSegDist(vec2(0.95, 0.95), vec2(0.95, 0.05), fragDrawTilePos) < 0.1f) {
				gl_FragColor = vec4(0.34, 0.44, 0.45, 1);
				return true;
			}
			else {
				float dist = signedDistanceBox(fragDrawTilePos - vec2(0.5, 0.5), vec2(0.25, 0.25));
				vec4 dark = vec4(0.34, 0.44, 0.45, 1);
				vec4 light = vec4(0.49, 0.62, 0.61, 1);
				float amount = dist / 0.5f;
				//amount = sqrt(amount);
				gl_FragColor = mix(light, dark, amount);
				return true;

				gl_FragColor = vec4(0.49, 0.62, 0.61, 1);
				return true;
			}
			break;
		}

		return false;
}

void colorPixel() {
	// Rendering heierarchy (player 'drawn last'): 
	// player > entity > basis > force
	
	bool insideBasis  = false;
	bool insidePlayer = false;
	bool insideEntity = false;
	float forceWeight = 0.5f;
	vec4 forceColor;

	if (isInsidePlayer()) {
		insidePlayer = true;
		gl_FragColor = vec4(0.984, 0.953, 0.835, 1);
		return;
	} 
	else if (isInsideEntity()) {
		insideEntity = true;
		return;
	} 
	else if(isInsideBasis()) {
		insideBasis = true;
	}
	else /* We are just on the tile somewhere, so key into the texture: */ {
		gl_FragColor = mix(tileTexColor(), tileInfos[currentTileIndex].color, 0.5);
		forceWeight = 0.5f;
	}

	// Apply an effect if the tile has a force applied over it:
	forceColor = vec4(1,1,1,1);
	if (tileInfos[currentTileIndex].hasForce == TRUE) {
		float X = 0, Y = 0;
		switch(tileInfos[currentTileIndex].forceDirection) {
		case 0: X =  dot(vertA, fragDrawTilePos);     Y =  dot(vertB, fragDrawTilePos); break;
		case 1: X = -dot(vertB, fragDrawTilePos) + 1; Y =  dot(vertA, fragDrawTilePos); break;
		case 2: X = -dot(vertA, fragDrawTilePos) + 1; Y =  dot(vertB, fragDrawTilePos); break;
		case 3: X =  dot(vertB, fragDrawTilePos);     Y =  dot(vertA, fragDrawTilePos); break;
		}

		forceWeight *= (cos(2*PI*(X + .5*abs(abs(Y) - 0.5)) - deltaTime * 4) + 1) / 2.0f;
		float offset = (cos(8 * (2*PI*(X + .5*abs(abs(Y) - 0.5)) - deltaTime * 4)) + 1) / 8.0f;
		gl_FragColor = mix(gl_FragColor, forceColor, forceWeight + offset);
	}

	gl_FragColor.r = distance(fragLocalBasisPos, fragDrawTilePos);
	gl_FragColor.r = fragLocalBasisPos.x;
}

void main() {
	// Quick check to see if we are in the initial tile.  If so, we don't have to bother raycasting:
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