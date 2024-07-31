#include "entityManager.h"

void EntityManager::createEntity(int tileIndex, Entity::Type entityType, LocalDirection orientation, bool override) {

	Tile* tile = p_tileManager->tiles[tileIndex];

	if (!override && tile->hasEntity(LOCAL_POSITION_CENTER)) {
		return;
	}

	entities.push_back(Entity(entityType, LOCAL_POSITION_CENTER, LOCAL_DIRECTION_STATIC, orientation,
		0.0f, -1, -1, glm::vec4(randColor(), 1)));
	entities.back().index = (int)entities.size() - 1;
	entities.back().tileIndices[0] = tileIndex;

	tile->entityIndices[LOCAL_POSITION_CENTER] = entities.back().index;
	tile->entityObstructionMask |= TileNavigator::localPositionToObstructionMask(LOCAL_POSITION_CENTER);

	// Connect up to leader/follower line if necessary:
}

void EntityManager::deleteEntity(Entity* entity) {
	Tile* tile;
	// remove from tile(s):
	for (int i = 0; i < 2; i++) {
		if (!entity->connectedToTile(i)) { continue; }
		tile = p_tileManager->tiles[entity->tileIndices[i]];
		tile->entityIndices[entity->localPositions[i]] = -1;
		tile->entityObstructionMask &= ~TileNavigator::localPositionToObstructionMask(entity->localPositions[i]);
	}

	if (entity->index == entities.size() - 1) {
		entities.pop_back();
		return;
	}

	// copy last entity to this one, then delete last entity:
	int newIndex = entity->index;
	(*entity) = entities.back();
	entity->index = newIndex;

	//if (lastEntity->hasLeader())   { entities[lastEntity->leadingEntityIndex].followingEntityIndex = newIndex; }
	//if (lastEntity->hasFollower()) { entities[lastEntity->followingEntityIndex].leadingEntityIndex = newIndex; }
	for (int i = 0; i < 2; i++) {
		if (!entity->connectedToTile(i)) { continue; }
		tile = p_tileManager->tiles[entity->tileIndices[i]];
		tile->entityIndices[entity->localPositions[i]] = newIndex;
	}
	entities.pop_back();
}

bool EntityManager::tryMoveEntityInterior(Entity* entity) {

	LocalPosition newPosition = TileNavigator::nextLocalPosition(entity->localPositions[0], entity->localDirections[0]);
	Tile* tile = p_tileManager->tiles[entity->tileIndices[0]]; // If in only 1 tile, the first index points to it.
	uint16_t currentMask = TileNavigator::localPositionToObstructionMask(entity->localPositions[0]);
	uint16_t newMask = TileNavigator::localPositionToObstructionMask(newPosition);
	bool obstructed = (tile->entityObstructionMask & ~currentMask) & newMask;
	if (obstructed) {
		return false;
	}

	tile->entityIndices[entity->localPositions[0]] = -1;
	tile->entityObstructionMask = (tile->entityObstructionMask & ~currentMask) | newMask;
	entity->localPositions[0] = newPosition;
	tile->entityIndices[newPosition] = entity->index;

	// TODO: MOVE FOLLOWERS:

	// TODO: if the new position has no force action on it (not in central position/no force in tile) and
	// there is an obstruction in the next spot it would be in, inverse the entity's direction:

	return true;
}

bool EntityManager::tryMoveEntityToEdge(Entity* entity) {
	Tile* tile = getTile0(entity);
	Tile* neighborile = tile->getNeighbor(entity->localDirections[0]);

	LocalPosition newPosition = (LocalPosition)entity->localDirections[0];
	LocalPosition neighborNewPosition = (LocalPosition)tile->sideInfos.connectedSideIndices[entity->localDirections[0]];

	uint16_t currentMask = TileNavigator::localPositionToObstructionMask(entity->localPositions[0]);
	uint16_t newMask = TileNavigator::localPositionToObstructionMask(newPosition);
	uint16_t neighborNewMask = TileNavigator::localPositionToObstructionMask(neighborNewPosition);

	bool obstruction = neighborile->entityObstructionMask & neighborNewMask;
	if (obstruction) {
		return false;
	}

	// Move things around:
	tile->entityIndices[entity->localPositions[0]] = -1;
	tile->entityIndices[newPosition] = entity->index;
	tile->entityObstructionMask = (tile->entityObstructionMask & ~currentMask) | newMask;

	neighborile->entityIndices[neighborNewPosition] = entity->index;
	neighborile->entityObstructionMask |= neighborNewMask;

	entity->localPositions[0] = newPosition;

	entity->tileIndices[1] = neighborile->index;
	entity->localPositions[1] = neighborNewPosition;
	entity->localDirections[1] = LocalDirection((neighborNewPosition + 2) % 4);
	entity->localOrientations[1] = TileNavigator::orientationToOrientationMap(tile->type, neighborile->type,
		entity->localDirections[0], entity->localOrientations[0]);

	// TODO: MOVE FOLLOWERS:

	// TODO: if the new position has no force action on it (not in central position/no force in tile) and
	// there is an obstruction in the next spot it would be in, inverse the entity's direction:

	return true;
}

