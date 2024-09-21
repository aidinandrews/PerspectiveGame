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
	glm::vec3 frontTileColor, glm::vec3 backTileColor) 
{
	using namespace tnav;

	Tile* foreTile = new Tile(getTileType(tileType, true), maxPoint);
	Tile* backTile = new Tile(getTileType(tileType, false), maxPoint);

	// before connecting everything up, it is importand that this new tile pair 
	// does not overlap another tile pair, as that would be against the rules:
	if (!tileIsUnique(*foreTile)) {
		return false;
	}

	foreTile->sibling = backTile;
	foreTile->type = TileType(tileType * 2);
	foreTile->color = frontTileColor;

	backTile->sibling = foreTile;
	backTile->type = TileType(tileType * 2 + 1);
	backTile->color = backTileColor;

	updateNeighborConnections(foreTile);
	updateNeighborConnections(backTile);

	updateCornerSafety(foreTile);
	updateCornerSafety(backTile);
	for (int i = 0; i < 4; i++) {
		Tile* neighbor1 = foreTile->neighbors[i];
		int mapIndex = foreTile->neighborAlignmentMaps[i];
		Tile* neighbor1a = neighbor1->neighbors[tnav::getMappedAlignment(mapIndex, LocalDirection((i + 1) % 4))];
		Tile* neighbor1b = neighbor1->neighbors[tnav::getMappedAlignment(mapIndex, LocalDirection((i + 3) % 4))];
		updateCornerSafety(neighbor1);
		updateCornerSafety(neighbor1a);
		updateCornerSafety(neighbor1b);

		neighbor1 = backTile->neighbors[i];
		mapIndex = backTile->neighborAlignmentMaps[i];
		neighbor1a = neighbor1->neighbors[tnav::getMappedAlignment(mapIndex, LocalDirection((i + 1) % 4))];
		neighbor1b = neighbor1->neighbors[tnav::getMappedAlignment(mapIndex, LocalDirection((i + 3) % 4))];
		updateCornerSafety(neighbor1);
		updateCornerSafety(neighbor1a);
		updateCornerSafety(neighbor1b);
	}

	createMetaNodeConnections(foreTile, backTile);

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

void TileManager::removeConnectedMetaNodes(Tile* tile, Tile* sibling, std::vector<SuperPosition*>& affectedNodes)
{
	bool lastDirCornersChecked = false;
	for (LocalDirection dir = LOCAL_DIRECTION_0; dir < 4; dir = LocalDirection(dir + 1)) {
		Tile* neighbor = tile->neighbors[dir];
		if (neighbor == sibling) {
			continue; // There was no node there yet anyway.
		}
		LocalDirection neighborDirection = tile->mapAlignmentToNeighbor(dir, tnav::oppositeAlignment(dir));
		SideSuperPosition* sideNode = (SideSuperPosition*)neighbor->metaNodes[neighborDirection];

		for (LocalDirection d : tnav::NON_STATIC_LOCAL_DIRECTION_LIST) {
			if (sideNode->getNeighbor(d) == nullptr) { continue; }
			affectedNodes.push_back(sideNode->getNeighbor(d));
		}

		// We also need to get the connected corner nodes:
		// We only need to check 2 of the neighbors as it is impossible 
		// for an overlapping meta node to exist in the corners here:
		if (lastDirCornersChecked == false) {
			CornerSuperPosition* cornerNode = (CornerSuperPosition*)neighbor->metaNodes[neighborDirection + 4];
			for (LocalDirection d : tnav::NON_STATIC_LOCAL_DIRECTION_LIST) {
				if (cornerNode->getNeighbor(d) == nullptr) { continue; }
				affectedNodes.push_back(cornerNode->getNeighbor(d));
			}
			cornerNode = (CornerSuperPosition*)neighbor->metaNodes[((neighborDirection + 3) % 4) + 4];
			for (LocalDirection d : tnav::NON_STATIC_LOCAL_DIRECTION_LIST) {
				if (cornerNode->getNeighbor(d) == nullptr) { continue; }
				affectedNodes.push_back(cornerNode->getNeighbor(d));
			}
			lastDirCornersChecked = true;
		}
		else {
			lastDirCornersChecked = false;
		}

		p_metaNodeNetwork->removeNode(neighbor->metaNodes[dir]);
	}
}

void TileManager::createMetaNodeConnections(Tile* tile, Tile* sibling)
{
	std::vector<Tile*> affectedTiles;
	std::vector<LocalDirection> affectedPositions;
	removeConnectedMetaNodes(tile, sibling, affectedTiles, affectedPositions);

	tile->centerMetaNode = p_metaNodeNetwork->add(CenterSuperPosition(tile->index));
	sibling->centerMetaNode = p_metaNodeNetwork->add(CenterSuperPosition(sibling->index));
	
	for (LocalPosition pos = LOCAL_POSITION_0; pos < 4; pos = LocalPosition(pos + 1)) {
		if (tile->neighbors[pos] == sibling) {
			int siblingToNodeMap = sibling->neighborAlignmentMaps[pos];
			SideSuperPosition sideNode(tile->index, pos, ALIGNMENT_MAP_IDENTITY,
									  sibling->index, pos, siblingToNodeMap);
			
			tile->sideMetaNodes[pos] = p_metaNodeNetwork->add(sideNode);
			tile->metaNodeAlignmentMaps[pos] = ALIGNMENT_MAP_IDENTITY;
			
			sibling->sideMetaNodes[pos] = tile->sideMetaNodes[pos];
			sibling->metaNodeAlignmentMaps[pos] = sibling->neighborAlignmentMaps[pos];
		}
		else {
			
		}
	}

	for (LocalPosition pos = LOCAL_POSITION_0_1; pos < LOCAL_POSITION_CENTER; pos = LocalPosition(pos + 1)) {
		if (tile->cornerIsSafe[pos - 4]) {
			LocalDirection toNeighbor0 = tnav::getAlignmentComponents(pos)[0];
			LocalDirection toNeighbor1 = tnav::getAlignmentComponents(pos)[1];
			LocalDirection toNeighbor2 = toNeighbor0;
			toNeighbor2 = tile->mapAlignmentToNeighbor(toNeighbor1, toNeighbor2);
			
			Tile* neighbor0 = tile->neighbors[toNeighbor0];
			Tile* neighbor1 = tile->neighbors[toNeighbor1];
			Tile* neighbor2 = neighbor1->neighbors[toNeighbor2];

			LocalPosition neighbor0Pos = tnav::combineAlignments(tnav::oppositeAlignment(toNeighbor0), toNeighbor1);
			neighbor0Pos = tile->mapAlignmentToNeighbor(toNeighbor0, neighbor0Pos);
			LocalPosition neighbor1Pos = tnav::combineAlignments(tnav::oppositeAlignment(toNeighbor1), toNeighbor0);
			neighbor1Pos = tile->mapAlignmentToNeighbor(toNeighbor1, neighbor1Pos);
			LocalPosition neighbor2Pos = tnav::oppositeAlignment(pos);
			neighbor2Pos = tile->mapAlignmentToNeighbor(toNeighbor1, neighbor2Pos);
			neighbor2Pos = tile->mapAlignmentToNeighbor(toNeighbor2, neighbor2Pos);

			int neighbor0ToTileMap = tnav::inverseAlignmentMapIndex(tile->neighborAlignmentMaps[toNeighbor0]);
			int neighbor1ToTileMap = tnav::inverseAlignmentMapIndex(tile->neighborAlignmentMaps[toNeighbor1]);
			int neighbor2ToTileMap = tnav::inverseAlignmentMapIndex(neighbor1->neighborAlignmentMaps[toNeighbor2]);
			neighbor2ToTileMap = tnav::combineAlignmentMappings(neighbor2ToTileMap, neighbor1ToTileMap);

			CornerSuperPosition sideNode(tile->index, pos, ALIGNMENT_MAP_IDENTITY,
										neighbor0->index, neighbor0Pos, neighbor0ToTileMap,
										neighbor1->index, neighbor1Pos, neighbor1ToTileMap,
										neighbor2->index, neighbor2Pos, neighbor2ToTileMap);
			tile->cornerMetaNodes[pos] = p_metaNodeNetwork->add(sideNode);
			
			tile->metaNodeAlignmentMaps[pos] = ALIGNMENT_MAP_IDENTITY;
			neighbor0->metaNodeAlignmentMaps[neighbor0Pos] = tile->neighborAlignmentMaps[toNeighbor0];
			neighbor1->metaNodeAlignmentMaps[neighbor1Pos] = tile->neighborAlignmentMaps[toNeighbor1];
			neighbor2->metaNodeAlignmentMaps[neighbor2Pos] = tile->neighborAlignmentMaps[toNeighbor2];

			if (neighbor0 == sibling || neighbor1 == sibling) {
				// there is only one corner node connected to the tiles.
				sibling->cornerMetaNodes[pos] = tile->cornerMetaNodes[pos];
			}
			else {
				// another corner node needs to be created, connected to the sibling tile.
			}
		}
	}
}

void TileManager::updateMetaNodeConnections(Tile* tile)
{

}

void TileManager::updateCornerSafety(Tile* tile)
{
	for (int i = 0; i < 4; i++) {
		int cornerIndex = i;
		LocalDirection dir1 = LocalDirection(i);
		LocalDirection dir2 = LocalDirection((dir1 + 3) % 4); // cornerSafety[0] maps to tile corner[0]
		Tile* neighbor1 = tile->neighbors[dir1];
		Tile* neighbor2 = tile->neighbors[dir2];

		if (neighbor1->index == neighbor2->index) {
			tile->cornerIsSafe[i] = Tile::CORNER_UNSAFE;
			continue;
		}

		LocalDirection dir1a = tile->mapAlignmentToNeighbor(dir1, dir2);
		LocalDirection dir2a = tile->mapAlignmentToNeighbor(dir2, dir1);
		Tile* neighborNeighbor1 = neighbor1->neighbors[dir1a];
		Tile* neighborNeighbor2 = neighbor2->neighbors[dir2a];

		if (neighborNeighbor1->index == neighborNeighbor2->index) {
			tile->cornerIsSafe[i] = Tile::CORNER_SAFE;
		}
		else {
			tile->cornerIsSafe[i] = Tile::CORNER_UNSAFE;
		}

	}
}

// Will return a value [0-4] denoting the tile connection's 'visibility.'  Higher values are
// more visible, meaning that if we have to choose between two possible connections, the one
// with the higher visibility will win out.  'sideIndex' is the index to the side who's
// connection visibility will be queried.  
// *Note that the index follows Tile (not DrawTile) ordering.
const int tileVisibility(Tile* tile, int sideIndex) {
	TileType connectionTileType = tile->neighbors[sideIndex]->type;
	return tnav::getTileVisibility(tile->type, LocalDirection(sideIndex), connectionTileType);
}

// Will return a value [0-4] denoting the potential tile connection's 'visibility.'  Higher 
// values are more visible, meaning that if we have to choose between two possible connections, 
// the one with the higher visibility will win out.  subjectType/subjectSideIndex corrospond
// to one tile, and otherType is the tile type of the other tile being connected to subject.
// *Note that the index follows Tile (not DrawTile) ordering.
const int tileVisibility(TileType subjectType, int subjectSideIndex, TileType otherType) {
	return tnav::getTileVisibility(subjectType, LocalDirection(subjectSideIndex), otherType);
}

// Returns true if the other tile is more visible than the current 
// tile connected to the subject's side denoted by subjectSideIndex.
bool tileIsMoreVisible(Tile* subject, int subjectSideIndex,
	Tile* other, int otherSideIndex) {

	const int currentConnectionVisibility = tileVisibility(other, otherSideIndex);
	const int subjectConnectionVisibility = tileVisibility(subject, subjectSideIndex);
	const int newConnectionVisibility = tileVisibility(subject->type, subjectSideIndex, other->type);

	return newConnectionVisibility >= currentConnectionVisibility &&
		newConnectionVisibility > subjectConnectionVisibility;
}

void TileManager::updateNeighborConnections(Tile* tile)
{
	// These act as initializer values, as if the tile is not connected to anything else, it must be connected to its sibling:
	for (int i = 0; i < 4; i++) {
		tile->neighbors[i] = tile->sibling;
		tile->neighborAlignmentMaps[i] = tnav::getNeighborMap(LocalAlignment(i), LocalAlignment(i));
	}

	// Each side of a tile can only even theoredically connect to some types/orientations of tile,
	// so each edge (connectableTiles[X][]) gets a list of the types of tiles it can connect to 
	// (connectableTiles[][X]).

	TileType possibleTileType;
	glm::ivec3 subjectTileMaxPoint = tile->position;
	glm::ivec3 otherTileMaxPoint;
	glm::ivec3 possibleMaxPoint;

	for (Tile* otherTile : tiles) {
		otherTileMaxPoint = otherTile->position;

		for (int sideIndex = 0; sideIndex < 4; sideIndex++) {

			for (int connectionType = 0; connectionType < 3; connectionType++) {

				possibleTileType = tnav::getConnectableTileType(tile->type, sideIndex, connectionType);
				if (otherTile->type != possibleTileType) {
					continue;
				}
				possibleMaxPoint = subjectTileMaxPoint + tnav::getConnectableTileOffset(tile->type, sideIndex, connectionType);
				if (otherTileMaxPoint == possibleMaxPoint) {
					tryConnect(tile, otherTile);
				}
			}
		}
	}
}


bool TileManager::tryConnect(Tile* subject, Tile* other) 
{
	using namespace tnav;

	LocalDirection subjectConnections[2]{};
	LocalDirection otherConnections[2]{};
	int connectionsIndex = 0;
	// Actually find the two matching vertex pairs:
	for (int subjectSideIndex = 0; subjectSideIndex < 4; subjectSideIndex++) {
		for (int otherSideIndex = 0; otherSideIndex < 4; otherSideIndex++) {
			if (subject->getVertPos(subjectSideIndex) == other->getVertPos(otherSideIndex)) {

				subjectConnections[connectionsIndex] = (LocalDirection)subjectSideIndex;
				otherConnections[connectionsIndex] = (LocalDirection)otherSideIndex;
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
	LocalDirection temp;
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

	// Now that we know what side connects to what side, we can see if we actually 
	// want to connect the tiles.  Different tile sub types have different visibility 
	// to other tile sub types depending on the edge of the tile being connected.  
	// We dont want to connect this pair if there is a more visible tile already connected!
	LocalDirection subjectConnectionIndex = subjectConnections[0];
	if (tileIsMoreVisible(subject, subjectConnectionIndex, other, otherConnections[0])) {

		subject->neighbors[subjectConnectionIndex] = other;
		subject->neighborAlignmentMaps[subjectConnectionIndex] = getNeighborMap(subjectConnectionIndex, otherConnections[0]);
		
		other->neighbors[otherConnections[0]] = subject;
		other->neighborAlignmentMaps[otherConnections[0]] = getNeighborMap(otherConnections[0], subjectConnections[0]);
		
		true;
	}
	return false;
}

void TileManager::deleteTilePair(Tile* tile, bool allowDeletePovTile)
{
	// Stuff will break if you delete the tile you are on.  Don't do that:
	if (!allowDeletePovTile && (tile == povTile.tile || tile->sibling == povTile.tile)) {
		return;
	}

	Tile* sibling = tile->sibling;
#ifdef RUNNING_DEBUG
	if (sibling == nullptr) {
		throw std::runtime_error("NO SIBLING FOUND TO DELETE!");
	}
#endif

	// Gather up all the info needed to reconnect neighbor tiles to the map after removing the tile pair:
	std::vector<Tile*> neighbors;
	for (int i = 0; i < 4; i++) {
		if (tile->neighbors[i] != sibling) {
			neighbors.push_back(tile->neighbors[i]);
		}
		if (sibling->neighbors[i] != tile) {
			neighbors.push_back(sibling->neighbors[i]);
		}
	}

	/*Tile* diagonalNeighbors[16];
	for (int i = 0; i < 4; i++) {
		diagonalNeighbors[4*i + 0]= tile->neighbors[i][1];
		diagonalNeighbors[4*i + 1]= tile->neighbors[i][2];
		diagonalNeighbors[4*i + 2]= sibling->neighbors[i][1];
		diagonalNeighbors[4*i + 3]= sibling->neighbors[i][2];
	}*/

	int firstIndex = std::min(tile->index, sibling->index);
	tiles.erase(tiles.begin() + firstIndex);
	tiles.erase(tiles.begin() + firstIndex);
	for (int i = firstIndex; i < tiles.size(); i++) {
		tiles[i]->index -= 2; // <- Because we are deleting two tiles, the indices need to be offset by 2.
	}

	for (Tile* t : neighbors) {
		updateNeighborConnections(t);
	}
	//for (int i = 0; i < 8; i++) {
	//	updateDiagonalNeighborConnections(neighborTilePtrs[i]);
	//}
	//for (int i = 0; i < 16; i++) {
	//	updateDiagonalNeighborConnections(diagonalNeighbors[i]);
	//}

	// Now that the tiles are all connected, we need to make sure that they have up to date info reguarding the
	// corner safety for player and entity movement updating:
	for (Tile* t : neighbors) {
		updateCornerSafety(t);
		// TODO: clean this up a little.  No need to update all these tiles
		for (int j = 0; j < 4; j++) {
			updateCornerSafety(t->neighbors[j]);
		}
	}

	// The index values stored in the tiles are messed up, so we need to update the gpu info as well:
	updateTileGpuInfos();

	delete tile;
	delete sibling;
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

	if (currentPov->tile->is1stDegreeNeighborMirrored(connectionIndex)) {
		newSideInfosOffset = (currentPov->sideInfosOffset + 2) % 4;
	}
	else {
		newSideInfosOffset = currentPov->sideInfosOffset;
	}

	newInitialSideIndex = currentPov->tile->get1stDegreeNeighborConnectedSideIndex(LocalDirection(connectionIndex));
	newInitialSideIndex += VERT_INFO_OFFSETS[drawTileSideIndex] * newSideInfosOffset;
	newInitialSideIndex %= 4;

	if (newSideInfosOffset == 3) {
		newInitialTexIndex = (newInitialSideIndex + 1) % 4;
	}
	else {
		newInitialTexIndex = newInitialSideIndex;
	}

	newTarget = currentPov->tile->neighbors[connectionIndex];

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

void TileManager::update3dRotationAdj() 
{
	using namespace vechelp;

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