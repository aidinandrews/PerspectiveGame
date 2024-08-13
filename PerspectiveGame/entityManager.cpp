#include "entityManager.h"

void EntityManager::createEntity(int tileIndex, EntityID entityType, LocalDirection orientation, bool override) {

	Tile* tile = p_tileManager->tiles[tileIndex];

	if (!override && tile->isObstructed(LOCAL_POSITION_CENTER)) {
		return;
	}

	entities.push_back(Entity(entityType, LOCAL_POSITION_CENTER, LOCAL_DIRECTION_STATIC, orientation,
		0.0f, -1, -1, glm::vec4(randColor(), 1)));
	Entity* newEntity = &entities.back();
	newEntity->index = (int)entities.size() - 1;
	newEntity->tileIndices[0] = tileIndex;

	tile->entityIndices[LOCAL_POSITION_CENTER] = newEntity->index;
	tile->entityInfoIndices[LOCAL_POSITION_CENTER] = 0; // If an entity is only in one tile, info[0] corrisponds to that tile.
	tile->obstructionMask |= TileNavigator::getObstructionMask(LOCAL_POSITION_CENTER);

	if (tile->hasForce() == false) {
		newEntity->staticListIndex = staticEntityIndices.size();
		staticEntityIndices.push_back(newEntity->index);
		return; // A static entity cannot be a follower or leader.
	}
	
	// Make sure to try and connect up a line if entities if needed:
	if (tryAddLeaderCenter(newEntity) == false) {
		int leaderIndex = leaderInfos.size();
		leaderInfos.push_back(LeaderInfo(leaderIndex, newEntity->index, tile->forceLocalDirection, 1));
		newEntity->leaderListIndex = leaderIndex;
	}
	int followerMass = tryAddFollowerCenter(newEntity);
	leaderInfos.back().mass += followerMass;
}

void EntityManager::removeEntityFromLeaderList(Entity* entity) {
	if (entity->hasFollower()) {
		Entity* follower = getFollower(entity);
		leaderInfos[entity->leaderListIndex].direction = follower->localDirections[0];
		if (follower->tileIndices[1] != -1) {
			leaderInfos[entity->leaderListIndex].direction = follower->localDirections[1];
		}
		leaderInfos[entity->leaderListIndex].entityIndex = follower->index;
		leaderInfos[entity->leaderListIndex].mass -= 1;
	}
	else {
		if (entity->leaderListIndex == leaderInfos.size() - 1) {
			leaderInfos.pop_back();
		}
		else {
			int leaderIndex = entity->leaderListIndex;
			std::swap(leaderInfos[leaderIndex], leaderInfos.back());
			leaderInfos[leaderIndex].leaderIndex = leaderIndex;
			entities[leaderInfos[leaderIndex].entityIndex].leaderListIndex = leaderIndex;
			leaderInfos.pop_back();
		}
	}
}

void EntityManager::removeEntityFromStaticList(Entity* entity) {
	if (entity->staticListIndex == staticEntityIndices.size() - 1) {
		staticEntityIndices.pop_back();
	}
	else {
		int staticIndex = entity->staticListIndex;
		std::swap(staticEntityIndices[staticIndex], staticEntityIndices.back());
		entities[staticEntityIndices[staticIndex]].staticListIndex = staticIndex;
		staticEntityIndices.pop_back();
	}
}

void EntityManager::removeEntityFromEntityList(Entity* entity) {
	if (entity->index == entities.size() - 1) {
		entities.pop_back();
	}
	else {
		int entityIndex = entity->index;
		std::swap(*entity, entities.back());
		entities.pop_back();
		Entity* swappedEntity = &entities[entityIndex];
		swappedEntity->index = entityIndex;

		// clean up other connections:
		if (swappedEntity->isStatic()) {
			staticEntityIndices[swappedEntity->staticListIndex] = entityIndex;
		}
		else if (swappedEntity->isLeader()) {
			leaderInfos[swappedEntity->leaderListIndex].entityIndex = entityIndex;
		}
		for (int i = 0; i < 2; i++) {
			if (swappedEntity->connectedToTile(i)) {
				getTile(swappedEntity, i)->entityIndices[swappedEntity->localPositions[i]] = entityIndex;
			}
		}
	}
}

