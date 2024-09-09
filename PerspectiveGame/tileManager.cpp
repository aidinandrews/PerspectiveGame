#include "tileManager.h"

const float TileManager::DRAW_TILE_OPACITY_DECRIMENT_STEP = 0.1f;

const int TileManager::VERT_INFO_OFFSETS[] = { 2,1,0,3 };

const int VERT_INFO_OFFSETS_MIRRORED_EYE_TILE[] = { 1,0,3,2 };

const glm::vec2 TileManager::DRAW_TILE_OFFSETS[] = {
			glm::vec2(1, 0), glm::vec2(0, -1), glm::vec2(-1, 0), glm::vec2(0, 1)
};

const glm::vec2 TileManager::INITIAL_FRUSTUM[] = {
	glm::vec2(0,0),glm::vec2(0,0),glm::vec2(0,0),
};

std::vector<glm::vec2> TileManager::INITIAL_DRAW_TILE_VERTS = {
	   glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0), glm::vec2(0, 1),
};

TileManager::~TileManager() {
	// Make sure to free all the allocated tiles:
	for (Tile* p : tiles) {
		delete p;
	}
}

bool TileManager::tileIsUnique(Tile& newTile) {
	for (Tile* t : tiles) {
		// Because the only way to make a 
		// 
		// tile is by giving it's max point, all tiles 
		// with the same vertices will have those vertices at the same indices.  I.e. it 
		// is impossible for two tiles that share all verices to share those vertices in a 
		// differing order, so all that is needed to check if the tile is the same is to 
		// check if they are the same type and have a position in common at the same index!
		if (t->type == newTile.type && t->position == newTile.position) {
			return false;
		}
	}
	return true;
}

bool TileManager::createTilePair(SuperTileType tileType, glm::ivec3 maxPoint,
	glm::vec3 frontTileColor, glm::vec3 backTileColor) {
	Tile* foreTile = new Tile(Tile::getTileType(tileType, true), maxPoint);
	Tile* backTile = new Tile(Tile::getTileType(tileType, false), maxPoint);

	// before connecting everything up, it is importand that this new tile pair 
	// does not overlap another tile pair, as that would be against the rules:
	if (!tileIsUnique(*foreTile)) {
		return false;
	}

	foreTile->sibling = backTile;
	backTile->sibling = foreTile;

	foreTile->color = frontTileColor;
	backTile->color = backTileColor;

	switch (tileType) {
	case TILE_TYPE_XY:
		foreTile->type = TILE_TYPE_XYF;
		backTile->type = TILE_TYPE_XYB;
		break;
	case TILE_TYPE_XZ:
		foreTile->type = TILE_TYPE_XZF;
		backTile->type = TILE_TYPE_XZB;
		break;
	case TILE_TYPE_YZ:
		foreTile->type = TILE_TYPE_YZF;
		backTile->type = TILE_TYPE_YZB;
		break;
	}

	for (int i = 0; i < 4; i++) {
		// As we have not yet checked if there are other tiles connected to this tile pair, 
		// it can only be known that these two tiles see each other.  This will be changed 
		// if other tiles are connected and are seen to be the visible connection:
		foreTile->neighborTilePtrs[i] = backTile;
		backTile->neighborTilePtrs[i] = foreTile;
		// When drawing and shuffling items between tiles, it is important to know 
		// what sides are connected so that the tile sides can be properly indexed:
		//frontTile->neighborConnectedSideIndices[i] = i;
		//backTile->neighborConnectedSideIndices[i] = i;
		// When on the edge of one tile and looking around to the face of the glued 
		// tile, it would seem that the other tile has been 'flipped up.'  This makes 
		// it look like the tile has been mirrored and so when drawing the 2D 3rd person 
		// POV, it is important to know that we go counter clockwise around the vertices 
		// instead of clockwise, as would be proper for other types of tile connections.
		//frontTile->isNeighborConnectionsMirrored[i] = true;
		//backTile->isNeighborConnectionsMirrored[i] = true;

		foreTile->neighborAlignmentMapIndex[i] = tnav::getNeighborAlignmentMapIndex(LocalAlignment(i), LocalAlignment(i));
		backTile->neighborAlignmentMapIndex[i] = tnav::getNeighborAlignmentMapIndex(LocalAlignment(i), LocalAlignment(i));
	}

	// Connect up the new tiles to the other ones in the scene:
	connectUpNewTile(foreTile);
	connectUpNewTile(backTile);

	updateCornerSafety(foreTile);
	updateCornerSafety(backTile);
	for (int i = 0; i < 4; i++) {
		Tile* neighbor1 = foreTile->getNeighbor(LocalDirection(i));
		int mapIndex = foreTile->neighborAlignmentMapIndex[i];
		Tile* neighbor1a = neighbor1->getNeighbor(tnav::getMappedAlignment(mapIndex, LocalDirection((i + 1) % 4)));
		Tile* neighbor1b = neighbor1->getNeighbor(tnav::getMappedAlignment(mapIndex, LocalDirection((i + 3) % 4)));
		updateCornerSafety(neighbor1);
		updateCornerSafety(neighbor1a);
		updateCornerSafety(neighbor1b);

		neighbor1 = backTile->getNeighbor(LocalDirection(i));
		mapIndex = backTile->neighborAlignmentMapIndex[i];
		neighbor1a = neighbor1->getNeighbor(tnav::getMappedAlignment(mapIndex, LocalDirection((i + 1) % 4)));
		neighbor1b = neighbor1->getNeighbor(tnav::getMappedAlignment(mapIndex, LocalDirection((i + 3) % 4)));
		updateCornerSafety(neighbor1);
		updateCornerSafety(neighbor1a);
		updateCornerSafety(neighbor1b);
	}

	// Finally, we can add them to the list!
	foreTile->index = (int)tiles.size();
	backTile->index = (int)tiles.size() + 1;

	tiles.push_back(foreTile);
	tiles.push_back(backTile);

	tileGpuInfos.push_back(GPU_TileInfo(foreTile));
	tileGpuInfos.push_back(GPU_TileInfo(backTile));

	updateTileGpuInfos();

	return true;
}

