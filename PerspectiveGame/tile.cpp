#include "tile.h"
#include "building.h"

Tile::Tile(TileType type, glm::ivec3 position) : type(type), position(position)
{
	texCoords[0] = glm::vec2(1, 1);
	texCoords[1] = glm::vec2(1, 0);
	texCoords[2] = glm::vec2(0, 0);
	texCoords[3] = glm::vec2(0, 1);

	for (int i = 0; i < 4; i++) {
		cornerSafety[i] = true;
	}

	for (int i = 0; i < 9; i++) {
		entityIndices[i] = -1;
		entityInfoIndices[i] = -1;
	}

}

glm::ivec3 Tile::getNormal()
{
	switch (type) {
	case TILE_TYPE_XYF: return glm::ivec3(00, 00, +1);
	case TILE_TYPE_XYB: return glm::ivec3(00, 00, -1);
	case TILE_TYPE_XZF: return glm::ivec3(00, +1, 00);
	case TILE_TYPE_XZB: return glm::ivec3(00, -1, 00);
	case TILE_TYPE_YZF: return glm::ivec3(+1, 00, 00);
	case TILE_TYPE_YZB: return glm::ivec3(-1, 00, 00);
	default: throw std::runtime_error("Tile sub type is ERROR on getNormal()!");
	}
}

glm::ivec3 Tile::getVertPos(int index)
{
	using namespace tnav;

	if (index == 0) { return position; }

	switch (getSuperTileType(type)) {
	case TILE_TYPE_XY:
		switch (index) {
		case 1: return position - glm::ivec3(0, 1, 0);
		case 2: return position - glm::ivec3(1, 1, 0);
		case 3: return position - glm::ivec3(1, 0, 0);
		default: throw std::runtime_error("getVertPos() index out of scope!");
		}
	case TILE_TYPE_XZ:
		switch (index) {
		case 1: return position - glm::ivec3(1, 0, 0);
		case 2: return position - glm::ivec3(1, 0, 1);
		case 3: return position - glm::ivec3(0, 0, 1);
		default: throw std::runtime_error("getVertPos() index out of scope!");
		}
	case TILE_TYPE_YZ:
		switch (index) {
		case 1: return position - glm::ivec3(0, 0, 1);
		case 2: return position - glm::ivec3(0, 1, 1);
		case 3: return position - glm::ivec3(0, 1, 0);
		default: throw std::runtime_error("getVertPos() index out of scope!");
		}
	default: throw std::runtime_error("getVertPos() is switching based on faulty tileSubType values!");
	}
}

GPU_TileInfo::GPU_TileInfo(Tile* tile)
{
	color = glm::vec4(tile->color, 1);

	basisType = (int)tile->basis.type;
	basisOrientation = tile->basis.localOrientation;

	getTileType = (int)tile->type;

	for (int i = 0; i < 4; i++) {
		neighborIndices[i] = tile->getNeighbor(LocalDirection(i))->index;
		//neighborMirrored[i] = (int)tile->isNeighborConnectionsMirrored[i];
		//neighborSideIndex[i] = tile->neighborConnectedSideIndices[i];
// TODO: fix gpu-info/shaders to use the new alignment mappings
		neighborMirrored[i] = (int)tile->is1stDegreeNeighborMirrored(i);
		neighborSideIndex[i] = tnav::getMappedAlignment(tile->getNeighborAlignmentMap(LocalDirection(i)), tnav::oppositeAlignment(LocalAlignment(i)));
		texCoords[i] = tile->texCoords[i];
		cornerSafety[i] = tile->cornerSafety[i];
	}

	// Entity info is UNORDERED by position in the gpu info to save space.  There can only ever be 4
	// entities in a tile at once after all, so why store 9 ints when you can store 4 instead?
	int numEntitiesInTile = 0;
	for (int i = 0; i < 9; i++) {
		if (tile->entityIndices[i] == -1) { continue; }
		entityIndices[numEntitiesInTile] = tile->entityIndices[i];
		entityInfoIndices[numEntitiesInTile] = tile->entityInfoIndices[i];
		numEntitiesInTile++;
	}
	for (int i = numEntitiesInTile; i < 4; i++) {
		entityIndices[i] = -1;
		entityInfoIndices[i] = -1;
	}
}