void EntityManager::deleteEntity(Entity* entity) {
	for (int i = 0; i < 2; i++) {
		if (entity->connectedToTile(i)) {
			Tile*  tile = getTile(entity, i);
			tile->entityIndices[entity->localPositions[i]] = -1;
			obstructionMaskChanges.push_back(ObstructionMaskChange(
				tile->index, TileNavigator::getObstructionMask(entity->localPositions[i]), 
				tile->index, 0));
		}
	}

	if (entity->isLeader()) { removeEntityFromLeaderList(entity); }
	if (entity->isStatic()) { removeEntityFromStaticList(entity); }
	removeEntityFromEntityList(entity);
}

int EntityManager::tryAddFollowerCenter(Entity* entity) {
	Tile* tile = getTile0(entity);
	LocalPosition followerPosition = LocalPosition((tile->forceLocalDirection + 2) % 4);

	if (tile->entityIndices[followerPosition] == NO_ENTITY_INDEX) { 
		return 0; 
	}

	Entity* follower = &entities[tile->entityIndices[followerPosition]];
	bool followerInfoIndex = tile->entityInfoIndices[followerPosition];
	LocalDirection followerDirection = follower->localDirections[followerInfoIndex];

	if (followerDirection == tile->forceLocalDirection) {
		entity->followingEntityIndex = follower->index;
		follower->leadingEntityIndex = entities.back().index;
		
		int followerCount = 1;
		while (follower->hasFollower()) {
			follower = getFollower(follower);
			followerCount++;
		}
		return followerCount;
	}
	return 0;
}

bool EntityManager::tryAddLeaderCenter(Entity* entity) {
	Tile* tile = getTile(entity, 0);
	LocalPosition leaderPosition = (LocalPosition)tile->forceLocalDirection;

	if (tile->entityIndices[leaderPosition] == NO_ENTITY_INDEX) { return false; }

	Entity* leader = &entities[tile->entityIndices[leaderPosition]];
	bool leaderInfoIndex = tile->entityInfoIndices[leaderPosition];
	LocalDirection leaderDirection = leader->localDirections[leaderInfoIndex];
	
	if (leaderDirection == tile->forceLocalDirection) {
		leader->followingEntityIndex = entities.back().index;
		entity->leadingEntityIndex = leader->index;
		return true;
	}
	return false;
}

void EntityManager::manageEntityCollision(LeaderInfo* leaderInfo, int entityInfosIndex) {
	std::cout << "COLLISION" << std::endl;

	Entity* entity = getEntity(leaderInfo);
	// find the opposing entity:
	Tile* tile = p_tileManager->tiles[entity->tileIndices[0]];
	Tile* opposingTile = tile;
	// opposing entities are always 2 positions away, so first step once:
	LocalPosition entityPos = entity->localPositions[entityInfosIndex];
	LocalDirection entityDir = entity->localDirections[entityInfosIndex];
	LocalPosition middle = TileNavigator::nextLocalPosition(entityPos, entityDir);
	bool inOtherTile = false;
	if (middle == LOCAL_POSITION_INVALID) {
		inOtherTile = true;
		opposingTile = tile->neighbor(entityDir);
		LocalPosition neighborPos = (LocalPosition)tile->sideInfos.connectedSideIndices[entityDir];
		middle = TileNavigator::nextLocalPosition(neighborPos, LocalDirection((neighborPos + 2) % 4));
	}
	LocalPosition opposingPos = TileNavigator::nextLocalPosition(middle, entityDir);
	// if the middle is on the edge, the opposing entity will be in a neighboring tile:
	if (opposingPos == LOCAL_POSITION_INVALID) {
		inOtherTile = true;
		opposingTile = tile->neighbor(middle);
		LocalPosition neighborMiddle = (LocalPosition)tile->sideInfos.connectedSideIndices[middle];
		opposingPos = TileNavigator::nextLocalPosition(neighborMiddle, LocalDirection((neighborMiddle + 2) % 4));
	}
	if (opposingTile->hasEntity(opposingPos) == false) {
		// This shouldnt be possible, but just in case:
		std::cout << "OPPOSING ENTITY DOES NOT EXIST!" << std::endl;
		reverseEntityLineDir(entity, leaderInfo->leaderIndex);
		return;
	}
	// Now we know that there is an opposing entity, we can either add this entity as a follower or inverse the
	// directions of entities in both lines depending on the collision:
	Entity* opposingEntity = getEntity(opposingTile, opposingPos);
	// if there is a collision between two lines, than the lines must not be going in the same direction,
	// as the only time that is possible is on creating of an entity, and that outlier is solved for in the
	// createEntity function already!

	// TODO: change this so that on collision, funky Newton's Cradle stuff happens:
	reverseEntityLineDir(entity, entity->leaderListIndex);
	reverseEntityLineDir(opposingEntity, opposingEntity->leaderListIndex);

}

