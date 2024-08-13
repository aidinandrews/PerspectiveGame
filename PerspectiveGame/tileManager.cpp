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
		if (t->type == newTile.type && t->maxVert == newTile.maxVert) {
			return false;
		}
	}
	return true;
}

bool TileManager::createTilePair(Tile::Type tileType, glm::ivec3 maxPoint,
	glm::vec3 frontTileColor, glm::vec3 backTileColor) {
	Tile* frontTile = new Tile(Tile::tileSubType(tileType, true), maxPoint);
	Tile* backTile = new Tile(Tile::tileSubType(tileType, false), maxPoint);

	// before connecting everything up, it is importand that this new tile pair 
	// does not overlap another tile pair, as that would be against the rules:
	if (!tileIsUnique(*frontTile)) {
		return false;
	}

	frontTile->sibling = backTile;
	backTile->sibling = frontTile;

	switch (tileType) {
	case Tile::TILE_TYPE_XY:
		frontTile->type = TileSubType::TILE_TYPE_XY_FRONT;
		backTile->type = TileSubType::TILE_TYPE_XY_BACK;
		break;
	case Tile::TILE_TYPE_XZ:
		frontTile->type = TileSubType::TILE_TYPE_XZ_FRONT;
		backTile->type = TileSubType::TILE_TYPE_XZ_BACK;
		break;
	case Tile::TILE_TYPE_YZ:
		frontTile->type = TileSubType::TILE_TYPE_YZ_FRONT;
		backTile->type = TileSubType::TILE_TYPE_YZ_BACK;
		break;
	}

	frontTile->color = frontTileColor;
	backTile->color = backTileColor;

	for (int i = 0; i < 4; i++) {
		// As we have not yet checked if there are other tiles connected to this tile pair, 
		// it can only be known that these two tiles see each other.  This will be changed 
		// if other tiles are connected and are seen to be the visible connection:
		frontTile->sideInfos.connectedTiles[i] = backTile;
		backTile->sideInfos.connectedTiles[i] = frontTile;
		// When drawing and shuffling items between tiles, it is important to know 
		// what sides are connected so that the tile sides can be properly indexed:
		frontTile->sideInfos.connectedSideIndices[i] = i;
		backTile->sideInfos.connectedSideIndices[i] = i;
		// When on the edge of one tile and looking around to the face of the glued 
		// tile, it would seem that the other tile has been 'flipped up.'  This makes 
		// it look like the tile has been mirrored and so when drawing the 2D 3rd person 
		// POV, it is important to know that we go counter clockwise around the vertices 
		// instead of clockwise, as would be proper for other types of tile connections.
		frontTile->sideInfos.connectionsMirrored[i] = true;
		backTile->sideInfos.connectionsMirrored[i] = true;
	}

	// Connect up the new tiles to the other ones in the scene:
	connectUpNewTile(frontTile);
	connectUpNewTile(backTile);

	// Finally, we can add them to the list!
	frontTile->index = (int)tiles.size();
	backTile->index = (int)tiles.size() + 1;

	tiles.push_back(frontTile);
	tiles.push_back(backTile);

	tileGpuInfos.push_back(TileGpuInfo(frontTile));
	tileGpuInfos.push_back(TileGpuInfo(backTile));

	//updateTileGpuInfoIndices();

	return true;
}