bool EntityManager::tryMoveEntityFromEdge(Entity* entity) {
	// figure out what tile to move fully to:
	bool arriving, leaving;
	if (entity->localDirections[0] == entity->localPositions[0]) { arriving = 1; leaving = 0; }
	else { arriving = 0; leaving = 1; }

	Tile* arrivingTile = getTile(entity, arriving);
	Tile* leavingTile = getTile(entity, leaving);

	LocalPosition newPosition = LocalPosition(entity->localPositions[arriving] + 4);

	uint16_t arrivingCurrentMask = TileNavigator::localPositionToObstructionMask(entity->localPositions[arriving]);
	uint16_t leavingCurrentMask = TileNavigator::localPositionToObstructionMask(entity->localPositions[leaving]);
	uint16_t newMask = TileNavigator::localPositionToObstructionMask(newPosition);

	bool obstruction = (arrivingTile->entityObstructionMask & ~arrivingCurrentMask) & newMask;
	if (obstruction) {
		std::cout << "OBSTRUCTION" << std::endl;
		return false;
	}

	// Move things around:
	leavingTile->entityObstructionMask &= ~leavingCurrentMask;
	leavingTile->entityIndices[entity->localPositions[leaving]] = -1;

	arrivingTile->entityObstructionMask |= arrivingCurrentMask;
	arrivingTile->entityIndices[entity->localPositions[arriving]] = -1;
	arrivingTile->entityIndices[newPosition] = entity->index;

	entity->localPositions[0] = newPosition;
	entity->tileIndices[0] = arrivingTile->index;
	entity->localDirections[0] = entity->localDirections[arriving];
	entity->lastLocalDirections[0] = entity->lastLocalDirections[arriving];
	entity->localOrientations[0] = entity->localOrientations[arriving];

	entity->tileIndices[1] = -1;
}

bool EntityManager::tryMoveEntity(Entity* entity) {
	if (!entity->hasDirection()) { 
		return false; 
	}

	if (entity->inEdgePosition()) { 
		return tryMoveEntityFromEdge(entity); 
	}
	else if (entity->movingToEdge()) { 
		return tryMoveEntityToEdge(entity); 
	}
	else /*if (entity->inCenterPosition() || entity->movingToCenter())*/ { 
		return tryMoveEntityInterior(entity); 
	}
}