void EntityManager::reverseEntityLineDir(Entity* entity, int leaderIndex) {
	int lineMass = 1;

	entity->localDirections[0] = LocalDirection((entity->localDirections[0] + 2) % 4);
	entity->localDirections[1] = LocalDirection((entity->localDirections[1] + 2) % 4);
	while (entity->hasFollower()) {
		entity = getFollower(entity);
		entity->localDirections[0] = LocalDirection((entity->localDirections[0] + 2) % 4);
		entity->localDirections[1] = LocalDirection((entity->localDirections[1] + 2) % 4);
		lineMass++;
	}

	LocalDirection tailEntityDir = entity->localDirections[0];
	if (entity->tileIndices[1] == true) {
		tailEntityDir == entity->localDirections[1];
	}

	if (entity->isLeader()) {
		leaderInfos[leaderIndex].entityIndex = entity->index;
		leaderInfos[leaderIndex].direction = tailEntityDir;
	}
	else {
		entity->leaderListIndex = leaderInfos.size();
		leaderInfos.push_back(LeaderInfo(leaderInfos.size(), entity->index, tailEntityDir, lineMass));
	}
}

bool EntityManager::tryMoveLeaderInterior(LeaderInfo* leaderInfo) {
	
	Entity* entity = &entities[leaderInfos[leaderInfo->entityIndex].entityIndex];
	LocalPosition newPosition = TileNavigator::nextLocalPosition(entity->localPositions[0], entity->localDirections[0]);
	uint16_t currentMask = TileNavigator::getObstructionMask(entity->localPositions[0]);
	uint16_t newMask = TileNavigator::getObstructionMask(newPosition);
	Tile* tile = p_tileManager->tiles[entity->tileIndices[0]]; // If in only 1 tile, the first index points to it.
	bool obstructed = (tile->obstructionMask & ~currentMask) & newMask;
	if (obstructed) {
		manageEntityCollision(leaderInfo, 0);
		return false;
	}
	int oldPositionIndex = entity->localPositions[0];
	tile->entityIndices[oldPositionIndex] = -1;
	tile->entityIndices[newPosition] = entity->index;
	
	entity->localPositions[0] = newPosition;

	obstructionMaskChanges.push_back(ObstructionMaskChange(tile->index, currentMask, tile->index, newMask));

	// TODO: MOVE FOLLOWERS:

	// TODO: if the new position has no force action on it (not in central position/no force in tile) and
	// there is an obstruction in the next spot it would be in, inverse the entity's direction:

	return true;
}

bool EntityManager::tryMoveLeaderToEdge(LeaderInfo* leaderInfo) {
	
	Entity* entity = &entities[leaderInfos[leaderInfo->entityIndex].entityIndex];
	Tile* tile = getTile0(entity);
	Tile* neighborTile = tile->getNeighbor(entity->localDirections[0]);
	LocalPosition newPosition = (LocalPosition)entity->localDirections[0];
	LocalPosition neighborNewPosition = (LocalPosition)tile->sideInfos.connectedSideIndices[entity->localDirections[0]];

	uint16_t currentMask = TileNavigator::getObstructionMask(entity->localPositions[0]);
	uint16_t newMask = TileNavigator::getObstructionMask(newPosition);
	uint16_t neighborNewMask = TileNavigator::getObstructionMask(neighborNewPosition);

	bool obstruction = neighborTile->obstructionMask & neighborNewMask;
	if (obstruction) {
		manageEntityCollision(leaderInfo, 0);
		return false;
	}

	// Move things around:
	tile->entityIndices[entity->localPositions[0]] = -1;
	tile->entityIndices[newPosition] = entity->index;
	neighborTile->entityIndices[neighborNewPosition] = entity->index;

	entity->localPositions[0] = newPosition;
	entity->tileIndices[1] = neighborTile->index;
	entity->localPositions[1] = neighborNewPosition;
	entity->localDirections[1] = LocalDirection((neighborNewPosition + 2) % 4);
	entity->localOrientations[1] = TileNavigator::orientationToOrientationMap(tile->type, neighborTile->type,
		entity->localDirections[0], entity->localOrientations[0]);

	obstructionMaskChanges.push_back(ObstructionMaskChange(tile->index, currentMask, tile->index, newMask));
	obstructionMaskChanges.push_back(ObstructionMaskChange(neighborTile->index, neighborNewMask, neighborTile->index, neighborNewMask));

	// TODO: MOVE FOLLOWERS:

	// TODO: if the new position has no force action on it (not in central position/no force in tile) and
	// there is an obstruction in the next spot it would be in, inverse the entity's direction:

	return true;
}

