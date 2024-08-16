#version 430 core // We need 430 for SSBOs.

#define PEEK_OBSTRUCTION_MAPS

#define PI 3.1415926535897932384626433832795

#define TRUE  1
#define FALSE 0

// Entity IDs:
#define NONE 0
#define OMNI 1

// Basis IDs:
#define EMPTY			0
#define PRODUCER		1
#define CONSUMER		2
#define FORCE_SINK		3
#define FORCE_GENERATOR	4

// Corner Building IDs:
#define BELT_MIDDLE_FORWARD  1
#define BELT_MIDDLE_BACKWARD 2
#define BELT_END_FORWARD     3
#define BELT_END_BACKWARD    4

#define MAX_STEPS 1000 // <- Protects against infinite loops.

// Lazy defines:
#define CurrentTile tileInfos[currentTileIndex]
#define CurrentTileMirrored currentSideOffset==3
#define CurrentEntity(i) entityInfos[CurrentTile.entityIndices[i]]



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

struct TileInfo {
	vec2 texCoords[4];
	int neighborIndices[4];
	int neighborMirrored[4];
	int neighborSideIndex[4];
	vec4 color;
	int entityIndices[9]; // 36
	int basisType;
	int basisOrientation;
	int tileSubType;
	int obstructionMask;
	int padding[3];
};

struct EntityInfo {
	vec4 color;
	int localOrientation[2];
	int localDirection[2];
	int position[2];
	int type;
	int padding;
};

layout (std430, binding = 1) buffer tileInfosBuffer   { TileInfo tileInfos[];     };
layout (std430, binding = 2) buffer entityInfosBuffer { EntityInfo entityInfos[]; };



// USEFUL FUNCTIONS NOT RELATED TO THIS SHADER IN PARTICULAR:

float signedDistanceBox(vec2 point, vec2 box) {
    vec2 d = abs(point) - box;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
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
vec2 rotate(vec2 v, float angle) {
    return mat2(cos(angle), -sin(angle), sin(angle), cos(angle)) * v;
}
vec4 rgb(float r, float g, float b) { return vec4(r/256,g/256,b/256,1); }



// GLOBAL CONST VARIABLES:

const int  VERT_INFO_OFFSETS[]       = { 2,1,0,3 };
const vec2 DRAW_TILE_COORDS[]        = { vec2(1, 1), vec2(1, 0), vec2(0, 0), vec2(0, 1) };
const bool IS_OBJECT[]               = { false, true, true, true, false, false, false, false, };
const vec2 ADJACENT_ENTITY_OFFSETS[] = { vec2(1, 0), vec2(0, -1), vec2(-1, 0), vec2(0, 1) };
const int ORIENTATION_TO_ORIENTATION_MAP[6][6][4][4] = { // Key: [Current Tile Type][Destination Tile Type][Exiting side][Current Orientation]
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
const vec2 LOCAL_POS_INDEX_TO_VEC2[] = { 
	vec2(1.0f,  0.5f), vec2(0.5f, 0.0f ), vec2(0.0f,  0.5f), vec2(0.5f, 1.0f ), 
	vec2(0.75f, 0.5f), vec2(0.5f, 0.25f), vec2(0.25f, 0.5f), vec2(0.5f, 0.75f), 
	vec2(0.5f,  0.5f) 
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



// GLOBAL VARIABLES:

// Used for navigating throught the tile map:
int currentTileIndex		= initialTileIndex;
int currentInitialSideIndex	= initialSideIndex;
int currentInitialVertIndex = initialTexCoordIndex;
int currentSideOffset		= initialSideOffset;

// Coord relative to bottom left of the fragWorldPos's drawTile:
vec2 fragDrawTilePos; // Lies in domain { (x, y) | 0 <= x <= 1, 0 <= y <= 1 }.
vec2 fragLocalBasisPos; // Gives position if side 0 was x+ and side 3 was y+. Domain 0->1 for x and y.
vec2 vertOrigin;
vec2 vertA;
vec2 vertB;

// Returns the adjusted relative orientation of an entity/basis when going from one tile to another.
int orientationToOrientationMap(int currentTileType, int neighborTileType, int exitingSide, int currentOrientation) {
	return ORIENTATION_TO_ORIENTATION_MAP[currentTileType][neighborTileType][exitingSide][currentOrientation];
}

const uint coordToObstructionMask[2][4] = { { 0x8888, 0x4444, 0x2222, 0x1111 }, { 0x000f, 0x00f0, 0x0f00, 0xf000, } };
uint vec2ToObstructionMask(vec2 v) {
	uint xFlag = coordToObstructionMask[0][int(v.x * 4)];
	uint yFlag = coordToObstructionMask[1][int(v.y * 4)];
	return xFlag & yFlag;
}
bool fragInObstructionMask() {
	uint posFlag = vec2ToObstructionMask(fragLocalBasisPos);
	if ((posFlag & CurrentTile.obstructionMask) != 0) { return true; }
	return false;
}



// Early Declarations for lower orginization:
bool tryDrawForceBlock(vec2 entityForward, int tempOrientationOverride);



// Returns the coordinates of the fragment local to its tile.
// Domain is 0-1 for x and y and is relative to the screen.  (x+ == right, y+ == up).
void getFragDrawTilePos() {
	// scaleUV will tell us how much to alias between the bottom left uv coord the the top left/bottom left uv coords.
	// As each tile is 1 x 1, the difference between them will always act also as a way to scale between coord information.
	fragDrawTilePos = abs(fragWorldPos) - floor(abs(fragWorldPos));
	// We flipped (abs) the coordinates to get in a domain of [0, 1] for negative fragWorldPositions, so now we flip them back:
	if (fragWorldPos.x < 0) { fragDrawTilePos.x = 1 - fragDrawTilePos.x; }
	if (fragWorldPos.y < 0) { fragDrawTilePos.y = 1 - fragDrawTilePos.y; }
}
// returns the relative position if the fragment inside its tile.
// Position is relative to the sides of the tile: x+ == side 0, y+ == side 3.
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
		case 2: fragLocalBasisPos = center + rotate(fragDrawTilePos - center, PI); break;
		default: /* case 3: */ fragLocalBasisPos = center + rotate(fragDrawTilePos - center,  -PI/2.0f);
		}
	}
	else {
		switch(currentInitialSideIndex) {
		case 0: fragLocalBasisPos = center + rotate(fragDrawTilePos.yx - center, PI/2.0f); break;
		case 1: fragLocalBasisPos = center + rotate(fragDrawTilePos.yx - center, PI); break;
		case 2: fragLocalBasisPos = center + rotate(fragDrawTilePos.yx - center, -PI/2.0f); break;
		default: /* case 3: */ fragLocalBasisPos = fragDrawTilePos.yx;
		}
	}
}