void TileManager::connectUpNewTile(Tile* subjectTile) {
	// Each side of a tile can only even theoredically connect to some types/orientations of tile,
	// so each edge (connectableTiles[X][]) gets a list of the types of tiles it can connect to 
	// (connectableTiles[][X]).
	TileType connectableTilesSubType[4][3];
	glm::ivec3 connectableTilesMaxPoint[4][3]; // tile.sideInfos[0].pos is always the max vert.
	const int SIDE_A = 0, SIDE_B = 1, SIDE_C = 2, SIDE_D = 3;
	glm::ivec3 subjectTileMaxPoint = subjectTile->position;
	
	// Big honkin' list of all the possible tiles that can connect to this tile, depending on what tile sub 
	// type this new tile is.  Index into it to get the exact position that the connected tile's 1st vertex 
	// (max vertex) must have to actually be connected to it.
	switch (subjectTile->type) {
	case TileType::TILE_TYPE_XYF:
		connectableTilesSubType[SIDE_A][0] = TileType::TILE_TYPE_XYF;
		connectableTilesSubType[SIDE_A][1] = TileType::TILE_TYPE_YZF;
		connectableTilesSubType[SIDE_A][2] = TileType::TILE_TYPE_YZB;
		connectableTilesMaxPoint[SIDE_A][0] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);
		connectableTilesMaxPoint[SIDE_A][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		connectableTilesMaxPoint[SIDE_A][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);

		connectableTilesSubType[SIDE_B][0] = TileType::TILE_TYPE_XYF;
		connectableTilesSubType[SIDE_B][1] = TileType::TILE_TYPE_XZF;
		connectableTilesSubType[SIDE_B][2] = TileType::TILE_TYPE_XZB;
		connectableTilesMaxPoint[SIDE_B][0] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);
		connectableTilesMaxPoint[SIDE_B][1] = subjectTileMaxPoint + glm::ivec3(0, -1, 1);
		connectableTilesMaxPoint[SIDE_B][2] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);

		connectableTilesSubType[SIDE_C][0] = TileType::TILE_TYPE_XYF;
		connectableTilesSubType[SIDE_C][1] = TileType::TILE_TYPE_YZF;
		connectableTilesSubType[SIDE_C][2] = TileType::TILE_TYPE_YZB;
		connectableTilesMaxPoint[SIDE_C][0] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);
		connectableTilesMaxPoint[SIDE_C][1] = subjectTileMaxPoint + glm::ivec3(-1, 0, 1);
		connectableTilesMaxPoint[SIDE_C][2] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);

		connectableTilesSubType[SIDE_D][0] = TileType::TILE_TYPE_XYF;
		connectableTilesSubType[SIDE_D][1] = TileType::TILE_TYPE_XZF;
		connectableTilesSubType[SIDE_D][2] = TileType::TILE_TYPE_XZB;
		connectableTilesMaxPoint[SIDE_D][0] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);
		connectableTilesMaxPoint[SIDE_D][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		connectableTilesMaxPoint[SIDE_D][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);
		break;
	case TileType::TILE_TYPE_XYB:
		connectableTilesSubType[SIDE_A][0] = TileType::TILE_TYPE_XYB;
		connectableTilesSubType[SIDE_A][1] = TileType::TILE_TYPE_YZF;
		connectableTilesSubType[SIDE_A][2] = TileType::TILE_TYPE_YZB;
		connectableTilesMaxPoint[SIDE_A][0] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);
		connectableTilesMaxPoint[SIDE_A][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);
		connectableTilesMaxPoint[SIDE_A][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);

		connectableTilesSubType[SIDE_B][0] = TileType::TILE_TYPE_XYB;
		connectableTilesSubType[SIDE_B][1] = TileType::TILE_TYPE_XZF;
		connectableTilesSubType[SIDE_B][2] = TileType::TILE_TYPE_XZB;
		connectableTilesMaxPoint[SIDE_B][0] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);
		connectableTilesMaxPoint[SIDE_B][1] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);
		connectableTilesMaxPoint[SIDE_B][2] = subjectTileMaxPoint + glm::ivec3(0, -1, 1);

		connectableTilesSubType[SIDE_C][0] = TileType::TILE_TYPE_XYB;
		connectableTilesSubType[SIDE_C][1] = TileType::TILE_TYPE_YZF;
		connectableTilesSubType[SIDE_C][2] = TileType::TILE_TYPE_YZB;
		connectableTilesMaxPoint[SIDE_C][0] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);
		connectableTilesMaxPoint[SIDE_C][1] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);
		connectableTilesMaxPoint[SIDE_C][2] = subjectTileMaxPoint + glm::ivec3(-1, 0, 1);

		connectableTilesSubType[SIDE_D][0] = TileType::TILE_TYPE_XYB;
		connectableTilesSubType[SIDE_D][1] = TileType::TILE_TYPE_XZF;
		connectableTilesSubType[SIDE_D][2] = TileType::TILE_TYPE_XZB;
		connectableTilesMaxPoint[SIDE_D][0] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);
		connectableTilesMaxPoint[SIDE_D][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);
		connectableTilesMaxPoint[SIDE_D][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		break;
	case TileType::TILE_TYPE_XZF:
		connectableTilesSubType[SIDE_A][0] = TileType::TILE_TYPE_XZF;
		connectableTilesSubType[SIDE_A][1] = TileType::TILE_TYPE_XYF;
		connectableTilesSubType[SIDE_A][2] = TileType::TILE_TYPE_XYB;
		connectableTilesMaxPoint[SIDE_A][0] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);
		connectableTilesMaxPoint[SIDE_A][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		connectableTilesMaxPoint[SIDE_A][2] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);

		connectableTilesSubType[SIDE_B][0] = TileType::TILE_TYPE_XZF;
		connectableTilesSubType[SIDE_B][1] = TileType::TILE_TYPE_YZF;
		connectableTilesSubType[SIDE_B][2] = TileType::TILE_TYPE_YZB;
		connectableTilesMaxPoint[SIDE_B][0] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);
		connectableTilesMaxPoint[SIDE_B][1] = subjectTileMaxPoint + glm::ivec3(-1, 1, 0);
		connectableTilesMaxPoint[SIDE_B][2] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);

		connectableTilesSubType[SIDE_C][0] = TileType::TILE_TYPE_XZF;
		connectableTilesSubType[SIDE_C][1] = TileType::TILE_TYPE_XYF;
		connectableTilesSubType[SIDE_C][2] = TileType::TILE_TYPE_XYB;
		connectableTilesMaxPoint[SIDE_C][0] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);
		connectableTilesMaxPoint[SIDE_C][1] = subjectTileMaxPoint + glm::ivec3(0, 1, -1);
		connectableTilesMaxPoint[SIDE_C][2] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);

		connectableTilesSubType[SIDE_D][0] = TileType::TILE_TYPE_XZF;
		connectableTilesSubType[SIDE_D][1] = TileType::TILE_TYPE_YZF;
		connectableTilesSubType[SIDE_D][2] = TileType::TILE_TYPE_YZB;
		connectableTilesMaxPoint[SIDE_D][0] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);
		connectableTilesMaxPoint[SIDE_D][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		connectableTilesMaxPoint[SIDE_D][2] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);
		break;
	case TileType::TILE_TYPE_XZB:
		connectableTilesSubType[SIDE_A][0] = TileType::TILE_TYPE_XZB;
		connectableTilesSubType[SIDE_A][1] = TileType::TILE_TYPE_XYF;
		connectableTilesSubType[SIDE_A][2] = TileType::TILE_TYPE_XYB;
		connectableTilesMaxPoint[SIDE_A][0] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);
		connectableTilesMaxPoint[SIDE_A][1] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);
		connectableTilesMaxPoint[SIDE_A][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);

		connectableTilesSubType[SIDE_B][0] = TileType::TILE_TYPE_XZB;
		connectableTilesSubType[SIDE_B][1] = TileType::TILE_TYPE_YZF;
		connectableTilesSubType[SIDE_B][2] = TileType::TILE_TYPE_YZB;
		connectableTilesMaxPoint[SIDE_B][0] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);
		connectableTilesMaxPoint[SIDE_B][1] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);
		connectableTilesMaxPoint[SIDE_B][2] = subjectTileMaxPoint + glm::ivec3(-1, 1, 0);

		connectableTilesSubType[SIDE_C][0] = TileType::TILE_TYPE_XZB;
		connectableTilesSubType[SIDE_C][1] = TileType::TILE_TYPE_XYF;
		connectableTilesSubType[SIDE_C][2] = TileType::TILE_TYPE_XYB;
		connectableTilesMaxPoint[SIDE_C][0] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);
		connectableTilesMaxPoint[SIDE_C][1] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);
		connectableTilesMaxPoint[SIDE_C][2] = subjectTileMaxPoint + glm::ivec3(0, 1, -1);

		connectableTilesSubType[SIDE_D][0] = TileType::TILE_TYPE_XZB;
		connectableTilesSubType[SIDE_D][1] = TileType::TILE_TYPE_YZF;
		connectableTilesSubType[SIDE_D][2] = TileType::TILE_TYPE_YZB;
		connectableTilesMaxPoint[SIDE_D][0] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);
		connectableTilesMaxPoint[SIDE_D][1] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);
		connectableTilesMaxPoint[SIDE_D][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		break;
	case TileType::TILE_TYPE_YZF:
		connectableTilesSubType[SIDE_A][0] = TileType::TILE_TYPE_YZF;
		connectableTilesSubType[SIDE_A][1] = TileType::TILE_TYPE_XZF;
		connectableTilesSubType[SIDE_A][2] = TileType::TILE_TYPE_XZB;
		connectableTilesMaxPoint[SIDE_A][0] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);
		connectableTilesMaxPoint[SIDE_A][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		connectableTilesMaxPoint[SIDE_A][2] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);

		connectableTilesSubType[SIDE_B][0] = TileType::TILE_TYPE_YZF;
		connectableTilesSubType[SIDE_B][1] = TileType::TILE_TYPE_XYF;
		connectableTilesSubType[SIDE_B][2] = TileType::TILE_TYPE_XYB;
		connectableTilesMaxPoint[SIDE_B][0] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);
		connectableTilesMaxPoint[SIDE_B][1] = subjectTileMaxPoint + glm::ivec3(1, 0, -1);
		connectableTilesMaxPoint[SIDE_B][2] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);

		connectableTilesSubType[SIDE_C][0] = TileType::TILE_TYPE_YZF;
		connectableTilesSubType[SIDE_C][1] = TileType::TILE_TYPE_XZF;
		connectableTilesSubType[SIDE_C][2] = TileType::TILE_TYPE_XZB;
		connectableTilesMaxPoint[SIDE_C][0] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);
		connectableTilesMaxPoint[SIDE_C][1] = subjectTileMaxPoint + glm::ivec3(1, -1, 0);
		connectableTilesMaxPoint[SIDE_C][2] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);

		connectableTilesSubType[SIDE_D][0] = TileType::TILE_TYPE_YZF;
		connectableTilesSubType[SIDE_D][1] = TileType::TILE_TYPE_XYF;
		connectableTilesSubType[SIDE_D][2] = TileType::TILE_TYPE_XYB;
		connectableTilesMaxPoint[SIDE_D][0] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);
		connectableTilesMaxPoint[SIDE_D][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		connectableTilesMaxPoint[SIDE_D][2] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);
		break;
	case TileType::TILE_TYPE_YZB:
		connectableTilesSubType[SIDE_A][0] = TileType::TILE_TYPE_YZB;
		connectableTilesSubType[SIDE_A][1] = TileType::TILE_TYPE_XZF;
		connectableTilesSubType[SIDE_A][2] = TileType::TILE_TYPE_XZB;
		connectableTilesMaxPoint[SIDE_A][0] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);
		connectableTilesMaxPoint[SIDE_A][1] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);
		connectableTilesMaxPoint[SIDE_A][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);

		connectableTilesSubType[SIDE_B][0] = TileType::TILE_TYPE_YZB;
		connectableTilesSubType[SIDE_B][1] = TileType::TILE_TYPE_XYF;
		connectableTilesSubType[SIDE_B][2] = TileType::TILE_TYPE_XYB;
		connectableTilesMaxPoint[SIDE_B][0] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);
		connectableTilesMaxPoint[SIDE_B][1] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);
		connectableTilesMaxPoint[SIDE_B][2] = subjectTileMaxPoint + glm::ivec3(1, 0, -1);

		connectableTilesSubType[SIDE_C][0] = TileType::TILE_TYPE_YZB;
		connectableTilesSubType[SIDE_C][1] = TileType::TILE_TYPE_XZF;
		connectableTilesSubType[SIDE_C][2] = TileType::TILE_TYPE_XZB;
		connectableTilesMaxPoint[SIDE_C][0] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);
		connectableTilesMaxPoint[SIDE_C][1] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);
		connectableTilesMaxPoint[SIDE_C][2] = subjectTileMaxPoint + glm::ivec3(1, -1, 0);

		connectableTilesSubType[SIDE_D][0] = TileType::TILE_TYPE_YZB;
		connectableTilesSubType[SIDE_D][1] = TileType::TILE_TYPE_XYF;
		connectableTilesSubType[SIDE_D][2] = TileType::TILE_TYPE_XYB;
		connectableTilesMaxPoint[SIDE_D][0] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);
		connectableTilesMaxPoint[SIDE_D][1] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);
		connectableTilesMaxPoint[SIDE_D][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		break;
	}

	for (Tile* otherTile : tiles) {
		glm::ivec3 otherTileMaxPoint = otherTile->position;
		for (int sideIndex = 0; sideIndex < 4; sideIndex++) {
			for (int connectionType = 0; connectionType < 3; connectionType++) {

				if (otherTileMaxPoint == connectableTilesMaxPoint[sideIndex][connectionType] &&
					otherTile->type == connectableTilesSubType[sideIndex][connectionType]) {
					tryConnect(subjectTile, otherTile);
				}
			}

		}
	}
}