bool EntityManager::tryMoveLeaderFromEdge(LeaderInfo* leaderInfo) {
	// figure out what tile to move fully to:
	Entity* entity = &entities[leaderInfos[leaderInfo->entityIndex].entityIndex];
	bool arrivingIndex, leavingIndex;
	if (entity->localDirections[0] == entity->localPositions[0]) { arrivingIndex = 1; leavingIndex = 0; }
	else { arrivingIndex = 0; leavingIndex = 1; }

	Tile* arrivingTile = getTile(entity, arrivingIndex);
	Tile* leavingTile = getTile(entity, leavingIndex);

	LocalPosition newPosition = LocalPosition(entity->localPositions[arrivingIndex] + 4);

	uint16_t arrivingCurrentMask = TileNavigator::getObstructionMask(entity->localPositions[arrivingIndex]);
	uint16_t leavingCurrentMask = TileNavigator::getObstructionMask(entity->localPositions[leavingIndex]);
	uint16_t newMask = TileNavigator::getObstructionMask(newPosition);

	bool obstruction = (arrivingTile->obstructionMask & ~arrivingCurrentMask) & newMask;
	if (obstruction) { 
		manageEntityCollision(leaderInfo, leavingIndex);
		return false; 
	}

	// Edge->middle position movement has the potential for conflicts.  Conflict check here:
	if (hasCornerConflict(entity, arrivingTile, arrivingIndex, 1)) { return false; }
	if (hasCornerConflict(entity, arrivingTile, arrivingIndex, 3)) { return false; }

	// Move things around:
	leavingTile->entityIndices[entity->localPositions[leavingIndex]] = -1;
	arrivingTile->entityIndices[entity->localPositions[arrivingIndex]] = -1;
	arrivingTile->entityIndices[newPosition] = entity->index;

	entity->localPositions[0] = newPosition;
	entity->tileIndices[0] = arrivingTile->index;
	entity->localDirections[0] = entity->localDirections[arrivingIndex];
	entity->lastLocalDirections[0] = entity->lastLocalDirections[arrivingIndex];
	entity->localOrientations[0] = entity->localOrientations[arrivingIndex];
	entity->tileIndices[1] = -1;

	obstructionMaskChanges.push_back(ObstructionMaskChange(leavingTile->index, leavingCurrentMask, arrivingTile->index, newMask));

	return true;
}

bool EntityManager::hasCornerConflict(Entity* entity,Tile* arrivingTile, int arrivingIndex, int edgeIndexOffset) {
	LocalPosition potentialConflictPosition = LocalPosition((entity->localPositions[arrivingIndex] + edgeIndexOffset) % 4);
	int potentialConflictEntityIndex = arrivingTile->entityIndices[potentialConflictPosition];
	bool hasEntity = potentialConflictEntityIndex != -1;
	if (!hasEntity) {
		return false;
	}

	Entity* potentialConflictEntity = &entities[potentialConflictEntityIndex];
	int i = 0;
	if (i != arrivingTile->index) { i = 1; }
	if (potentialConflictEntity->localDirections[i] == ((potentialConflictPosition + 2) % 4)) {
		std::cout << "corner conflict" << std::endl;
		return true;
	}

	return false;
}

bool EntityManager::tryMoveLeader(LeaderInfo* leaderInfo) {
	Entity* entity = &entities[leaderInfos[leaderInfo->leaderIndex].entityIndex];
	if (entity->hasDirection() == false) {
		return false;
	}
	if (entity->inEdgePosition()) { 
		return tryMoveLeaderFromEdge(leaderInfo);
	}
	if (entity->movingToEdge()) { 
		return tryMoveLeaderToEdge(leaderInfo);
	}
	return tryMoveLeaderInterior(leaderInfo);
}