bool EntityManager::tryMoveEntityCenterToMiddle(Entity* entity) {
	//// entity is moving from a central position to a middle position
	//Tile* currentTile = p_tileManager->tiles[entity->tileIndex];
	//LocalPosition newPosition = TileNavigator::nextLocalPosition(entity->localPosition, entity->localDirection);
	//uint16_t oldObstructionMap = Entity::localPosToObstructionMask(entity->localPosition);
	//uint16_t newObstructionMap = Entity::localPosToObstructionMask(newPosition);
	//Entity* currentEntity = entity;

	//// check if current tile is clear for movement:
	//// if not, inverse entity/followers direction(s):
	//bool newPositionObstructed = (currentTile->entityObstructionMask &= ~oldObstructionMap) |= newObstructionMap;
	//if (newPositionObstructed) {
	//	currentEntity->localDirection = TileNavigator::oppositeDirection(currentEntity->localDirection);
	//	while (currentEntity->hasFollowers()) {
	//		currentEntity = currentEntity->follower;
	//		currentEntity->localDirection = TileNavigator::oppositeDirection(currentEntity->localDirection);
	//	}
	//	return false;
	//}

	//// move over entity/followers:
	//// add obstruction to current tile:
	//Tile* tailTile = currentTile;
	//LocalPosition tailPosition = currentEntity->localPosition;

	//currentTile->entityObstructionMask |= newObstructionMap;
	//currentTile->entityIndices[currentEntity->localPosition] = -1;
	//currentTile->entityIndices[newPosition] = currentEntity->index;
	//currentEntity->localPosition = TileNavigator::nextLocalPosition(currentEntity->localPosition, currentEntity->localDirection);
	//bool onEdge = true; // Eecause the leader is initially in the middle, the next follower must be on an edge.
	//while (currentEntity->hasFollowers()) {
	//	currentEntity = currentEntity->follower;
	//	currentTile = getTile(currentEntity);

	//	tailTile = currentTile;
	//	tailPosition = currentEntity->localPosition;

	//	currentTile->entityIndices[currentEntity->localPosition] = -1;
	//	newPosition = TileNavigator::nextLocalPosition(currentEntity->localPosition, currentEntity->localDirection);
	//	currentTile->entityIndices[newPosition] = currentEntity->index;
	//	if (onEdge) { // Edge entities have placed data in both connected tiles:
	//		int neighborPos = currentTile->sideInfos.connectedSideIndices[newPosition];
	//		currentTile->neighbor(currentEntity->localPosition)->entityIndices[neighborPos] = -1;
	//	}
	//	currentEntity->localPosition = TileNavigator::nextLocalPosition(currentEntity->localPosition, currentEntity->localDirection);

	//	onEdge = !onEdge;
	//}
	//// Remove obstruction from tail entity:
	//oldObstructionMap = Entity::localPosToObstructionMask(tailPosition);
	//newObstructionMap = Entity::localPosToObstructionMask(currentEntity->localPosition);
	//currentTile->entityObstructionMask = (currentTile->entityObstructionMask & ~oldObstructionMap) | newObstructionMap;

	//return true;
	return false;

}
bool EntityManager::tryMoveEntityMiddleToCenter(Entity* entity) {
	return tryMoveEntityCenterToMiddle(entity);
}
bool EntityManager::tryMoveEntityMiddleToEdge(Entity* entity) {
	//// entity is in a middle position and headed to an edge.

	//// check if neighboring tile is clear for movement:
	//Entity* currentEntity = entity;
	//Tile* currentTile = p_tileManager->tiles[entity->tileIndex];
	//LocalPosition newPosition = TileNavigator::nextLocalPosition(entity->localPosition, entity->localDirection);
	//Tile* neighbor = getTile(entity)->neighbor(newPosition);
	//LocalPosition neighborPosition = (LocalPosition)currentTile->sideInfos.connectedSideIndices[newPosition];
	//uint16_t newObstructionMap = Entity::localPosToObstructionMask(neighborPosition);
	//
	//// if not, inverse entity/followers direction(s):
	//bool newPositionObstructed = neighbor->entityObstructionMask & newObstructionMap;
	//if (newPositionObstructed) {
	//	currentEntity->localDirection = TileNavigator::oppositeDirection(currentEntity->localDirection);
	//	while (currentEntity->hasFollowers()) {
	//		currentEntity = currentEntity->follower;
	//		currentEntity->localDirection = TileNavigator::oppositeDirection(currentEntity->localDirection);
	//	}
	//	return false;
	//}

	//// move over entity/followers:
	//// add obstruction to current tile:
	//Tile* tailTile = currentTile;
	//LocalPosition tailPosition = currentEntity->localPosition;

	//currentTile->entityObstructionMask |= newObstructionMap;
	//currentTile->entityIndices[currentEntity->localPosition] = -1;
	//currentTile->entityIndices[newPosition] = currentEntity->index;
	//currentEntity->localPosition = TileNavigator::nextLocalPosition(currentEntity->localPosition, currentEntity->localDirection);
	//int neighborPos = currentTile->sideInfos.connectedSideIndices[currentEntity->localPosition];
	//currentTile->neighbor(currentEntity->localPosition)->entityIndices[neighborPos] = currentEntity->index;
	//bool onEdge = false; // Eecause the leader is initially in the middle, the next follower must be on an edge.
	//while (currentEntity->hasFollowers()) {
	//	currentEntity = currentEntity->follower;

	//	tailTile = currentTile;
	//	tailPosition = currentEntity->localPosition;

	//	currentTile = getTile(currentEntity);
	//	currentTile->entityIndices[currentEntity->localPosition] = -1;
	//	newPosition = TileNavigator::nextLocalPosition(currentEntity->localPosition, currentEntity->localDirection);
	//	currentTile->entityIndices[newPosition] = currentEntity->index;

	//	currentEntity->localPosition = TileNavigator::nextLocalPosition(currentEntity->localPosition, currentEntity->localDirection);

	//	if (onEdge) { // Edge entities have placed data in both connected tiles:
	//		int neighborPos = currentTile->sideInfos.connectedSideIndices[newPosition];
	//		currentTile->neighbor(currentEntity->localPosition)->entityIndices[neighborPos] = currentEntity->index;
	//	}

	//	onEdge = !onEdge;
	//}
	//// Remove obstruction from tail entity:
	//uint16_t tailObstructionMap = Entity::localPosToObstructionMask(tailPosition);
	//newObstructionMap = Entity::localPosToObstructionMask(currentEntity->localPosition);
	//currentTile->entityObstructionMask = (currentTile->entityObstructionMask & ~tailObstructionMap) | newObstructionMap;

	//return true;
	return false;

}
bool EntityManager::tryMoveEntityEdgeToMiddle(Entity* entity) {
	return false;

}
bool EntityManager::tryMoveEntityEdgeToNeighbor(Entity* entity) {
	return false;

}