void TileManager::connectUpNewTile(Tile* subjectTile) {
	// Each side of a tile can only even theoredically connect to some types/orientations of tile,
	// so each edge (connectableTiles[X][]) gets a list of the types of tiles it can connect to 
	// (connectableTiles[][X]).
	TileSubType connectableTilesSubType[4][3];
	glm::ivec3 connectableTilesMaxPoint[4][3]; // tile.sideInfos[0].pos is always the max vert.
	const int SIDE_A = 0, SIDE_B = 1, SIDE_C = 2, SIDE_D = 3;
	glm::ivec3 subjectTileMaxPoint = subjectTile->maxVert;
	
	// Big honkin' list of all the possible tiles that can connect to this tile, depending on what tile sub 
	// type this new tile is.  Index into it to get the exact position that the connected tile's 1st vertex 
	// (max vertex) must have to actually be connected to it.
	switch (subjectTile->type) {
	case TileSubType::TILE_TYPE_XY_FRONT:
		connectableTilesSubType[SIDE_A][0] = TileSubType::TILE_TYPE_XY_FRONT;
		connectableTilesSubType[SIDE_A][1] = TileSubType::TILE_TYPE_YZ_FRONT;
		connectableTilesSubType[SIDE_A][2] = TileSubType::TILE_TYPE_YZ_BACK;
		connectableTilesMaxPoint[SIDE_A][0] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);
		connectableTilesMaxPoint[SIDE_A][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		connectableTilesMaxPoint[SIDE_A][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);

		connectableTilesSubType[SIDE_B][0] = TileSubType::TILE_TYPE_XY_FRONT;
		connectableTilesSubType[SIDE_B][1] = TileSubType::TILE_TYPE_XZ_FRONT;
		connectableTilesSubType[SIDE_B][2] = TileSubType::TILE_TYPE_XZ_BACK;
		connectableTilesMaxPoint[SIDE_B][0] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);
		connectableTilesMaxPoint[SIDE_B][1] = subjectTileMaxPoint + glm::ivec3(0, -1, 1);
		connectableTilesMaxPoint[SIDE_B][2] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);

		connectableTilesSubType[SIDE_C][0] = TileSubType::TILE_TYPE_XY_FRONT;
		connectableTilesSubType[SIDE_C][1] = TileSubType::TILE_TYPE_YZ_FRONT;
		connectableTilesSubType[SIDE_C][2] = TileSubType::TILE_TYPE_YZ_BACK;
		connectableTilesMaxPoint[SIDE_C][0] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);
		connectableTilesMaxPoint[SIDE_C][1] = subjectTileMaxPoint + glm::ivec3(-1, 0, 1);
		connectableTilesMaxPoint[SIDE_C][2] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);

		connectableTilesSubType[SIDE_D][0] = TileSubType::TILE_TYPE_XY_FRONT;
		connectableTilesSubType[SIDE_D][1] = TileSubType::TILE_TYPE_XZ_FRONT;
		connectableTilesSubType[SIDE_D][2] = TileSubType::TILE_TYPE_XZ_BACK;
		connectableTilesMaxPoint[SIDE_D][0] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);
		connectableTilesMaxPoint[SIDE_D][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		connectableTilesMaxPoint[SIDE_D][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);
		break;
	case TileSubType::TILE_TYPE_XY_BACK:
		connectableTilesSubType[SIDE_A][0] = TileSubType::TILE_TYPE_XY_BACK;
		connectableTilesSubType[SIDE_A][1] = TileSubType::TILE_TYPE_YZ_FRONT;
		connectableTilesSubType[SIDE_A][2] = TileSubType::TILE_TYPE_YZ_BACK;
		connectableTilesMaxPoint[SIDE_A][0] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);
		connectableTilesMaxPoint[SIDE_A][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);
		connectableTilesMaxPoint[SIDE_A][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);

		connectableTilesSubType[SIDE_B][0] = TileSubType::TILE_TYPE_XY_BACK;
		connectableTilesSubType[SIDE_B][1] = TileSubType::TILE_TYPE_XZ_FRONT;
		connectableTilesSubType[SIDE_B][2] = TileSubType::TILE_TYPE_XZ_BACK;
		connectableTilesMaxPoint[SIDE_B][0] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);
		connectableTilesMaxPoint[SIDE_B][1] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);
		connectableTilesMaxPoint[SIDE_B][2] = subjectTileMaxPoint + glm::ivec3(0, -1, 1);

		connectableTilesSubType[SIDE_C][0] = TileSubType::TILE_TYPE_XY_BACK;
		connectableTilesSubType[SIDE_C][1] = TileSubType::TILE_TYPE_YZ_FRONT;
		connectableTilesSubType[SIDE_C][2] = TileSubType::TILE_TYPE_YZ_BACK;
		connectableTilesMaxPoint[SIDE_C][0] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);
		connectableTilesMaxPoint[SIDE_C][1] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);
		connectableTilesMaxPoint[SIDE_C][2] = subjectTileMaxPoint + glm::ivec3(-1, 0, 1);

		connectableTilesSubType[SIDE_D][0] = TileSubType::TILE_TYPE_XY_BACK;
		connectableTilesSubType[SIDE_D][1] = TileSubType::TILE_TYPE_XZ_FRONT;
		connectableTilesSubType[SIDE_D][2] = TileSubType::TILE_TYPE_XZ_BACK;
		connectableTilesMaxPoint[SIDE_D][0] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);
		connectableTilesMaxPoint[SIDE_D][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);
		connectableTilesMaxPoint[SIDE_D][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		break;
	case TileSubType::TILE_TYPE_XZ_FRONT:
		connectableTilesSubType[SIDE_A][0] = TileSubType::TILE_TYPE_XZ_FRONT;
		connectableTilesSubType[SIDE_A][1] = TileSubType::TILE_TYPE_XY_FRONT;
		connectableTilesSubType[SIDE_A][2] = TileSubType::TILE_TYPE_XY_BACK;
		connectableTilesMaxPoint[SIDE_A][0] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);
		connectableTilesMaxPoint[SIDE_A][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		connectableTilesMaxPoint[SIDE_A][2] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);

		connectableTilesSubType[SIDE_B][0] = TileSubType::TILE_TYPE_XZ_FRONT;
		connectableTilesSubType[SIDE_B][1] = TileSubType::TILE_TYPE_YZ_FRONT;
		connectableTilesSubType[SIDE_B][2] = TileSubType::TILE_TYPE_YZ_BACK;
		connectableTilesMaxPoint[SIDE_B][0] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);
		connectableTilesMaxPoint[SIDE_B][1] = subjectTileMaxPoint + glm::ivec3(-1, 1, 0);
		connectableTilesMaxPoint[SIDE_B][2] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);

		connectableTilesSubType[SIDE_C][0] = TileSubType::TILE_TYPE_XZ_FRONT;
		connectableTilesSubType[SIDE_C][1] = TileSubType::TILE_TYPE_XY_FRONT;
		connectableTilesSubType[SIDE_C][2] = TileSubType::TILE_TYPE_XY_BACK;
		connectableTilesMaxPoint[SIDE_C][0] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);
		connectableTilesMaxPoint[SIDE_C][1] = subjectTileMaxPoint + glm::ivec3(0, 1, -1);
		connectableTilesMaxPoint[SIDE_C][2] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);

		connectableTilesSubType[SIDE_D][0] = TileSubType::TILE_TYPE_XZ_FRONT;
		connectableTilesSubType[SIDE_D][1] = TileSubType::TILE_TYPE_YZ_FRONT;
		connectableTilesSubType[SIDE_D][2] = TileSubType::TILE_TYPE_YZ_BACK;
		connectableTilesMaxPoint[SIDE_D][0] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);
		connectableTilesMaxPoint[SIDE_D][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		connectableTilesMaxPoint[SIDE_D][2] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);
		break;
	case TileSubType::TILE_TYPE_XZ_BACK:
		connectableTilesSubType[SIDE_A][0] = TileSubType::TILE_TYPE_XZ_BACK;
		connectableTilesSubType[SIDE_A][1] = TileSubType::TILE_TYPE_XY_FRONT;
		connectableTilesSubType[SIDE_A][2] = TileSubType::TILE_TYPE_XY_BACK;
		connectableTilesMaxPoint[SIDE_A][0] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);
		connectableTilesMaxPoint[SIDE_A][1] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);
		connectableTilesMaxPoint[SIDE_A][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);

		connectableTilesSubType[SIDE_B][0] = TileSubType::TILE_TYPE_XZ_BACK;
		connectableTilesSubType[SIDE_B][1] = TileSubType::TILE_TYPE_YZ_FRONT;
		connectableTilesSubType[SIDE_B][2] = TileSubType::TILE_TYPE_YZ_BACK;
		connectableTilesMaxPoint[SIDE_B][0] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);
		connectableTilesMaxPoint[SIDE_B][1] = subjectTileMaxPoint + glm::ivec3(-1, 0, 0);
		connectableTilesMaxPoint[SIDE_B][2] = subjectTileMaxPoint + glm::ivec3(-1, 1, 0);

		connectableTilesSubType[SIDE_C][0] = TileSubType::TILE_TYPE_XZ_BACK;
		connectableTilesSubType[SIDE_C][1] = TileSubType::TILE_TYPE_XY_FRONT;
		connectableTilesSubType[SIDE_C][2] = TileSubType::TILE_TYPE_XY_BACK;
		connectableTilesMaxPoint[SIDE_C][0] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);
		connectableTilesMaxPoint[SIDE_C][1] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);
		connectableTilesMaxPoint[SIDE_C][2] = subjectTileMaxPoint + glm::ivec3(0, 1, -1);

		connectableTilesSubType[SIDE_D][0] = TileSubType::TILE_TYPE_XZ_BACK;
		connectableTilesSubType[SIDE_D][1] = TileSubType::TILE_TYPE_YZ_FRONT;
		connectableTilesSubType[SIDE_D][2] = TileSubType::TILE_TYPE_YZ_BACK;
		connectableTilesMaxPoint[SIDE_D][0] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);
		connectableTilesMaxPoint[SIDE_D][1] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);
		connectableTilesMaxPoint[SIDE_D][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		break;
	case TileSubType::TILE_TYPE_YZ_FRONT:
		connectableTilesSubType[SIDE_A][0] = TileSubType::TILE_TYPE_YZ_FRONT;
		connectableTilesSubType[SIDE_A][1] = TileSubType::TILE_TYPE_XZ_FRONT;
		connectableTilesSubType[SIDE_A][2] = TileSubType::TILE_TYPE_XZ_BACK;
		connectableTilesMaxPoint[SIDE_A][0] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);
		connectableTilesMaxPoint[SIDE_A][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		connectableTilesMaxPoint[SIDE_A][2] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);

		connectableTilesSubType[SIDE_B][0] = TileSubType::TILE_TYPE_YZ_FRONT;
		connectableTilesSubType[SIDE_B][1] = TileSubType::TILE_TYPE_XY_FRONT;
		connectableTilesSubType[SIDE_B][2] = TileSubType::TILE_TYPE_XY_BACK;
		connectableTilesMaxPoint[SIDE_B][0] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);
		connectableTilesMaxPoint[SIDE_B][1] = subjectTileMaxPoint + glm::ivec3(1, 0, -1);
		connectableTilesMaxPoint[SIDE_B][2] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);

		connectableTilesSubType[SIDE_C][0] = TileSubType::TILE_TYPE_YZ_FRONT;
		connectableTilesSubType[SIDE_C][1] = TileSubType::TILE_TYPE_XZ_FRONT;
		connectableTilesSubType[SIDE_C][2] = TileSubType::TILE_TYPE_XZ_BACK;
		connectableTilesMaxPoint[SIDE_C][0] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);
		connectableTilesMaxPoint[SIDE_C][1] = subjectTileMaxPoint + glm::ivec3(1, -1, 0);
		connectableTilesMaxPoint[SIDE_C][2] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);

		connectableTilesSubType[SIDE_D][0] = TileSubType::TILE_TYPE_YZ_FRONT;
		connectableTilesSubType[SIDE_D][1] = TileSubType::TILE_TYPE_XY_FRONT;
		connectableTilesSubType[SIDE_D][2] = TileSubType::TILE_TYPE_XY_BACK;
		connectableTilesMaxPoint[SIDE_D][0] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);
		connectableTilesMaxPoint[SIDE_D][1] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		connectableTilesMaxPoint[SIDE_D][2] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);
		break;
	case TileSubType::TILE_TYPE_YZ_BACK:
		connectableTilesSubType[SIDE_A][0] = TileSubType::TILE_TYPE_YZ_BACK;
		connectableTilesSubType[SIDE_A][1] = TileSubType::TILE_TYPE_XZ_FRONT;
		connectableTilesSubType[SIDE_A][2] = TileSubType::TILE_TYPE_XZ_BACK;
		connectableTilesMaxPoint[SIDE_A][0] = subjectTileMaxPoint + glm::ivec3(0, 1, 0);
		connectableTilesMaxPoint[SIDE_A][1] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);
		connectableTilesMaxPoint[SIDE_A][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);

		connectableTilesSubType[SIDE_B][0] = TileSubType::TILE_TYPE_YZ_BACK;
		connectableTilesSubType[SIDE_B][1] = TileSubType::TILE_TYPE_XY_FRONT;
		connectableTilesSubType[SIDE_B][2] = TileSubType::TILE_TYPE_XY_BACK;
		connectableTilesMaxPoint[SIDE_B][0] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);
		connectableTilesMaxPoint[SIDE_B][1] = subjectTileMaxPoint + glm::ivec3(0, 0, -1);
		connectableTilesMaxPoint[SIDE_B][2] = subjectTileMaxPoint + glm::ivec3(1, 0, -1);

		connectableTilesSubType[SIDE_C][0] = TileSubType::TILE_TYPE_YZ_BACK;
		connectableTilesSubType[SIDE_C][1] = TileSubType::TILE_TYPE_XZ_FRONT;
		connectableTilesSubType[SIDE_C][2] = TileSubType::TILE_TYPE_XZ_BACK;
		connectableTilesMaxPoint[SIDE_C][0] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);
		connectableTilesMaxPoint[SIDE_C][1] = subjectTileMaxPoint + glm::ivec3(0, -1, 0);
		connectableTilesMaxPoint[SIDE_C][2] = subjectTileMaxPoint + glm::ivec3(1, -1, 0);

		connectableTilesSubType[SIDE_D][0] = TileSubType::TILE_TYPE_YZ_BACK;
		connectableTilesSubType[SIDE_D][1] = TileSubType::TILE_TYPE_XY_FRONT;
		connectableTilesSubType[SIDE_D][2] = TileSubType::TILE_TYPE_XY_BACK;
		connectableTilesMaxPoint[SIDE_D][0] = subjectTileMaxPoint + glm::ivec3(0, 0, 1);
		connectableTilesMaxPoint[SIDE_D][1] = subjectTileMaxPoint + glm::ivec3(1, 0, 0);
		connectableTilesMaxPoint[SIDE_D][2] = subjectTileMaxPoint + glm::ivec3(0, 0, 0);
		break;
	}

	for (Tile* otherTile : tiles) {
		glm::ivec3 otherTileMaxPoint = otherTile->maxVert;
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
	TileSubType connectionTileType = tile->sideInfos.connectedTiles[sideIndex]->type;
	return TILE_VISIBILITY[tile->type][sideIndex][connectionTileType];
}

