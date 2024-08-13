#include "tile.h"
#include "building.h"

//Tile::Tile() {
//	index = 0;
//	type = TILE_TYPE_XY_FRONT;
//	color = glm::vec3(1, 1, 1);
//	sibling = nullptr;
//	this->maxVert = glm::ivec3(0, 0, 0);
//	forceLocalDirection = LOCAL_DIRECTION_INVALID;
//
//	for (int i = 0; i < 9; i++) {
//		entityIndices[i] = -1;
//	}
//}

Tile::Tile(TileSubType tileSubType, glm::ivec3 maxVert) : type(tileSubType), maxVert(maxVert) {

	sideInfos.texCoords[0] = glm::vec2(1, 1);
	sideInfos.texCoords[1] = glm::vec2(1, 0);
	sideInfos.texCoords[2] = glm::vec2(0, 0);
	sideInfos.texCoords[3] = glm::vec2(0, 1);

	for (int i = 0; i < 9; i++) {
		entityIndices[i] = -1;
		entityInfoIndices[i] = 0;
	}

	obstructionMask = 0;

	forceLocalDirection = LOCAL_DIRECTION_INVALID;

	for (int i = 0; i < 4; i++) { cornerBuildings[i] = CORNER_BUILDING_NONE; }
}

glm::ivec3 Tile::normal() {
	switch (type) {
	case TILE_TYPE_XY_FRONT: 
		return glm::ivec3(0, 0, 1);
	case TILE_TYPE_XY_BACK: 
		return glm::ivec3(0, 0, -1);
	case TILE_TYPE_XZ_FRONT: 
		return glm::ivec3(0, 1, 0);
	case TILE_TYPE_XZ_BACK: 
		return glm::ivec3(0, -1, 0);
	case TILE_TYPE_YZ_FRONT: 
		return glm::ivec3(1, 0, 0);
	default /*TILE_TYPE_YZ_BACK:*/:
		return glm::ivec3(-1, 0, 0);
	}
}

inline void initNeighborsToNull(Tile *neighbors[4][3]) {
	for (int edge = 0; edge < 4; edge++) {
		for (int angle = 0; angle < 3; angle++) {
			neighbors[edge][angle] = nullptr;
		}
	}
}

glm::ivec3 Tile::getVertPos(int index) {
	if (index == 0) {
		return maxVert;
	}

	switch (superTileType(type)) {
	case TILE_TYPE_XY:
		switch (index) {
		case 1: return maxVert - glm::ivec3(0, 1, 0);
		case 2: return maxVert - glm::ivec3(1, 1, 0);
		case 3: return maxVert - glm::ivec3(1, 0, 0);
		default: 
			std::cout << "getVertPos() index out of scope! index: " <<index<< std::endl; 
			return glm::ivec3(0, 0, 0);
		}
	case TILE_TYPE_XZ:
		switch (index) {
		case 1: return maxVert - glm::ivec3(1, 0, 0);
		case 2: return maxVert - glm::ivec3(1, 0, 1);
		case 3: return maxVert - glm::ivec3(0, 0, 1);
		default: 
			std::cout << "getVertPos() index out of scope! index: " << index << std::endl;
			return glm::ivec3(0, 0, 0);
		}
	case TILE_TYPE_YZ:
		switch (index) {
		case 1: return maxVert - glm::ivec3(0, 0, 1);
		case 2: return maxVert - glm::ivec3(0, 1, 1);
		case 3: return maxVert - glm::ivec3(0, 1, 0);
		default: 
			std::cout << "getVertPos() index out of scope! index: " << index << std::endl;
			return glm::ivec3(0, 0, 0);
		}
	default:
		std::cout << "getVertPos() is switching based on faulty tileSubType values!" << std::endl;
		return glm::ivec3(0, 0, 0);
	}
}

Tile::Tile::Type Tile::getTileType(glm::ivec3 A, glm::ivec3 B, glm::ivec3 C, glm::ivec3 D) {
	if (A.z == B.z && A.z == C.z && A.z == D.z) {
		return Tile::TILE_TYPE_XY;
	} else if (A.y == B.y && A.y == C.y && A.y == D.y) {
		return TILE_TYPE_XZ;
	} else /*(A.x == B.x && A.x == C.x && A.x == D.x)*/ {
		return TILE_TYPE_YZ;
	}
}