void TileManager::updateCornerSafety(Tile* tile)
{
	for (int i = 0; i < 4; i++) {
		int cornerIndex = i;
		LocalDirection dir1 = LocalDirection(i);
		LocalDirection dir2 = LocalDirection((dir1 + 3) % 4);
		Tile* neighbor1 = tile->getNeighbor(dir1);
		Tile* neighbor2 = tile->getNeighbor(dir2);

		if (neighbor1->index == neighbor2->index) {
			tile->cornerSafety[i] = Tile::CORNER_UNSAFE;
			continue;
		}

		LocalDirection dir1a = tile->getMappedNeighborAlignment(dir1, dir2);
		LocalDirection dir2a = tile->getMappedNeighborAlignment(dir2, dir1);
		Tile* neighborNeighbor1 = neighbor1->getNeighbor(dir1a);
		Tile* neighborNeighbor2 = neighbor2->getNeighbor(dir2a);

		if (neighborNeighbor1->index == neighborNeighbor2->index) {
			tile->cornerSafety[i] = Tile::CORNER_SAFE;
		}
		else {
			tile->cornerSafety[i] = Tile::CORNER_UNSAFE;
		}

	}
}

// Returns an integer from 0 (never visible) to 4 (most visible) when comparing two tile subtypes 
// from the perspective of a certain side.  Make sure to input into the array as follows:
// TILE_VISIBILITY[SUBJECT TILE SUB TYPE][SUBJECT TILE SIDE INDEX][COMPARE TILE SUB TYPE]
// SubType order is as follows:
// TILE_TYPE_XY_FRONT = 0,
// TILE_TYPE_XY_BACK  = 1,
// TILE_TYPE_XZ_FRONT = 2,
// TILE_TYPE_XZ_BACK  = 3,
// TileSubType::TILE_TYPE_YZ_FRONT = 4,
// TileSubType::TILE_TYPE_YZ_BACK  = 5,
const int TILE_VISIBILITY[6][4][6] = {
	{ // TILE_TYPE_XY_FRONT:
		{ // SIDE A (SideInfo index 0 -> 1):
			3, 1, 0, 0, 2, 4, // <- Levels of visibility
		},
		{ // SIDE B (SideInfo index 1 -> 2):
			3, 1, 4, 2, 0, 0, // <- Levels of visibility
		},
		{ // SIDE C (SideInfo index 2 -> 3):
			3, 1, 0, 0, 4, 2, // <- Levels of visibility
		},
		{ // SIDE D (SideInfo index 3 -> 0):
			3, 1, 2, 4, 0, 0, // <- Levels of visibility
		},
	},
	{ // TILE_TYPE_XY_BACK:
		{ // SIDE A (SideInfo index 0 -> 1):
			1, 3, 0, 0, 2, 4, // <- Levels of visibility
		},
		{ // SIDE B (SideInfo index 1 -> 2):
			1, 3, 4, 2, 0, 0, // <- Levels of visibility
		},
		{ // SIDE C (SideInfo index 2 -> 3):
			1, 3, 0, 0, 4, 2, // <- Levels of visibility
		},
		{ // SIDE D (SideInfo index 3 -> 0):
			1, 3, 2, 4, 0, 0, // <- Levels of visibility
		},
	},
	{ // TILE_TYPE_XZ_FRONT:
		{ // SIDE A (SideInfo index 0 -> 1):
			2, 4, 3, 1, 0, 0, // <- Levels of visibility
		},
		{ // SIDE B (SideInfo index 1 -> 2):
			0, 0, 3, 1, 4, 2, // <- Levels of visibility
		},
		{ // SIDE C (SideInfo index 2 -> 3):
			4, 2, 3, 1, 0, 0, // <- Levels of visibility
		},
		{ // SIDE D (SideInfo index 3 -> 0):
			0, 0, 3, 1, 2, 4, // <- Levels of visibility
		},
	},
	{ // TILE_TYPE_XZ_BACK = 3,
		{ // SIDE A (SideInfo index 0 -> 1):
			2, 4, 1, 3, 0, 0, // <- Levels of visibility
		},
		{ // SIDE B (SideInfo index 1 -> 2):
			0, 0, 1, 3, 4, 2, // <- Levels of visibility
		},
		{ // SIDE C (SideInfo index 2 -> 3):
			4, 2, 1, 3, 0, 0, // <- Levels of visibility
		},
		{ // SIDE D (SideInfo index 3 -> 0):
			0, 0, 1, 3, 2, 4, // <- Levels of visibility
		},
	},
	{ // TileSubType::TILE_TYPE_YZ_FRONT:
		{ // SIDE A (SideInfo index 0 -> 1):
			0, 0, 2, 4, 3, 1, // <- Levels of visibility
		},
		{ // SIDE B (SideInfo index 1 -> 2):
			4, 2, 0, 0, 3, 1, // <- Levels of visibility
		},
		{ // SIDE C (SideInfo index 2 -> 3):
			0, 0, 4, 2, 3, 1, // <- Levels of visibility
		},
		{ // SIDE D (SideInfo index 3 -> 0):
			2, 4, 0, 0, 3, 1, // <- Levels of visibility
		},
	},
	{ // TileSubType::TILE_TYPE_YZ_BACK:
		{ // SIDE A (SideInfo index 0 -> 1):
			0, 0, 2, 4, 1, 3, // <- Levels of visibility
		},
		{ // SIDE B (SideInfo index 1 -> 2):
			4, 2, 0, 0, 1, 3, // <- Levels of visibility
		},
		{ // SIDE C (SideInfo index 2 -> 3):
			0, 0, 4, 2, 1, 3, // <- Levels of visibility
		},
		{ // SIDE D (SideInfo index 3 -> 0):
			2, 4, 0, 0, 1, 3, // <- Levels of visibility
		},
	}
};