// Returns true if the fragment is inside the player.
// Right now the player is a simple circle, but that should change!
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

bool colorEntity(int position) {
	vec2 entityCenter = LOCAL_POS_INDEX_TO_VEC2[position];
	if (max(abs(fragLocalBasisPos.x - entityCenter.x), abs(fragLocalBasisPos.y - entityCenter.y)) < 0.25) {
		gl_FragColor = CurrentEntity(position).color;
		return true;
	}
	return false;
}
bool isInsideEntity() {
	for (int i = 0; i<9; i++) {
		if (CurrentTile.entityIndices[i] != -1) {
			if (colorEntity(i)) {
				return true;
			}
		}
	}
	return false;
}


// Given some info, will find and return the color of a fragment inside a force block.
bool drawForceGenerator(int orientation) {
	vec2 arrowPoint = vec2(1, 0.5);
	vec2 arrowArm1  = vec2(0, 1);
	vec2 arrowArm2  = vec2(0, 0);
	vec2 center = vec2(0.5, 0.5);

	float inv = 1;
	if (currentSideOffset == 3) {
		inv = -1;
	}

	float angle = (currentInitialSideIndex + CurrentTile.basisOrientation) * (inv * PI / 2.0f);
	arrowPoint = center + (rotate(arrowPoint - center, angle));
	arrowArm1 =  center + (rotate(arrowArm1 - center, angle));
	arrowArm2 =  center + (rotate(arrowArm2 - center, angle));
	

	if (pointToLineSegDist(arrowArm1, arrowPoint, fragDrawTilePos) < 0.1f ||
		pointToLineSegDist(arrowPoint, arrowArm2, fragDrawTilePos) < 0.1f) {
		gl_FragColor = vec4(0.937, 0.737, 0.608, 1);
		return true;
	} else {
		gl_FragColor = vec4(0.612, 0.686, 0.667, 1);
		return true;
	}
	return false;


//	// Find the vertices of the arrow:
//	vec2 A, B, C;
//	switch(tempOrientationOverride) {
//	case(0):
//		A = vertOrigin + vertB;
//		B = vertOrigin + vertA + vertB/2.0f;
//		C = vertOrigin;
//		break;
//	case(1):
//		A = vertOrigin + vertA + vertB;
//		B = vertOrigin + vertA/2.0f;
//		C = vertOrigin + vertB;
//		break;
//	case(2):
//		A = vertOrigin + vertA;
//		B = vertOrigin + vertB/2.0f;
//		C = vertOrigin + vertA + vertB;
//		break;
//	case(3):
//		A = vertOrigin;
//		B = vertOrigin + vertA/2.0f + vertB;
//		C = vertOrigin + vertA;
//		break;
//	}
//	A += entityForward;
//	B += entityForward;
//	C += entityForward;

	
}
// Colors the fragment if it is inside a basis.  Returns true if inside one.
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

		case FORCE_GENERATOR: drawForceGenerator(CurrentTile.basisOrientation); return true;
		}

		return false;
}