// Will return a value [0-4] denoting the potential tile connection's 'visibility.'  Higher 
// values are more visible, meaning that if we have to choose between two possible connections, 
// the one with the higher visibility will win out.  subjectType/subjectSideIndex corrospond
// to one tile, and otherType is the tile type of the other tile being connected to subject.
// *Note that the index follows Tile (not DrawTile) ordering.
const int tileVisibility(TileSubType subjectType, int subjectSideIndex, TileSubType otherType) {
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
		subject->sideInfos.connectedTiles[subjectConnectionIndex] = other;
		subject->sideInfos.connectedSideIndices[subjectConnectionIndex] = otherConnections[0];
		subject->sideInfos.connectionsMirrored[subjectConnectionIndex] = isMirroredConnection;
		// If one tile sees the other, the other tile must see it, and they will have the same winding:
		other->sideInfos.connectedTiles[otherConnections[0]] = subject;
		other->sideInfos.connectedSideIndices[otherConnections[0]] = subjectConnections[0];
		other->sideInfos.connectionsMirrored[otherConnections[0]] = isMirroredConnection;
		return true;
	}
	return false;
}

void TileManager::drawTile(std::vector<glm::vec2> tileVerts, std::vector<glm::vec2> tileTexCoords, glm::vec4 tileColor) {
	// prepare the tile:
	verts.clear();
	indices.clear();
	int indexOffset = (int)verts.size() / 11;
	for (int i = 0; i < tileVerts.size(); i++) {
		// pos:
		verts.push_back(tileVerts[i].x);
		verts.push_back(tileVerts[i].y);
		verts.push_back(0);
		// normal:
		verts.push_back(0.0f);
		verts.push_back(0.0f);
		verts.push_back(1.0f);
		// color:
		verts.push_back(tileColor.r);
		verts.push_back(tileColor.g);
		verts.push_back(tileColor.b);
		// texture coord:
		verts.push_back(tileTexCoords[i].x);
		verts.push_back(tileTexCoords[i].y);
	}
	for (int i = 0; i < tileVerts.size() - 2; i++) {
		indices.push_back(indexOffset);
		indices.push_back(indexOffset + i + 1);
		indices.push_back(indexOffset + i + 2);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLuint alphaID = glGetUniformLocation(p_shaderManager->simpleShader.ID, "inAlpha");
	glUniform1f(alphaID, tileColor.a);

	GLuint colorAlphaID = glGetUniformLocation(p_shaderManager->simpleShader.ID, "inColorAlpha");
	glUniform1f(colorAlphaID, 0.5f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verts.size(), verts.data(), GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
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

	windowFrustumDiagonalLength = glm::distance(windowFrustum[0], windowFrustum[2]);
}

TileTarget TileManager::adjustTileTarget(TileTarget* currentPov, int drawTileSideIndex) {
	int newInitialSideIndex,
		newInitialTexIndex,
		newSideInfosOffset,
		connectionIndex = currentPov->sideIndex(drawTileSideIndex);
	Tile* newTarget;

	if (currentPov->tile->sideInfos.connectionsMirrored[connectionIndex]) {
		newSideInfosOffset = (currentPov->sideInfosOffset + 2) % 4;
	}
	else {
		newSideInfosOffset = currentPov->sideInfosOffset;
	}

	newInitialSideIndex = currentPov->tile->sideInfos.connectedSideIndices[connectionIndex];
	newInitialSideIndex += VERT_INFO_OFFSETS[drawTileSideIndex] * newSideInfosOffset;
	newInitialSideIndex %= 4;

	if (newSideInfosOffset == 3) {
		newInitialTexIndex = (newInitialSideIndex + 1) % 4;
	}
	else {
		newInitialTexIndex = newInitialSideIndex;
	}

	newTarget = currentPov->tile->sideInfos.connectedTiles[connectionIndex];

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

void TileManager::collisionDetectUnsafeCorners() {
	int cornerIndex1, cornerIndex2;
	glm::vec2 closestCorner(0, 0);
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
		if (distToCorner < 0.25f) {
			float scale = (0.25f - distToCorner) / 0.25f;
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
	case TileSubType::TILE_TYPE_XY_FRONT: rotate = glm::mat4(1); break;
	case TileSubType::TILE_TYPE_XY_BACK: rotate = glm::rotate(glm::mat4(1), float(M_PI), glm::vec3(0, 1, 0)); break;
	case TileSubType::TILE_TYPE_XZ_FRONT: rotate = glm::rotate(glm::mat4(1), float(M_PI / 2.0f), glm::vec3(1, 0, 0)); break;
	case TileSubType::TILE_TYPE_XZ_BACK: rotate = glm::rotate(glm::mat4(1), -float(M_PI / 2.0f), glm::vec3(1, 0, 0)); break;
	case TileSubType::TILE_TYPE_YZ_FRONT: rotate = glm::rotate(glm::mat4(1), -float(M_PI / 2.0f), glm::vec3(0, 1, 0)); break;
	case TileSubType::TILE_TYPE_YZ_BACK: rotate = glm::rotate(glm::mat4(1), float(M_PI / 2.0f), glm::vec3(0, 1, 0)); break;
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

	glm::vec3 povTileNormal = povTile.tile->normal();
	glm::vec3 adjPovTileNormal = glm::vec3(rotate * glm::vec4(povTileNormal, 1));
	glm::vec3 targetNormal = povTileNormal;
	glm::vec3 targetNormalAdj(0, 0, 0);
	glm::vec3 flippedNormalAdj(0, 0, 0);
	TileTarget target;
	if (p_camera->viewPlanePos.x > 0.5f) {
		glm::vec3 neighborNormal = adjustTileTarget(&povTile, 0).tile->normal();
		if (neighborNormal == -povTileNormal) {
			targetNormal *= -((p_camera->viewPlanePos.x - 0.5f) * 2.0f - 1.0f);
			flippedNormalAdj += glm::vec3(1, 0, 0) * (p_camera->viewPlanePos.x - 0.5f) * 2.0f;
		}
		else if (neighborNormal != povTileNormal) {
			targetNormalAdj += neighborNormal * (p_camera->viewPlanePos.x - 0.5f) * 2.0f;
		}
	}
	else {
		glm::vec3 neighborNormal = adjustTileTarget(&povTile, 2).tile->normal();
		if (neighborNormal == -povTileNormal) {
			targetNormal *= p_camera->viewPlanePos.x * 2.0f;
			flippedNormalAdj += glm::vec3(-1, 0, 0) * -((p_camera->viewPlanePos.x * 2.0f) - 1.0f);
		}
		else if (neighborNormal != povTileNormal) {
			targetNormalAdj += neighborNormal * -((p_camera->viewPlanePos.x * 2.0f) - 1.0f);
		}
	}
	if (p_camera->viewPlanePos.y > 0.5f) {
		glm::vec3 neighborNormal = adjustTileTarget(&povTile, 3).tile->normal();
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
		glm::vec3 neighborNormal = adjustTileTarget(&povTile, 1).tile->normal();
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

void TileManager::deleteBuilding(Tile* tile) {
	// We dont have to care about manually deleting materials as they are not referenced by anything besides the
	// tile itsef, which is being deleted anyway.  Some entities are buildings though and have vectors of structs
	// corrosponding to them.  Those we have to find and delete so they are no longer referencing a tile that 
	// doesn't exist.
	/*if (tile->hasEntity() == false) {
		return;
	}
	switch (tile->entity->type) {
	case EntityType::BUILDING_COMPRESSOR:

		break;
	case EntityType::BUILDING_FORCE_BLOCK:

		break;
	case EntityType::BUILDING_FORCE_MIRROR:

		break;
	}

	switch (tile->basis.type) {
	case BASIS_TYPE_PRODUCER:
		for (int i = 0; i < producers.size(); i++) {
			if (producers[i].tileIndex == tile->index) {
				producers[i] = producers[producers.size() - 1];
				producers.pop_back();
			}
		}
		break;
	case BASIS_TYPE_CONSUMER:
		for (int i = 0; i < consumers.size(); i++) {
			if (consumers[i].tileIndex == tile->index) {
				consumers[i] = consumers[consumers.size() - 1];
				consumers.pop_back();
			}
		}
		break;
	}*/
}



void TileManager::deleteTile(Tile* tile) {
	// Stuff will break if you delete the tile you are on.  Don't do that:
	if (tile == povTile.tile || tile->sibling == povTile.tile) {
		return;
	}

	Tile* sibling = tile->sibling;
	if (sibling == nullptr) { // Just in case.
		std::cout << "NO SIBLING FOUND TO DELETE!" << std::endl;
		return;
	}

	deleteBuilding(tile);
	deleteBuilding(sibling);

	// Gather up all the info needed to reconnect neighbor tiles to the map after removing the tile pair:
	Tile* connectedTiles[8];
	int connectedTileIndices[8];
	for (int i = 0; i < 4; i++) {
		connectedTiles[i] = tile->sideInfos.connectedTiles[i];
		connectedTiles[i + 4] = sibling->sideInfos.connectedTiles[i];
		connectedTileIndices[i] = tile->sideInfos.connectedSideIndices[i];
		connectedTileIndices[i + 4] = sibling->sideInfos.connectedSideIndices[i];
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
		connectedTiles[i]->sideInfos.connectionsMirrored[connectedTileIndices[i]] = true;
		connectedTiles[i]->sideInfos.connectedSideIndices[connectedTileIndices[i]] = connectedTileIndices[i];
		connectedTiles[i]->sideInfos.connectedTiles[connectedTileIndices[i]] = connectedTiles[i]->sibling;
		connectUpNewTile(connectedTiles[i]);
	}
	// The index values stored in the tiles are messed up, so we need to update the gpu info for all the messed
	// up tiles:
	//updateTileGpuInfoIndices();

	delete tile;
	delete sibling;
}

// If going from one tile to another, the direction headed could change, as the orientation of the tiles
// themselves have also changed, so in order for the direction to remain constant form one coordinate space to
// the next, it may have to be altered.  This function does that.  Given a tile and a direction from that tile,
// it will return the corrisponding direction once translated into the neghboring tile.
int adjustedDirection(Tile* initialTile, int initialDirection) {
	int orientationOffset = initialDirection - (initialTile->sideInfos.connectedSideIndices[initialDirection] + 2) % 4;
	if (orientationOffset < 0) {
		orientationOffset *= -3;
	}
	return (initialDirection + orientationOffset*3) % 4;
}

int inverseDirection(int dir) {
	return (dir + 2) % 4;
}

void TileManager::update() {
	collisionDetectUnsafeCorners();
	updatePovTileTarget();
	update3dRotationAdj();
	updateWindowFrustum();

	verts.clear();
	indices.clear();
	drawnTiles = 0;
	TOTAL_TIME = 0;
	
	getRelativePovPosGpuInfos();
}

// In order to show where the player is in the 2D 3rd person POV view, we send some relative positional data
// to the GPU.  The positions are relative to drawVert[2] as origin with drawVert[2]->drawVert[1] acting as the
// 'x' direction and drawVert[2]->drawVert[3] acting as the 'y' direction.  Becuase the player is no larger than
// a single tile, a maximum of 4 tiles must have relative position data.  In the shader, each pixel knows what
// tile it is in, so it queries this info using that index to see if it is actually inside the player, then colors
// accordingly.
void TileManager::getRelativePovPosGpuInfos() {
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
			return relativePos[2] = glm::vec2(0, 0);
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
			return relativePos[2] = glm::vec2(0, 0);
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
			return relativePos[2] = glm::vec2(0, 0);
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
			return relativePos[2] = glm::vec2(0, 0);
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

void TileManager::updateTileGpuInfoIndices() {
	tileGpuInfos.clear();
	for (Tile* tile : tiles) {
		tileGpuInfos.push_back(TileGpuInfo(tile));
	}
}

void TileManager::draw2D3rdPerson() {
	glViewport(0, 0, WindowSize.x, WindowSize.y);
	drawSetup();

	const float INITIAL_OPACITY = 0.5f;
	bool previousSides[4] = { 0, 0, 0, 0 };

	// Draw the povTile itself to start:
	std::vector<glm::vec2> croppedDrawTileTexCoords = {
		povTile.tile->sideInfos.texCoords[(povTile.initialVertIndex + povTile.sideInfosOffset * 0) % 4],
		povTile.tile->sideInfos.texCoords[(povTile.initialVertIndex + povTile.sideInfosOffset * 1) % 4],
		povTile.tile->sideInfos.texCoords[(povTile.initialVertIndex + povTile.sideInfosOffset * 2) % 4],
		povTile.tile->sideInfos.texCoords[(povTile.initialVertIndex + povTile.sideInfosOffset * 3) % 4],
	};
	std::vector<glm::vec2> povTileDrawVerts = {
		glm::vec2(1, 1),glm::vec2(1, 0),glm::vec2(0, 0),glm::vec2(0, 1)
	};
	drawTile(INITIAL_DRAW_TILE_VERTS, croppedDrawTileTexCoords, glm::vec4(povTile.tile->color, INITIAL_OPACITY));

	// Start the recursive call to draw each tile connected to the eye tile edges:
	for (int drawTileSideIndex = 0; drawTileSideIndex < 4; drawTileSideIndex++) {
		glm::vec2 newFrustum[3] = {
			glm::normalize(INITIAL_DRAW_TILE_VERTS[(drawTileSideIndex) % 4]
						   - glm::vec2(p_camera->viewPlanePos)),
			glm::vec2(0, 0),
			glm::normalize(INITIAL_DRAW_TILE_VERTS[(drawTileSideIndex + 1) % 4]
						   - glm::vec2(p_camera->viewPlanePos)),
		};

		// After a draw tile moves in a direction, it should never need to move back 
		// in that direction again, thus we can make sure it doesnt with this bool array:
		bool newPreviousSides[4]{};
		newPreviousSides[(drawTileSideIndex + 2) % 4] = true;

		int newSideOffset, newInitialSideIndex, newInitialTexIndex;
		int sideIndex = (povTile.initialSideIndex + povTile.sideInfosOffset * drawTileSideIndex) % 4;
		Tile::SideInfos* currentSideInfo = &povTile.tile->sideInfos;
		// The next draw tile can be either mirrored or unmirrored.  Mirrored tiles are 
		// wound the opposite way and thus have an opposite side index offset.  Unmirrored 
		// tiles have the same winding, so no adjustment is necessary.  As the side index 
		// is in the domain of [0,3], we can also just wrap around from 3 -> 0 with % 4.
		if (currentSideInfo->connectionsMirrored[sideIndex]) {
			newSideOffset = (povTile.sideInfosOffset + 2) % 4;
		}
		else { // Current connection is unmirrored:
			newSideOffset = povTile.sideInfosOffset;
		}
		newInitialSideIndex = (currentSideInfo->connectedSideIndices[sideIndex]
			+ VERT_INFO_OFFSETS[drawTileSideIndex] * newSideOffset) % 4;
		newInitialTexIndex = newInitialSideIndex;
		if (newSideOffset == 3) {
			// then the next tile will be wound counterclockwise, and it's initial side 
			// index will key into the *top right* tex coord instead of the top left.  
			// Because the winding is counterclockwise, we can adjust the initial tex 
			// coord by incrementing it once, going from the top right to the top left!
			newInitialTexIndex = (newInitialTexIndex + 1) % 4;
		}
		std::vector<glm::vec2> newTileVerts
			= createNewDrawTileVerts(INITIAL_DRAW_TILE_VERTS, DRAW_TILE_OFFSETS[drawTileSideIndex]);

		// Tiles that change angle will change opacity or 'tint' so that it 
		// can be noticed with traversing 3D space from this perspective:
		float newTileOpacity;
		// We want a smooth transition from one tile opacity to another, so
		// it should fade as you get closer to the next draw tile's edge:
		float edgeDist = distToLineSeg((glm::vec2)p_camera->viewPlanePos,
			INITIAL_DRAW_TILE_VERTS[drawTileSideIndex],
			INITIAL_DRAW_TILE_VERTS[(drawTileSideIndex + 1) % 4],
			nullptr);
		newTileOpacity = INITIAL_OPACITY;
		if (currentSideInfo->connectedTiles[sideIndex]->type != povTile.tile->type) {
			newTileOpacity -= DRAW_TILE_OPACITY_DECRIMENT_STEP;
		}
		if (edgeDist < 0.5f && currentSideInfo->connectedTiles[sideIndex]->type != povTile.tile->type) {
			newTileOpacity += (-((edgeDist * 2) - 1)) * 0.1f;
		}

		// Finally!  We can actually go onto drawing the next tile:
		drawTiles(currentSideInfo->connectedTiles[sideIndex], newTileVerts,
			newInitialSideIndex, newInitialTexIndex, newSideOffset,
			newFrustum, newPreviousSides, newTileOpacity);
		//break;
	}


	/*auto end = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::milliseconds::period>(end - start).count();
	TOTAL_TIME += time;*/
	//std::cout << TOTAL_TIME << std::endl;

	drawCleanup();
}

void TileManager::drawTiles(Tile* tile, std::vector<glm::vec2>& drawTileVerts,
	int initialSideIndex, int initialTexIndex, int tileVertInfoOffset,
	glm::vec2 frustum[3], bool previousSides[4], float tileOpacity) {

	if (!tileOnScreen(drawTileVerts) || tileOpacity <= 0 || drawnTiles > MAX_DRAW_TILES) {
		return;
	}

	std::vector<glm::vec2> croppedDrawTileVerts = createNewDrawTileVerts(drawTileVerts, glm::vec2(0, 0));
	std::vector<glm::vec2> croppedDrawTileTexCoords = {
		tile->sideInfos.texCoords[initialTexIndex],
		tile->sideInfos.texCoords[(initialTexIndex + tileVertInfoOffset) % 4],
		tile->sideInfos.texCoords[(initialTexIndex + tileVertInfoOffset * 2) % 4],
		tile->sideInfos.texCoords[(initialTexIndex + tileVertInfoOffset * 3) % 4],
	};

	frustum[0] += (glm::vec2)p_camera->viewPlanePos;
	frustum[1] += (glm::vec2)p_camera->viewPlanePos;
	frustum[2] += (glm::vec2)p_camera->viewPlanePos;
	cropTileToFrustum(croppedDrawTileVerts, croppedDrawTileTexCoords, frustum);
	frustum[0] -= (glm::vec2)p_camera->viewPlanePos;
	frustum[1] -= (glm::vec2)p_camera->viewPlanePos;
	frustum[2] -= (glm::vec2)p_camera->viewPlanePos;

	if (croppedDrawTileVerts.size() < 3) {
		return;
	}
	drawTile(croppedDrawTileVerts, croppedDrawTileTexCoords, glm::vec4(tile->color, tileOpacity));
	drawnTiles++;

	for (int drawTileSideIndex = 0; drawTileSideIndex < 4; drawTileSideIndex++) {
		if (previousSides[drawTileSideIndex]) {
			continue;
		}
		glm::vec2 newFrustum[3] = {
			glm::normalize(drawTileVerts[drawTileSideIndex] - glm::vec2(p_camera->viewPlanePos)),
			glm::vec2(0, 0),
			glm::normalize(drawTileVerts[(drawTileSideIndex + 1) % 4] - glm::vec2(p_camera->viewPlanePos)),
		};
		// initial case has frustum verts = (0, 0), so we make sure to init frustum from that:
		if (frustum[0] != frustum[2]) {
			if (!vecInsideVecs(newFrustum[0], frustum[0], frustum[2])) {
				newFrustum[0] = frustum[0];
			}
			if (!vecInsideVecs(newFrustum[2], frustum[0], frustum[2])) {
				newFrustum[2] = frustum[2];
			}
		}
		// After a draw tile moves in a direction, it should never need to move back 
		// in that direction again, thus we can make sure it doesnt with this bool array:
		bool newPreviousSides[] = {
			previousSides[0], previousSides[1], previousSides[2], previousSides[3]
		};
		newPreviousSides[(drawTileSideIndex + 2) % 4] = true;

		int newSideOffset, newInitialSideIndex, newInitialTexIndex;
		int sideIndex = (initialSideIndex + tileVertInfoOffset * drawTileSideIndex) % 4;
		// The next draw tile can be either mirrored or unmirrored.  Mirrored tiles are 
		// wound the opposite way and thus have an opposite side index offset.  Unmirrored 
		// tiles have the same winding, so no adjustment is necessary.  As the side index 
		// is in the domain of [0,3], we can also just wrap around from 3 -> 0 with % 4.
		if (tile->sideInfos.connectionsMirrored[sideIndex]) {
			newSideOffset = (tileVertInfoOffset + 2) % 4;
		}
		else { // Current connection is unmirrored:
			newSideOffset = tileVertInfoOffset;
		}
		newInitialSideIndex = (tile->sideInfos.connectedSideIndices[sideIndex]
			+ VERT_INFO_OFFSETS[drawTileSideIndex] * newSideOffset) % 4;
		newInitialTexIndex = newInitialSideIndex;
		if (newSideOffset == 3) {
			// then the next tile will be wound counterclockwise, and it's initial side 
			// index will key into the *top right* tex coord instead of the top left.  
			// Because the winding is counterclockwise, we can adjust the initial tex 
			// coord by incrementing it once, going from the top right to the top left!
			newInitialTexIndex = (newInitialTexIndex + 1) % 4;
		}
		std::vector<glm::vec2> newTileVerts
			= createNewDrawTileVerts(drawTileVerts, DRAW_TILE_OFFSETS[drawTileSideIndex]);
		// Tiles that change angle will change opacity or 'tint' so that it 
		// can be noticed with traversing 3D space from this perspective:
		float newTileOpacity = tileOpacity;
		// We want a smooth transition from one tile opacity to another, so
		// it should fade as you get closer to the next draw tile's edge:
		if (tile->sideInfos.connectedTiles[sideIndex]->type != tile->type) {
			float edgeDist = distToLineSeg((glm::vec2)p_camera->viewPlanePos,
				drawTileVerts[drawTileSideIndex],
				drawTileVerts[(drawTileSideIndex + 1) % 4], nullptr);
			if (edgeDist > 0.5) {
				newTileOpacity -= DRAW_TILE_OPACITY_DECRIMENT_STEP;
			}
			else {
				newTileOpacity = tileOpacity - (edgeDist * 2) * DRAW_TILE_OPACITY_DECRIMENT_STEP;
			}
		}

		// Finally!  We can actually go onto drawing the next tile:
		drawTiles(tile->sideInfos.connectedTiles[sideIndex], newTileVerts,
			newInitialSideIndex, newInitialTexIndex, newSideOffset,
			newFrustum, newPreviousSides, newTileOpacity);
	}
}

// DOESN'T DO SHIT FOR NOW, I BROKE IT:
void TileManager::drawAddTilePreview() {
	std::vector<glm::vec2> previewTileVerts = {
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		glm::vec2(0, 1)
	};
	std::vector<glm::vec2> previewTileTexCoords = {
		glm::vec2(0,0),
		glm::vec2(1,0),
		glm::vec2(1,1),
		glm::vec2(0,1)
	};

	/*for (glm::vec2& v : previewTileVerts) { v += drawTileOffset; }
	drawTile(previewTileVerts, previewTileTexCoords, glm::vec4(0, 1, 0, 1));*/
}

void TileManager::drawSetup() {
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	glBindVertexArray(p_framebuffer->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, p_framebuffer->VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_framebuffer->EBO);

	setVertAttribVec3PosVec3NormVec3ColorVec2TextCoord1Index();
	p_shaderManager->POV3D3rdPerson.use();
}

void TileManager::draw3DTile(Tile* tile) {
	// prepare the tile:
	verts.clear();
	indices.clear();
	for (int i = 0; i < 4; i++) {
		// pos:
		verts.push_back((GLfloat)tile->getVertPos(i).x);
		verts.push_back((GLfloat)tile->getVertPos(i).y);
		verts.push_back((GLfloat)tile->getVertPos(i).z);
		// normal:
		verts.push_back(0.0f);
		verts.push_back(0.0f);
		verts.push_back(1.0f);
		// color:
		verts.push_back((GLfloat)tile->color.r);
		verts.push_back((GLfloat)tile->color.g);
		verts.push_back((GLfloat)tile->color.b);
		// texture coord:
		verts.push_back(tile->sideInfos.texCoords[i].x);
		verts.push_back(tile->sideInfos.texCoords[i].y);
		// tile index:
		verts.push_back((GLfloat)tile->index);
	}

	if (tile->type == TileSubType::TILE_TYPE_XY_BACK ||
		tile->type == TileSubType::TILE_TYPE_XZ_BACK ||
		tile->type == TileSubType::TILE_TYPE_YZ_BACK) {
		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(3);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(3);
	}
	else {
		indices.push_back(3);
		indices.push_back(1);
		indices.push_back(0);
		indices.push_back(3);
		indices.push_back(2);
		indices.push_back(1);
	}

	GLuint alphaID = glGetUniformLocation(p_shaderManager->POV3D3rdPerson.ID, "inAlpha");
	glUniform1f(alphaID, 1.0f);

	GLuint colorAlphaID = glGetUniformLocation(p_shaderManager->POV3D3rdPerson.ID, "inColorAlpha");
	glUniform1f(colorAlphaID, 0.5f);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verts.size(), verts.data(), GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
}

void TileManager::drawPlayerPos() {
	drawSetup();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);

	// Temp player location:
	std::vector<glm::vec2> v = {
		glm::vec2(-0.1,-0.1),
		glm::vec2(+0.1,-0.1),
		glm::vec2(+0.1,+0.1),
		glm::vec2(-0.1,+0.1)
	};
	for (glm::vec2& vert : v) { vert += (glm::vec2)p_camera->viewPlanePos; }
	std::vector<glm::vec2> t = {
		glm::vec2(0,0),
		glm::vec2(1,0),
		glm::vec2(1,1),
		glm::vec2(0,1)
	};

	drawTile(v, t, glm::vec4(1, 1, 1, 1));
}

void getProjectedPlayerPosInfo(TileTarget& target, glm::vec2 P, int index, float rightOffset, float upOffset,
	glm::vec3* projectedTilePositions, int* tileIndices) {
	glm::vec3 bottomRight = target.tile->getVertPos(target.vertIndex(2));
	glm::vec3 rightward = (glm::vec3)target.tile->getVertPos(target.vertIndex(1)) - bottomRight;
	glm::vec3 upward = (glm::vec3)target.tile->getVertPos(target.vertIndex(3)) - bottomRight;

	projectedTilePositions[index] = bottomRight
		+ rightward * rightOffset
		+ upward * upOffset
		+ rightward * P.x
		+ upward * P.y;
	tileIndices[index] = target.tile->index;
}

glm::mat4 TileManager::packedPlayerPosInfo() {
	glm::vec3 projectedTilePositions[4];
	int tileIndices[4];
	TileTarget target;
	glm::vec3 P = p_camera->viewPlanePos;

	projectedTilePositions[0] = getPovTilePos();
	tileIndices[0] = povTile.tile->index;

	if (p_camera->viewPlanePos.x > 0.5f) {
		target = adjustTileTarget(&povTile, LocalDirection::LOCAL_DIRECTION_0);
		getProjectedPlayerPosInfo(target, P, 1, -1.0f, 0.0f, projectedTilePositions, tileIndices);

		if (p_camera->viewPlanePos.y > 0.5f) {
			target = adjustTileTarget(&target, LocalDirection::LOCAL_DIRECTION_3);
			getProjectedPlayerPosInfo(target, P, 3, -1.0f, -1.0f, projectedTilePositions, tileIndices);
		}
		else {
			target = adjustTileTarget(&target, LocalDirection::LOCAL_DIRECTION_1);
			getProjectedPlayerPosInfo(target, P, 3, -1.0f, 1.0f, projectedTilePositions, tileIndices);
		}
	}
	else {
		target = adjustTileTarget(&povTile, LocalDirection::LOCAL_DIRECTION_2);
		getProjectedPlayerPosInfo(target, P, 1, 1.0f, 0.0f, projectedTilePositions, tileIndices);

		if (p_camera->viewPlanePos.y > 0.5f) {
			target = adjustTileTarget(&target, LocalDirection::LOCAL_DIRECTION_3);
			getProjectedPlayerPosInfo(target, P, 3, 1.0f, -1.0f, projectedTilePositions, tileIndices);
		}
		else {
			target = adjustTileTarget(&target, LocalDirection::LOCAL_DIRECTION_1);
			getProjectedPlayerPosInfo(target, P, 3, 1.0f, 1.0f, projectedTilePositions, tileIndices);
		}
	}
	if (p_camera->viewPlanePos.y > 0.5f) {
		target = adjustTileTarget(&povTile, LocalDirection::LOCAL_DIRECTION_3);
		getProjectedPlayerPosInfo(target, P, 2, 0.0f, -1.0f, projectedTilePositions, tileIndices);
	}
	else {
		target = adjustTileTarget(&povTile, LocalDirection::LOCAL_DIRECTION_1);
		getProjectedPlayerPosInfo(target, P, 2, 0.0f, 1.0f, projectedTilePositions, tileIndices);
	}

	glm::mat4 playerPosInfo = {
		projectedTilePositions[0].x, projectedTilePositions[0].y, projectedTilePositions[0].z, tileIndices[0],
		projectedTilePositions[1].x, projectedTilePositions[1].y, projectedTilePositions[1].z, tileIndices[1],
		projectedTilePositions[2].x, projectedTilePositions[2].y, projectedTilePositions[2].z, tileIndices[2],
		projectedTilePositions[3].x, projectedTilePositions[3].y, projectedTilePositions[3].z, tileIndices[3],
	};

	return playerPosInfo;
}

void TileManager::draw3Dview() {
	drawSetup();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glPolygonMode(GL_FRONT, GL_FILL);
	glEnable(GL_CULL_FACE);

	Button* button = &p_buttonManager->buttons[ButtonManager::pov3d3rdPersonViewButtonIndex];
	tempMat = p_camera->getPerspectiveProjectionMatrix((float)button->pixelWidth(),
		(float)button->pixelHeight());

	glm::mat4 xMirror(1);
	xMirror[0][0] = -1;
	tempMat = xMirror * tempMat * tileRotationAdjFor3DView;
	GLuint transfMatrixID = glGetUniformLocation(p_shaderManager->POV3D3rdPerson.ID, "inTransfMatrix");
	glUniformMatrix4fv(transfMatrixID, 1, GL_FALSE, glm::value_ptr(tempMat));

	GLuint playerPosInfoID = glGetUniformLocation(p_shaderManager->POV3D3rdPerson.ID, "inPlayerPosInfo");
	glUniformMatrix4fv(playerPosInfoID, 1, GL_FALSE, glm::value_ptr(packedPlayerPosInfo()));

	glm::vec3 playerPos = getPovTilePos();
	playerPos = glm::vec3(tempMat * glm::vec4(playerPos, 1));
	GLuint playerPosID = glGetUniformLocation(p_shaderManager->POV3D3rdPerson.ID, "inPlayerPos");
	glUniform3f(playerPosID, playerPos.x, playerPos.y, playerPos.z);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	for (Tile* t : tiles) {
		draw3DTile(t);
	}

	drawCleanup();
}

std::vector<glm::vec2> TileManager::createNewDrawTileVerts(std::vector<glm::vec2>& parent,
	glm::vec2 adj) {
	std::vector<glm::vec2> newDrawTile;
	for (int i = 0; i < parent.size(); i++) {
		newDrawTile.push_back(parent[i] + adj);
	}
	return newDrawTile;
}

// True if B is 'inside' or 'between' A and C.
bool TileManager::vecInsideVecs(glm::vec2 A, glm::vec2 B, glm::vec2 C) {
	return (A.y * B.x - A.x * B.y) * (A.y * C.x - A.x * C.y) < 0;
}

bool TileManager::tileOnScreen(std::vector<glm::vec2>& tileVerts) {
	for (glm::vec2 v : tileVerts) {
		if (point_in_polygon(v, windowFrustum)) {
			return true;
		}
	}
	for (int wfi = 0; wfi < 4; wfi++) {
		for (int ti = 0; ti < tileVerts.size(); ti++) {
			if (doIntersect(tileVerts[ti], tileVerts[(ti + 1) % 4],
				windowFrustum[wfi], windowFrustum[(wfi + 1) % 4])) {
				return true;
			}
		}
	}
	return false;
}

void TileManager::drawCleanup() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}