// Will return a value [0-4] denoting the tile connection's 'visibility.'  Higher values are
// more visible, meaning that if we have to choose between two possible connections, the one
// with the higher visibility will win out.  'sideIndex' is the index to the side who's
// connection visibility will be queried.  
// *Note that the index follows Tile (not DrawTile) ordering.
const int tileVisibility(Tile* tile, int sideIndex) {
	TileType connectionTileType = tile->neighborTilePtrs[sideIndex]->type;
	return TILE_VISIBILITY[tile->type][sideIndex][connectionTileType];
}

// Will return a value [0-4] denoting the potential tile connection's 'visibility.'  Higher 
// values are more visible, meaning that if we have to choose between two possible connections, 
// the one with the higher visibility will win out.  subjectType/subjectSideIndex corrospond
// to one tile, and otherType is the tile type of the other tile being connected to subject.
// *Note that the index follows Tile (not DrawTile) ordering.
const int tileVisibility(TileType subjectType, int subjectSideIndex, TileType otherType) {
	return TILE_VISIBILITY[subjectType][subjectSideIndex][otherType];
}

// Returns true if the other tile is more visible than the current 
// tile connected to the subject's side denoted by subjectSideIndex.
bool tileIsMoreVisible(Tile* subject, int subjectSideIndex,
	Tile* other, int otherSideIndex) {

	const int currentConnectionVisibility
		= tileVisibility(other, otherSideIndex);
	const int subjectConnectionVisibility
		= tileVisibility(subject, subjectSideIndex);
	const int newConnectionVisibility
		= tileVisibility(subject->type, subjectSideIndex, other->type);

	return newConnectionVisibility > currentConnectionVisibility &&
		newConnectionVisibility > subjectConnectionVisibility;
}

bool TileManager::tryConnect(Tile* subject, Tile* other) {
	int subjectConnections[2]{};
	int otherConnections[2]{};
	int connectionsIndex = 0;
	// Actually find the two matching vertex pairs:
	for (int subjectSideIndex = 0; subjectSideIndex < 4; subjectSideIndex++) {
		for (int otherSideIndex = 0; otherSideIndex < 4; otherSideIndex++) {
			if (subject->getVertPos(subjectSideIndex) == other->getVertPos(otherSideIndex)) {

				subjectConnections[connectionsIndex] = subjectSideIndex;
				otherConnections[connectionsIndex] = otherSideIndex;
				connectionsIndex++;
				break;
			}
		}
		if (connectionsIndex == 2) {
			break;
		}
	}
	// The two tiles were not actually connected:
	if (connectionsIndex < 2) {
		return false;
	}
	// Make sure that the first entry in the connections arrays are the side index.  
	// Side A is from index 0 -> 1, and if indices 0 and 1 are found to be connections 
	// but are out of order, the connection will index into side B (1 -> 2) instead!
	int temp;
	bool subjectConnectionClockwise = subjectConnections[1] == (subjectConnections[0] + 1) % 4;
	if (!subjectConnectionClockwise) {
		temp = subjectConnections[0];
		subjectConnections[0] = subjectConnections[1];
		subjectConnections[1] = temp;
	}
	bool otherConnectionClockwise = otherConnections[1] == (otherConnections[0] + 1) % 4;
	if (!otherConnectionClockwise) {
		temp = otherConnections[0];
		otherConnections[0] = otherConnections[1];
		otherConnections[1] = temp;
	}
	bool isMirroredConnection = subjectConnectionClockwise == otherConnectionClockwise;

	// Now that we know what side connects to what side, we can see if we actually 
	// want to connect the tiles.  Different tile sub types have different visibility 
	// to other tile sub types depending on the edge of the tile being connected.  
	// We dont want to connect this pair if there is a more visible tile already connected!
	int subjectConnectionIndex = subjectConnections[0];
	if (tileIsMoreVisible(subject, subjectConnectionIndex, other, otherConnections[0])) {
		// Finally, we make sure that when drawing, we know if this connection is clockwise 
		// or counterclockwise and what the connection is connecting to.  This helps us know 
		// how to orient the draw tile and what tile side corrosponds to what draw tile side.
		subject->neighborTilePtrs[subjectConnectionIndex] = other;
		subject->neighborAlignmentMapIndex[subjectConnectionIndex] = tnav::getNeighborAlignmentMapIndex((LocalDirection)subjectConnectionIndex, (LocalAlignment)otherConnections[0]);
		//subject->neighborConnectedSideIndices[subjectConnectionIndex] = otherConnections[0];
		//subject->isNeighborConnectionsMirrored[subjectConnectionIndex] = isMirroredConnection;
		// If one tile sees the other, the other tile must see it, and they will have the same winding:
		other->neighborTilePtrs[otherConnections[0]] = subject;
		other->neighborAlignmentMapIndex[otherConnections[0]] = tnav::getNeighborAlignmentMapIndex((LocalDirection)otherConnections[0], (LocalAlignment)subjectConnections[0]);
		//other->neighborConnectedSideIndices[otherConnections[0]] = subjectConnections[0];
		//other->isNeighborConnectionsMirrored[otherConnections[0]] = isMirroredConnection;
		return true;
	}
	return false;
}