glm::ivec3 Tile::getMaxVert(glm::ivec3 A, glm::ivec3 B, glm::ivec3 C, glm::ivec3 D) {
	return glm::ivec3(std::max(std::max(std::max(A.x, B.x), C.x), D.x),
					  std::max(std::max(std::max(A.y, B.y), C.y), D.y),
					  std::max(std::max(std::max(A.z, B.z), C.z), D.z));
}

Tile::Tile::Type Tile::superTileType(TileSubType subType) {
	switch (subType) {
	case TILE_TYPE_XY_FRONT: 
		return TILE_TYPE_XY;
	case TILE_TYPE_XY_BACK: 
		return TILE_TYPE_XY;
	case TILE_TYPE_XZ_FRONT: 
		return TILE_TYPE_XZ;
	case TILE_TYPE_XZ_BACK: 
		return TILE_TYPE_XZ;
	case TILE_TYPE_YZ_FRONT: 
		return TILE_TYPE_YZ;
	default: /*TILE_TYPE_YZ_BACK*/
		return TILE_TYPE_YZ;
	}
}

TileSubType Tile::tileSubType(Tile::Type tileType, bool isFront) {
	switch (tileType) {
	case TILE_TYPE_XY:
		if (isFront) 
			return TILE_TYPE_XY_FRONT;
		else 
			return TILE_TYPE_XY_BACK;

	case TILE_TYPE_XZ:
		if (isFront) 
			return TILE_TYPE_XZ_FRONT;
		else 
			return TILE_TYPE_XZ_BACK;

	default: /*TILE_TYPE_YZ:*/
		if (isFront) 
			return TILE_TYPE_YZ_FRONT;
		else 
			return TILE_TYPE_YZ_BACK;
	}
}

TileSubType Tile::inverseTileType(TileSubType type) {
	switch (type) {
	case TILE_TYPE_XY_FRONT: return TILE_TYPE_XY_BACK;
	case TILE_TYPE_XY_BACK: return TILE_TYPE_XY_FRONT;
	case TILE_TYPE_XZ_FRONT: return TILE_TYPE_XZ_BACK;
	case TILE_TYPE_XZ_BACK: return TILE_TYPE_XZ_FRONT;
	case TILE_TYPE_YZ_FRONT: return TILE_TYPE_YZ_BACK;
	default: return TILE_TYPE_YZ_FRONT; // This is either TILE_TYPE_YZ_BACK or you and/or I fucked up.	
	}
}

Tile::TileRelation Tile::getRelation(TileSubType A, TileSubType B) {
	if (A == B) { return TILE_RELATION_FLAT; }
	
	switch (A) {
	case TILE_TYPE_XY_FRONT:
		switch (B) {
		case TILE_TYPE_XY_BACK: return TILE_RELATION_FLIPPED;
		}
	}
	
	return Tile::TileRelation();
}

TileGpuInfo::TileGpuInfo(Tile *tile) {
	color = glm::vec4(tile->color, 1);

	basisType = (int)tile->basis.type;
	basisOrientation = tile->basis.localOrientation;

	hasForce = (int)tile->hasForce();
	forceDirection = tile->forceLocalDirection;

	for (int i = 0; i < 9; i++) {
		entityIndices[i] = tile->entityIndices[i];
	}

	tileSubType = (int)tile->type;

	for (int i = 0; i < 4; i++) {
		neighborIndices[i] = tile->sideInfos.connectedTiles[i]->index;
		neighborMirrored[i] = (int)tile->sideInfos.connectionsMirrored[i];
		neighborSideIndex[i] = tile->sideInfos.connectedSideIndices[i];

		texCoords[i] = tile->sideInfos.texCoords[i];

		cornerBuildingTypes[i] = (int)tile->cornerBuildings[i];
	}

	obstructionMask = tile->obstructionMask;
}