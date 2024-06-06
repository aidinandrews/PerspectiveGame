#include "tile.h"
#include "building.h"

Tile::Tile() {
	index = 0;
	type = TILE_TYPE_XY_FRONT;
	color = glm::vec3(1, 1, 1);
	sibling = nullptr;
	this->maxVert = glm::ivec3(0, 0, 0);

	/*buildingType = BUILDING_TYPE_NONE;
	buildingOrientation = Tile::Edge::UP;
	entityType = ENTITY_TYPE_NONE;
	entityOffsetSide = Tile::Edge::UP;
	entityOffset = 0.0f;*/
}

Tile::Tile(Tile::SubType tileSubType, glm::ivec3 maxVert) : type(tileSubType), maxVert(maxVert) {

	glm::vec2 A(1, 1);
	glm::vec2 B(1, 0);
	glm::vec2 C(0, 0);
	glm::vec2 D(0, 1);

	// Generate verts:
	switch (superTileType(tileSubType)) {
	case TILE_TYPE_XY:
		sideInfos.texCoords[0] = A;
		sideInfos.texCoords[1] = B;
		sideInfos.texCoords[2] = C;
		sideInfos.texCoords[3] = D;
		break;
	case TILE_TYPE_XZ:
		sideInfos.texCoords[0] = D;
		sideInfos.texCoords[1] = C;
		sideInfos.texCoords[2] = B;
		sideInfos.texCoords[3] = A;
		break;
	default: /*TILE_TYPE_YZ*/
		sideInfos.texCoords[0] = A;
		sideInfos.texCoords[1] = B;
		sideInfos.texCoords[2] = C;
		sideInfos.texCoords[3] = D;
	}

	/*buildingType = BUILDING_TYPE_NONE;
	buildingOrientation = Tile::Edge::UP;
	entityType = ENTITY_TYPE_NONE;
	entityOffsetSide = Tile::Edge::UP;
	entityOffset = 0.0f;*/
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
			std::cout << "getVertPos() index out of scope!" << std::endl; 
			return glm::ivec3(0, 0, 0);
		}
	case TILE_TYPE_XZ:
		switch (index) {
		case 1: return maxVert - glm::ivec3(1, 0, 0);
		case 2: return maxVert - glm::ivec3(1, 0, 1);
		case 3: return maxVert - glm::ivec3(0, 0, 1);
		default: 
			std::cout << "getVertPos() index out of scope!" << std::endl; 
			return glm::ivec3(0, 0, 0);
		}
	case TILE_TYPE_YZ:
		switch (index) {
		case 1: return maxVert - glm::ivec3(0, 0, 1);
		case 2: return maxVert - glm::ivec3(0, 1, 1);
		case 3: return maxVert - glm::ivec3(0, 1, 0);
		default: 
			std::cout << "getVertPos() index out of scope!" << std::endl; 
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

float Tile::getVelocity() {
	// See https://www.desmos.com/calculator/bnp7kfmsgv for function.  
	// Built such that a velocity greater than 1 tile/tick is impossible.
#define MAX_OFFSET 5
	if (force.magnitude < entity.mass) {
		return (force.magnitude / (2.0f * entity.mass)) * DeltaTime * MAX_OFFSET;
	}
	else {
		return (1.0f - entity.mass / (2.0f * force.magnitude)) * DeltaTime * MAX_OFFSET;
	}
#undef MAX_OFFSET
}

Tile::Tile::Type Tile::superTileType(Tile::SubType subType) {
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

Tile::SubType Tile::tileSubType(Tile::Type tileType, bool isFront) {
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

Tile::SubType Tile::inverseTileType(Tile::SubType type) {
	switch (type) {
	case TILE_TYPE_XY_FRONT: return TILE_TYPE_XY_BACK;
	case TILE_TYPE_XY_BACK: return TILE_TYPE_XY_FRONT;
	case TILE_TYPE_XZ_FRONT: return TILE_TYPE_XZ_BACK;
	case TILE_TYPE_XZ_BACK: return TILE_TYPE_XZ_FRONT;
	case TILE_TYPE_YZ_FRONT: return TILE_TYPE_YZ_BACK;
	default: return TILE_TYPE_YZ_FRONT; // This is either TILE_TYPE_YZ_BACK or you and/or I fucked up.	
	}
}

Tile::TileRelation Tile::getRelation(Tile::SubType A, Tile::SubType B) {
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
	basisOrientation = tile->basis.orientation;

	hasForce = (int)tile->force.magnitude > 0;
	forceDirection = tile->force.direction;

	entityType = (int)tile->entity.type;
	entityOffset = tile->entity.offset;
	entityDirection = tile->entity.direction;
	entityOrientation = tile->entity.orientation;

	for (int i = 0; i < 4; i++) {
		neighborIndices[i] = tile->sideInfos.connectedTiles[i]->index;
		neighborMirrored[i] = (int)tile->sideInfos.connectionsMirrored[i];
		neighborSideIndex[i] = tile->sideInfos.connectedSideIndices[i];
		texCoords[i] = tile->sideInfos.texCoords[i];
	}
}