void TileManager::updateWindowFrustum() {
	// Initial frustum to use so that other frustums can be clipped to the screen:
	windowFrustum.clear();
	glm::vec2 topLeft(p_camera->inverseTransfMatrix * glm::vec4(-1, +1, 0, 1));
	glm::vec2 topRight(p_camera->inverseTransfMatrix * glm::vec4(+1, +1, 0, 1));
	glm::vec2 bottomRight(p_camera->inverseTransfMatrix * glm::vec4(+1, -1, 0, 1));
	glm::vec2 bottomLeft(p_camera->inverseTransfMatrix * glm::vec4(-1, -1, 0, 1));
	glm::vec2 middle = (topLeft + topRight + bottomRight + bottomLeft) / 4.0f;
	windowFrustum.push_back(topLeft);
	windowFrustum.push_back(topRight);
	windowFrustum.push_back(bottomRight);
	windowFrustum.push_back(bottomLeft);
}

TileTarget TileManager::adjustTileTarget(TileTarget* currentPov, int drawTileSideIndex) {
	int newInitialSideIndex,
		newInitialTexIndex,
		newSideInfosOffset,
		connectionIndex = currentPov->sideIndex(drawTileSideIndex);
	Tile* newTarget;

	if (currentPov->tile->isNeighborConnectionsMirrored(connectionIndex)) {
		newSideInfosOffset = (currentPov->sideInfosOffset + 2) % 4;
	}
	else {
		newSideInfosOffset = currentPov->sideInfosOffset;
	}

	newInitialSideIndex = currentPov->tile->getNeighborConnectedSideIndex(LocalDirection(connectionIndex));
	newInitialSideIndex += VERT_INFO_OFFSETS[drawTileSideIndex] * newSideInfosOffset;
	newInitialSideIndex %= 4;

	if (newSideInfosOffset == 3) {
		newInitialTexIndex = (newInitialSideIndex + 1) % 4;
	}
	else {
		newInitialTexIndex = newInitialSideIndex;
	}

	newTarget = currentPov->tile->neighborTilePtrs[connectionIndex];

	return TileTarget(newTarget, newSideInfosOffset, newInitialSideIndex, newInitialTexIndex);
}

void TileManager::updatePovTileTarget() {
	// If the player crosses over the right (X+) edge of the initial draw tile, they
	// have crossed over the draw tile side that is indexed under '1' (as a refresher 
	// the draw tile index order is top, right, bottom, left, or Y+, X+, Y-, X-).

	if (p_camera->viewPlanePos.x > 1.0f) {
		lastpovTileTransf = currentpovTileTransf;
		lastpovTileTransfWeight = 1.0f;
		lastCamPosOffset = p_camera->viewPlanePos - glm::vec3(1, 0, 0);

		p_camera->viewPlanePos.x -= 1.0f;
		povTile = adjustTileTarget(&povTile, 0);

	}
	else if (p_camera->viewPlanePos.x < 0.0f) {
		lastpovTileTransf = currentpovTileTransf;
		lastpovTileTransfWeight = 1.0f;
		lastCamPosOffset = p_camera->viewPlanePos + glm::vec3(1, 0, 0);

		p_camera->viewPlanePos.x += 1.0f;
		povTile = adjustTileTarget(&povTile, 2);
	}
	if (p_camera->viewPlanePos.y > 1.0f) {
		lastpovTileTransf = currentpovTileTransf;
		lastpovTileTransfWeight = 1.0f;
		lastCamPosOffset = p_camera->viewPlanePos - glm::vec3(0, 1, 0);

		p_camera->viewPlanePos.y -= 1.0f;
		povTile = adjustTileTarget(&povTile, 3);
	}
	else if (p_camera->viewPlanePos.y < 0.0f) {
		lastpovTileTransf = currentpovTileTransf;
		lastpovTileTransfWeight = 1.0f;
		lastCamPosOffset = p_camera->viewPlanePos + glm::vec3(0, 1, 0);

		p_camera->viewPlanePos.y += 1.0f;
		povTile = adjustTileTarget(&povTile, 1);
	}
	// After moving the camera around, we must make sure the new position 
	// is properly recorded in all the tranforms needed for drawing!
	p_camera->getProjectionMatrix();
}

glm::vec3 TileManager::getPovTilePos() {
	return povTile.drawTilePos(2)
		+ (povTile.drawTilePos(1) - povTile.drawTilePos(2)) * p_camera->viewPlanePos.x
		+ (povTile.drawTilePos(3) - povTile.drawTilePos(2)) * p_camera->viewPlanePos.y;
}

void TileManager::solvePlayerUnsafeCornerCollisions() {
	int cornerIndex1, cornerIndex2;
	auto closestCorner = glm::vec2(0, 0);
	TileTarget target = povTile;

	if (p_camera->viewPlanePos.x > 0.5f) {
		target = adjustTileTarget(&povTile, 0);
		closestCorner += glm::vec2(1, 0);
	}
	else {
		target = adjustTileTarget(&povTile, 2);
	}
	if (p_camera->viewPlanePos.y > 0.5f) {
		target = adjustTileTarget(&target, 3);
		closestCorner += glm::vec2(0, 1);
	}
	else {
		target = adjustTileTarget(&target, 1);
	}
	cornerIndex1 = target.tile->index;

	if (p_camera->viewPlanePos.y > 0.5f) {
		target = adjustTileTarget(&povTile, 3);
	}
	else {
		target = adjustTileTarget(&povTile, 1);
	}
	if (p_camera->viewPlanePos.x > 0.5f) {
		target = adjustTileTarget(&target, 0);
	}
	else {
		target = adjustTileTarget(&target, 2);
	}
	cornerIndex2 = target.tile->index;

	bool isUnsafeCorner = cornerIndex1 == povTile.tile->index || cornerIndex1 != cornerIndex2;
	if (isUnsafeCorner) {
		glm::vec2 vecFromPosToCorner = closestCorner - (glm::vec2)p_camera->viewPlanePos;
		float distToCorner = glm::length(vecFromPosToCorner);
		if (distToCorner < 0.5f) {
			float scale = (0.5f - distToCorner) / 0.5f;
			glm::vec2 offset = -vecFromPosToCorner * scale;
			p_camera->viewPlanePos += glm::vec3(offset, 0);
		}
	}
}