float S(float x, float o) { 
	return abs(PI * mod(abs((x - 2.0 * o) / PI), 1.0) - (PI / 2.0)) - (PI / 2.0);
}
float T(float x, float y, float o) { 
	return 0.5 * (-S(x, o) + sqrt(1.0 + 2.0*y - y*y - sqrt(1.0f + 4.0f*y))); 
}
bool isInSquare(vec2 p, vec2 o, float a, float s) {
	// Translate the pixel coordinates to the origin of the square
    vec2 translated = p - o;
    
    // Rotate the coordinates back to align with the square's axes
    float cosAngle = cos(-a);
    float sinAngle = sin(-a);
    vec2 rotated = vec2(
        translated.x * cosAngle - translated.y * sinAngle,
        translated.x * sinAngle + translated.y * cosAngle);
    
    // Check if the coordinates are within the bounds of the square
    return abs(rotated.x) <= s && abs(rotated.y) <= s;
}

#define BELT_SPEED deltaTime * 3

bool drawBeltMiddle(int cornerBuildingIndex, bool forward) {
	const vec4 DARK = rgb(54, 41, 56);
	const vec4 M1 = rgb(83, 61, 79);
	const vec4 M2 = rgb(110, 80, 97);
	const vec4 M3 = rgb(138, 100, 109);
	const vec4 M4 = rgb(160, 126, 124);
	const vec4 LIGHT = rgb(180, 161, 151);

	vec2 p;
	switch(cornerBuildingIndex) {
	case 0: p = rotate(fragLocalBasisPos - vec2(1, 0), PI / 2.0f)*vec2(-1, 1) + vec2(1, 0); break;
	case 1: p = -(fragLocalBasisPos - vec2(1, 0)); break;
	case 2: p = rotate(fragLocalBasisPos, -PI / 2.0); break;
	case 3: p = fragLocalBasisPos - vec2(0, 1); break;
	}

	if (p.y > 0.25) {
		return false;
	}

	float a;
	if (forward) { a = BELT_SPEED; }
	else { a = -BELT_SPEED; }

	// Thin outer belt:
	if (p.y > 2.0f / (3.0f * PI)) { 
		gl_FragColor = DARK; 
		return true;
	}

	// Axles:
	vec2 o = DRAW_TILE_COORDS[cornerBuildingIndex] - DRAW_TILE_COORDS[(cornerBuildingIndex+3)%4];
	float r = 0.0883883476483;
	if (distance(p, vec2(0,0)) < r || distance(p, o/2.0) < r || distance(p, o) < r) {
		gl_FragColor = DARK;
		return true; 
	}

	// Squares:
	if (isInSquare(p, vec2(0,0), a + (PI/4.0f), 2.0f/(3.0f*PI*sqrt(2)) ) ||
		isInSquare(p, o/2.0,     a,             2.0f/(3.0f*PI*sqrt(2)) ) ||
		isInSquare(p, o,         a + (PI/4.0f), 2.0f/(3.0f*PI*sqrt(2)) ) ) {
		gl_FragColor = M3; 
		return true; 
	}

	// Humps:
	float X = -3.0f*PI*p.x;
	float Y = -3.0f*PI*p.y + 2.0f;
	float t = T(X, Y, a);
	if (Y < sin(2.0f * t) - sin(t) - cos(t) + 1) { 
		gl_FragColor = LIGHT; 
		return true; 
	}

	// Back Squares:
	if (isInSquare(p, vec2(0,0), a,             2.0f/(3.0f*PI*sqrt(2)) ) ||
		isInSquare(p, o/2.0,     a + (PI/4.0f), 2.0f/(3.0f*PI*sqrt(2)) ) ||
		isInSquare(p, o,         a,             2.0f/(3.0f*PI*sqrt(2)) ) ) {
		gl_FragColor = M2; 
		return true; 
	}

	// Back Humps:
	a += PI / 4.0f;
	t = T(X, Y, a);
	if (Y < sin(2.0f * t) - sin(t) - cos(t) + 1) { 
		gl_FragColor = M4; 
		return true; 
	}

	gl_FragColor = M1; 
	return true;
}
bool drawBeltEnd(int cornerBuildingIndex, bool forward) {
	vec2 center;
	switch(cornerBuildingIndex) {
	case 0: center = vec2(1, 1); break;
	case 1: center = vec2(1, 0); break;
	case 2: center = vec2(0, 0); break;
	case 3: center = vec2(0, 1); break;
	}

	if (distance(fragLocalBasisPos, center) > 0.25) { return false; }

	// Belt:
	if (distance(fragLocalBasisPos, center) > 2.0f / (3.0f * PI)) { 
		gl_FragColor = rgb(54, 41, 56);
		return true; 
	}

	float a;
	if (forward) { a = BELT_SPEED; }
	else { a = -BELT_SPEED; }
	
	// Axle:
	if (distance(fragLocalBasisPos, center) < 0.0883883476483) {
		gl_FragColor = rgb(54, 41, 56);
		return true;
	}

	// Square:
	if (isInSquare(fragLocalBasisPos, center, a + PI / 4.0, 2.0f/(3.0f*PI*sqrt(2)) ) ) {
		gl_FragColor = rgb(138, 100, 109);
		return true;
	}
	
	// Humps:
		gl_FragColor = rgb(180, 161, 151);
		return true;
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
// Starting at the center of the screen (where the player is), will find the tile that the fragment resides in.
// This is necessary as a ray must be cast from the origin and move through tiles as it travels, since there is 
// no simple matrix transformation given this implimentation.
// uses the digital differential analyser (DDA) algorithm.
void findTile() {
	vec2 runningDist;
	vec2 stepDist = totalDist / abs(povPosToPixelPos);
	int stepCount = 0;
	int connectionIndex;
	int drawSideIndex;
	// Used for incrementing the ray:
	float currentDist = 0;

	// Setup:
	if (goingRight) { 
		runningDist.x = (1.0f - povPos.x) * stepDist.x; 
	} 
	else {
		runningDist.x = povPos.x * stepDist.x;
	}
	if (goingUp) { 
		runningDist.y = (1.0f - povPos.y) * stepDist.y; 
	} 
	else { 
		runningDist.y = povPos.y * stepDist.y; 
	}

	// Raycast:
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



// Given a local draw tile coordinate, will return the color of the base texture of a tile.
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
void colorPixel() {
	// Rendering heierarchy (player 'drawn last'): 
	// player > entity > basis > force

	#ifdef PEEK_OBSTRUCTION_MAPS
	if (fragInObstructionMask() && sin(128 * fragLocalBasisPos.x) > 0) { gl_FragColor = vec4(1, 0, 0, 1); return; }
	#endif

	if (isInsidePlayer()) { gl_FragColor = vec4(0.984, 0.953, 0.835, 1); }
	//else if (isInsideEntity()) {} 
	//else if (isInsideBasis()) {}
	else { gl_FragColor = mix(tileTexColor(), tileInfos[currentTileIndex].color, 0.5); }
}




void main() {
	// Quick check to see if we are in the initial tile.  If so, we don't have to bother raycasting:
	if (fragWorldPos.x > 0 && fragWorldPos.x < 1 && fragWorldPos.y > 0 && fragWorldPos.y < 1) {
		getFragDrawTilePos();
		getRelativeVertPositions();
		colorPixel();
		return;
	}

	findTile();
	getFragDrawTilePos();
	getRelativeVertPositions();
	colorPixel();
};