void TileManager::update3dRotationAdj() {
	glm::vec3 upVec = povTile.tile->getVertPos(povTile.vertIndex(0))
		- povTile.tile->getVertPos(povTile.vertIndex(1));
	glm::mat4 rotate(1);
	switch (povTile.tile->type) {
	case TileType::TILE_TYPE_XYF: rotate = glm::mat4(1); break;
	case TileType::TILE_TYPE_XYB: rotate = glm::rotate(glm::mat4(1), float(M_PI), glm::vec3(0, 1, 0)); break;
	case TileType::TILE_TYPE_XZF: rotate = glm::rotate(glm::mat4(1), float(M_PI / 2.0f), glm::vec3(1, 0, 0)); break;
	case TileType::TILE_TYPE_XZB: rotate = glm::rotate(glm::mat4(1), -float(M_PI / 2.0f), glm::vec3(1, 0, 0)); break;
	case TileType::TILE_TYPE_YZF: rotate = glm::rotate(glm::mat4(1), -float(M_PI / 2.0f), glm::vec3(0, 1, 0)); break;
	case TileType::TILE_TYPE_YZB: rotate = glm::rotate(glm::mat4(1), float(M_PI / 2.0f), glm::vec3(0, 1, 0)); break;
	default: std::cout << "updatePovTile tile type enum out of scope!" << std::endl;
	}

	upVec = glm::vec3(rotate * glm::vec4(upVec, 1));
	upVec = floor(upVec + glm::vec3(0.2, 0.2, 0.2));
	if (upVec == glm::vec3(1, 0, 0)) {
		rotate = glm::rotate(glm::mat4(1), float(M_PI / 2.0f), glm::vec3(0, 0, 1)) * rotate;
	}
	else if (upVec == glm::vec3(-1, 0, 0)) {
		rotate = glm::rotate(glm::mat4(1), -float(M_PI / 2.0f), glm::vec3(0, 0, 1)) * rotate;
	}
	else if (upVec == glm::vec3(0, -1, 0)) {
		rotate = glm::rotate(glm::mat4(1), float(M_PI), glm::vec3(0, 0, 1)) * rotate;
	}

	glm::vec3 povTileNormal = povTile.tile->getNormal();
	glm::vec3 adjPovTileNormal = glm::vec3(rotate * glm::vec4(povTileNormal, 1));
	glm::vec3 targetNormal = povTileNormal;
	glm::vec3 targetNormalAdj(0, 0, 0);
	glm::vec3 flippedNormalAdj(0, 0, 0);
	TileTarget target;
	if (p_camera->viewPlanePos.x > 0.5f) {
		glm::vec3 neighborNormal = adjustTileTarget(&povTile, 0).tile->getNormal();
		if (neighborNormal == -povTileNormal) {
			targetNormal *= -((p_camera->viewPlanePos.x - 0.5f) * 2.0f - 1.0f);
			flippedNormalAdj += glm::vec3(1, 0, 0) * (p_camera->viewPlanePos.x - 0.5f) * 2.0f;
		}
		else if (neighborNormal != povTileNormal) {
			targetNormalAdj += neighborNormal * (p_camera->viewPlanePos.x - 0.5f) * 2.0f;
		}
	}
	else {
		glm::vec3 neighborNormal = adjustTileTarget(&povTile, 2).tile->getNormal();
		if (neighborNormal == -povTileNormal) {
			targetNormal *= p_camera->viewPlanePos.x * 2.0f;
			flippedNormalAdj += glm::vec3(-1, 0, 0) * -((p_camera->viewPlanePos.x * 2.0f) - 1.0f);
		}
		else if (neighborNormal != povTileNormal) {
			targetNormalAdj += neighborNormal * -((p_camera->viewPlanePos.x * 2.0f) - 1.0f);
		}
	}
	if (p_camera->viewPlanePos.y > 0.5f) {
		glm::vec3 neighborNormal = adjustTileTarget(&povTile, 3).tile->getNormal();
		if (neighborNormal == -povTileNormal) {
			glm::vec3 pensive = povTileNormal * -((p_camera->viewPlanePos.y - 0.5f) * 2.0f - 1.0f);
			if (glm::length(pensive) < glm::length(targetNormal)) {
				targetNormal = pensive;
			}
			flippedNormalAdj += glm::vec3(0, 1, 0) * (p_camera->viewPlanePos.y - 0.5f) * 2.0f;
		}
		else if (neighborNormal != povTileNormal)
			targetNormalAdj += neighborNormal * (p_camera->viewPlanePos.y - 0.5f) * 2.0f;
	}
	else {
		glm::vec3 neighborNormal = adjustTileTarget(&povTile, 1).tile->getNormal();
		if (neighborNormal == -povTileNormal) {
			glm::vec3 pensive = povTileNormal * p_camera->viewPlanePos.y * 2.0f;
			if (glm::length(pensive) < glm::length(targetNormal)) {
				targetNormal = pensive;
			}
			flippedNormalAdj += glm::vec3(0, -1, 0) * -((p_camera->viewPlanePos.y * 2.0f) - 1.0f);
		}
		else if (neighborNormal != povTileNormal)
			targetNormalAdj += neighborNormal * -((p_camera->viewPlanePos.y * 2.0f) - 1.0f);
	}
	targetNormal = glm::normalize(glm::vec3(rotate * glm::vec4(targetNormal + targetNormalAdj, 1)) + flippedNormalAdj);
	float angle = angleBetween(targetNormal, adjPovTileNormal);
	glm::vec3 axis = glm::cross(targetNormal, adjPovTileNormal);
	if (axis != glm::vec3(0, 0, 0)) {
		rotate = glm::rotate(glm::mat4(1), angle, axis) * rotate;
	}

	if (lastpovTileTransfWeight > 0) {
		lastpovTileTransfWeight -= 5.0f * DeltaTime;

		glm::quat firstQuat = glm::quat_cast(lastpovTileTransf);
		glm::quat secondQuat = glm::quat_cast(rotate);
		glm::quat finalQuat = glm::slerp(firstQuat, secondQuat, -(lastpovTileTransfWeight - 1.0f));
		currentpovTileTransf = glm::mat4_cast(finalQuat);
	}
	else {
		currentpovTileTransf = rotate;
	}

	glm::mat4 originToCamPos = glm::translate(glm::mat4(1), p_camera->viewPlanePos);
	glm::mat4 tilePosToOrigin = glm::translate(glm::mat4(1), -getPovTilePos());

	tileRotationAdjFor3DView
		= originToCamPos
		* currentpovTileTransf
		* tilePosToOrigin;
}

void TileManager::deleteTilePair(Tile* tile, bool allowDeletePovTile) {
	// Stuff will break if you delete the tile you are on.  Don't do that:
	if (!allowDeletePovTile && (tile == povTile.tile || tile->sibling == povTile.tile)) {
		return;
	}

	Tile* sibling = tile->sibling;
	if (sibling == nullptr) {
		throw std::runtime_error("NO SIBLING FOUND TO DELETE!");
	}

	// Gather up all the info needed to reconnect neighbor tiles to the map after removing the tile pair:
	Tile* neighborTilePtrs[8];
	int connectedTileIndices[8];
	for (int i = 0; i < 4; i++) {
		neighborTilePtrs[i] = tile->neighborTilePtrs[i];
		neighborTilePtrs[i + 4] = sibling->neighborTilePtrs[i];
		connectedTileIndices[i] = tile->getNeighborConnectedSideIndex(LocalDirection(i));
		connectedTileIndices[i + 4] = sibling->getNeighborConnectedSideIndex(LocalDirection(i));
	}

	int firstIndex = std::min(tile->index, sibling->index);
	tiles.erase(tiles.begin() + firstIndex);
	tiles.erase(tiles.begin() + firstIndex);

	for (int i = firstIndex; i < tiles.size(); i++) {
		tiles[i]->index -= 2; // <- Because we are deleting two tiles, the indices need to be offset by 2.
	}

	for (int i = 0; i < 8; i++) {
		// connectUpNewTile expects the input tile to be connected to *something* on all 4 edges, so here
		// we can just connect the orphaned edges to the tile's sibling:
		neighborTilePtrs[i]->neighborTilePtrs[connectedTileIndices[i]] = neighborTilePtrs[i]->sibling;
		neighborTilePtrs[i]->neighborAlignmentMapIndex[connectedTileIndices[i]] = tnav::getNeighborAlignmentMapIndex(LocalAlignment(connectedTileIndices[i]), LocalAlignment(connectedTileIndices[i]));
		connectUpNewTile(neighborTilePtrs[i]);
	}

	// Now that the tiles are all connected, we need to make sure that they have up to date info reguarding the
	// corner safety for player and entity movement updating:
	for (int i = 0; i < 8; i++) {
		updateCornerSafety(neighborTilePtrs[i]);
		// TODO: clean this up a little.  No need to update all these tiles
		for (int j = 0; j < 4; j++) {
			updateCornerSafety(neighborTilePtrs[i]->getNeighbor(LocalDirection(j)));
		}
	}

	// The index values stored in the tiles are messed up, so we need to update the gpu info for all the messed
	// up tiles:
	updateTileGpuInfos();

	delete tile;
	delete sibling;
}

int inverseDirection(int dir) {
	return (dir + 2) % 4;
}

void TileManager::update() {
	drawnTiles = 0;
}

void TileManager::updateVisualInfos()
{
	solvePlayerUnsafeCornerCollisions();
	updatePovTileTarget();
	update3dRotationAdj();
	updateWindowFrustum();
}

// In order to show where the player is in the 2D 3rd person POV view, we send some relative positional data
// to the GPU.  The positions are relative to drawVert[2] as origin with drawVert[2]->drawVert[1] acting as the
// 'x' direction and drawVert[2]->drawVert[3] acting as the 'y' direction.  Becuase the player is no larger than
// a single tile, a maximum of 4 tiles must have relative position data.  In the shader, each pixel knows what
// tile it is in, so it queries this info using that index to see if it is actually inside the player, then colors
// accordingly.
void TileManager::getRelativePovPosGpuInfos(glm::vec2* relativePos, int* relativePosTileIndices) {
	TileTarget temp;

	// By definition we are always in the povTile:
	relativePosTileIndices[0] = povTile.tile->index;
	relativePos[0] = getRelativePovPosCentral(povTile);

	// Top/Bottom:
	if (p_camera->viewPlanePos.y >= 0.5f) {
		temp = adjustTileTarget(&povTile, 3);
		relativePosTileIndices[1] = temp.tile->index;
		relativePos[1] = getRelativePovPosTop(temp);

		// Corner:
		if (p_camera->viewPlanePos.x >= 0.5f) {
			temp = adjustTileTarget(&temp, 0);
			relativePosTileIndices[2] = temp.tile->index;
			relativePos[2] = getRelativePovPosTopRight(temp);
		}
		else {
			temp = adjustTileTarget(&temp, 2);
			relativePosTileIndices[2] = temp.tile->index;
			relativePos[2] = getRelativePovPosTopLeft(temp);
		}
	}
	else {
		temp = adjustTileTarget(&povTile, 1);
		relativePosTileIndices[1] = temp.tile->index;
		relativePos[1] = getRelativePovPosBottom(temp);

		// Corner:
		if (p_camera->viewPlanePos.x >= 0.5f) {
			temp = adjustTileTarget(&temp, 0);
			relativePosTileIndices[2] = temp.tile->index;
			relativePos[2] = getRelativePovPosBottomRight(temp);
		}
		else {
			temp = adjustTileTarget(&temp, 2);
			relativePosTileIndices[2] = temp.tile->index;
			relativePos[2] = getRelativePovPosBottomLeft(temp);
		}
	}

	// Left/Right and Corner:
	if (p_camera->viewPlanePos.x >= 0.5f) {
		temp = adjustTileTarget(&povTile, 0);
		relativePosTileIndices[3] = temp.tile->index;
		relativePos[3] = getRelativePovPosRight(temp);

		// Corner:
		if (p_camera->viewPlanePos.y >= 0.5f) {
			temp = adjustTileTarget(&temp, 3);
			relativePosTileIndices[4] = temp.tile->index;
			relativePos[4] = getRelativePovPosTopRight(temp);
		}
		else {
			temp = adjustTileTarget(&temp, 1);
			relativePosTileIndices[4] = temp.tile->index;
			relativePos[4] = getRelativePovPosBottomRight(temp);
		}
	}
	else {
		temp = adjustTileTarget(&povTile, 2);
		relativePosTileIndices[3] = temp.tile->index;
		relativePos[3] = getRelativePovPosLeft(temp);

		// Corner:
		if (p_camera->viewPlanePos.y >= 0.5f) {
			temp = adjustTileTarget(&temp, 3);
			relativePosTileIndices[4] = temp.tile->index;
			relativePos[4] = getRelativePovPosTopLeft(temp);
		}
		else {
			temp = adjustTileTarget(&temp, 1);
			relativePosTileIndices[4] = temp.tile->index;
			relativePos[4] = getRelativePovPosBottomLeft(temp);
		}
	}
}

glm::vec2 TileManager::getRelativePovPosCentral(TileTarget& target) {
	// Relative to vert[2] (origin), vert[1] (x+), and vert[3] (y+).
	glm::vec2 P = p_camera->viewPlanePos;

	if (target.woundClockwise()) {
		switch (target.initialVertIndex) {
		case 0: return glm::vec2(P.x, P.y);
		case 1: return glm::vec2(P.y, 1.0f - P.x);
		case 2: return glm::vec2(1.0f - P.x, 1.0f - P.y);
		case 3: return glm::vec2(1.0f - P.y, P.x);
		default:
			std::cout << "getRelativePosCentral initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
	else /*counterclockwise winding*/ {
		switch (target.initialVertIndex) {
		case 0: return glm::vec2(P.y, P.x);
		case 1: return glm::vec2(P.x, 1.0f - P.y);
		case 2: return glm::vec2(1.0f - P.y, 1.0f - P.x);
		case 3: return glm::vec2(1.0f - P.x, P.y);
		default:
			std::cout << "getRelativePosCentral initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
}
glm::vec2 TileManager::getRelativePovPosTop(TileTarget& target) {
	glm::vec2 P = (glm::vec2)p_camera->viewPlanePos;

	if (target.woundClockwise()) {
		switch (target.initialVertIndex) {
		case 0: return glm::vec2(P.x, P.y - 1.0f); break;
		case 1: return glm::vec2(P.y - 1, 1.0f - P.x); break;
		case 2: return glm::vec2(1.0f - P.x, 2.0f - P.y); break;
		case 3: return glm::vec2(2.0f - P.y, P.x); break;
		default:
			std::cout << "getRelativePosTop initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
	else { //Counterclockwise winding:
		switch (target.initialVertIndex) {
		case 0: return glm::vec2(P.y - 1.0f, P.x); break;
		case 1: return glm::vec2(P.x, 2.0f - P.y); break;
		case 2: return glm::vec2(2.0f - P.y, 1.0f - P.x); break;
		case 3: return glm::vec2(1.0f - P.x, P.y - 1.0f); break;
		default:
			std::cout << "getRelativePosTop initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
}
glm::vec2 TileManager::getRelativePovPosBottom(TileTarget& target) {
	glm::vec2 P = (glm::vec2)p_camera->viewPlanePos;

	if (target.woundClockwise()) {
		switch (target.initialVertIndex) {
		case 0: return glm::vec2(P.x, 1.0f + P.y); break;
		case 1: return glm::vec2(1.0f + P.y, 1.0f - P.x); break;
		case 2: return glm::vec2(1.0f - P.x, -P.y); break;
		case 3: return glm::vec2(-P.y, P.x); break;
		default:
			std::cout << "getRelativePosBottom initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
	else { //Counterclockwise winding:
		switch (target.initialVertIndex) {
		case 0: return glm::vec2(1.0f + P.y, P.x); break;
		case 1: return glm::vec2(P.x, -P.y); break;
		case 2: return glm::vec2(-P.y, 1.0f - P.x); break;
		case 3: return glm::vec2(1.0f - P.x, 1.0f + P.y); break;
		default:
			std::cout << "getRelativePosBottom initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
}
glm::vec2 TileManager::getRelativePovPosRight(TileTarget& target) {
	glm::vec2 P = (glm::vec2)p_camera->viewPlanePos;

	if (target.woundClockwise()) {
		switch (target.initialVertIndex) {
		case 0:return glm::vec2(P.x - 1.0f, P.y); break;
		case 1:return glm::vec2(P.y, 2.0f - P.x); break;
		case 2:return glm::vec2(2.0f - P.x, 1.0 - P.y); break;
		case 3:return glm::vec2(1.0f - P.y, P.x - 1.0f); break;
		default:
			std::cout << "getRelativePosRight initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
	else { //Counterclockwise winding:
		switch (target.initialVertIndex) {
		case 0:return glm::vec2(P.y, P.x - 1.0f); break;
		case 1:return glm::vec2(P.x - 1.0f, 1.0f - P.y); break;
		case 2:return glm::vec2(1.0f - P.y, 2.0f - P.x); break;
		case 3:return glm::vec2(2.0f - P.x, P.y); break;
		default:
			std::cout << "getRelativePosRight initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
}
glm::vec2 TileManager::getRelativePovPosLeft(TileTarget& target) {
	glm::vec2 P = (glm::vec2)p_camera->viewPlanePos;

	if (target.woundClockwise()) {
		switch (target.initialVertIndex) {
		case 0:return glm::vec2(1.0f + P.x, P.y); break;
		case 1:return glm::vec2(P.y, -P.x); break;
		case 2:return glm::vec2(-P.x, 1.0f - P.y); break;
		case 3:return glm::vec2(1.0f - P.y, 1.0f + P.x); break;
		default:
			std::cout << "getRelativePosLeft initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
	else { //Counterclockwise winding:
		switch (target.initialVertIndex) {
		case 0:return glm::vec2(P.y, 1.0f + P.x); break;
		case 1:return glm::vec2(1.0f + P.x, 1.0f - P.y); break;
		case 2:return glm::vec2(1.0f - P.y, -P.x); break;
		case 3:return glm::vec2(-P.x, P.y); break;
		default:
			std::cout << "getRelativePosLeft initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
}
glm::vec2 TileManager::getRelativePovPosTopRight(TileTarget& target) {
	glm::vec2 P = (glm::vec2)p_camera->viewPlanePos;

	if (target.woundClockwise()) {
		switch (target.initialVertIndex) {
		case 0: return glm::vec2(P.x - 1.0f, P.y - 1.0f); break;
		case 1: return glm::vec2(P.y - 1.0f, 2.0f - P.x); break;
		case 2: return glm::vec2(2.0f - P.x, 2.0f - P.y); break;
		case 3: return glm::vec2(2.0f - P.y, P.x - 1.0f); break;
		default:
			std::cout << "getRelativePosTopRight initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
	else { //Counterclockwise winding:
		switch (target.initialVertIndex) {
		case 0: return glm::vec2(P.y - 1.0f, P.x - 1.0f); break;
		case 1: return glm::vec2(P.x - 1.0f, 2.0f - P.y); break;
		case 2: return glm::vec2(2.0f - P.y, 2.0f - P.x); break;
		case 3: return glm::vec2(2.0f - P.x, P.y - 1.0f); break;
		default:
			std::cout << "getRelativePosTopRight initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
}
glm::vec2 TileManager::getRelativePovPosTopLeft(TileTarget& target) {
	glm::vec2 P = (glm::vec2)p_camera->viewPlanePos;

	if (target.woundClockwise()) {
		switch (target.initialVertIndex) {
		case 0: return glm::vec2(1.0f + P.x, P.y - 1.0f); break;
		case 1: return glm::vec2(P.y - 1.0f, -P.x); break;
		case 2: return glm::vec2(-P.x, 2.0f - P.y); break;
		case 3: return glm::vec2(2.0f - P.y, 1.0f + P.x); break;
		default:
			std::cout << "getRelativePosTopLeft initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
	else { //Counterclockwise winding:
		switch (target.initialVertIndex) {
		case 0: return glm::vec2(P.y - 1.0f, 1.0f + P.x); break;
		case 1: return glm::vec2(1.0f + P.x, 2.0f - P.y); break;
		case 2: return glm::vec2(2.0f - P.y, -P.x); break;
		case 3: return glm::vec2(-P.x, P.y - 1.0f); break;
		default:
			std::cout << "getRelativePosTopLeft initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
}
glm::vec2 TileManager::getRelativePovPosBottomRight(TileTarget& target) {
	glm::vec2 P = (glm::vec2)p_camera->viewPlanePos;

	if (target.woundClockwise()) {
		switch (target.initialVertIndex) {
		case 0: return glm::vec2(P.x - 1.0f, P.y + 1.0f); break;
		case 1: return glm::vec2(P.y + 1.0f, 2.0f - P.x); break;
		case 2: return glm::vec2(2.0f - P.x, -P.y); break;
		case 3: return glm::vec2(-P.y, P.x - 1.0f); break;
		default:
			std::cout << "getRelativePosBottomRight initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
	else { //Counterclockwise winding:
		switch (target.initialVertIndex) {
		case 0: return glm::vec2(1.0f + P.y, P.x - 1.0f); break;
		case 1: return glm::vec2(P.x - 1.0f, -P.y); break;
		case 2: return glm::vec2(-P.y, 2.0f - P.x); break;
		case 3: return glm::vec2(2.0f - P.x, 1.0f + P.y); break;
		default:
			std::cout << "getRelativePosBottomRight initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
}
glm::vec2 TileManager::getRelativePovPosBottomLeft(TileTarget& target) {
	glm::vec2 P = (glm::vec2)p_camera->viewPlanePos;

	if (target.woundClockwise()) {
		switch (target.initialVertIndex) {
		case 0: return glm::vec2(P.x + 1.0f, P.y + 1.0f);
		case 1: return glm::vec2(P.y + 1.0f, -P.x);
		case 2: return glm::vec2(-P.x, -P.y);
		case 3: return glm::vec2(-P.y, P.x + 1.0f);
		default:
			std::cout << "getRelativePosBottomLeft initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
	else { //Counterclockwise winding:
		switch (target.initialVertIndex) {
		case 0: return glm::vec2(P.y + 1.0f, P.x + 1.0f);
		case 1: return glm::vec2(P.x + 1.0f, -P.y);
		case 2: return glm::vec2(-P.y, -P.x);
		case 3: return glm::vec2(-P.x, P.y + 1.0f);
		default:
			std::cout << "getRelativePosBottomLeft initialVertIndex out of scope!" << std::endl;
			return glm::vec2(0, 0);
		}
	}
}

void TileManager::updateTileGpuInfos() {
	tileGpuInfos.clear();
	for (Tile* tile : tiles) {
		tileGpuInfos.push_back(GPU_TileInfo(tile));
	}
}