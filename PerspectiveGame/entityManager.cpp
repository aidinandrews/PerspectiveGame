#include "entityManager.h"

bool EntityManager::moveEntityToEdge(Entity* entity, LocalDirection side) {
	//Entity* currentEntity             = entity;
	//Tile* currentTile                 = getTile(entity);
	//Tile* neighborTile                = currentTile->getNeighbor(side);
	//LocalPosition neighborPos = (LocalPosition)currentTile->sideInfos.connectedSideIndices[side];
	//uint16_t newNeighborObstruction   = Entity::localPosToObstructionMask(neighborPos);
	//bool nextPosOccupied              = neighborTile->entityObstructionMap &= newNeighborObstruction != 0;
	//
	//if (nextPosOccupied) { return false; } // Don't move the entity if there is stuff in the way.

	//// Move the entity and its followers (if it has any):
	//neighborTile->entityObstructionMap |= Entity::localPosToObstructionMask(LocalPosition(neighborPos));
	//currentEntity->localPosition = TileNavigator::TileNavigator::nextLocalPosition(currentEntity->localPosition, currentEntity->localDirection);
	//LocalPosition previousLocalPos = currentEntity->localPosition;
	//while (currentEntity->hasFollowers()) {
	//	currentEntity = currentEntity->follower;
	//	previousLocalPos = currentEntity->localPosition;
	//	currentEntity->localPosition = TileNavigator::nextLocalPosition(currentEntity->localPosition, currentEntity->localDirection);
	//}
	//// Remove 'tail' of the last follower from the obstruction map:
	//currentTile->entityObstructionMap  &= ~Entity::localPosToObstructionMask(previousLocalPos);
	//currentTile->entityObstructionMap  |=  Entity::localPosToObstructionMask(currentEntity->localPosition);	

	//return true;
}

bool EntityManager::moveEntityToNeighborEdge(Entity* entity, LocalDirection side) {
	//Tile* currentTile                 = getTile(entity);
	//Tile* neighborTile                = currentTile->getNeighbor(side);
	//LocalPosition neighborPos = (LocalPosition)currentTile->sideInfos.connectedSideIndices[side];
	//uint16_t newNeighborObstruction   = Entity::localPosToObstructionMask(neighborPos);
	//bool nextPosOccupied              = neighborTile->entityObstructionMap &= newNeighborObstruction != 0;
	//
	//if (nextPosOccupied) { return false; }

	//// Because the entity moved, we need to update the relevant obstruction maps:
	//currentTile->entityObstructionMap  &= ~Entity::localPosToObstructionMask(entity->localPosition);
	//currentTile->entityObstructionMap  |=  Entity::localPosToObstructionMask(LocalPosition(side));
	//neighborTile->entityObstructionMap |=  Entity::localPosToObstructionMask(LocalPosition(neighborPos));

	//entity->localPosition    = neighborPos;
	//entity->localDirection   = TileNavigator::dirToDirMap(currentTile->type, neighborTile->type, side);
	//entity->localOrientation = TileNavigator::orientationToOrientationMap(currentTile->type, neighborTile->type, side, entity->localOrientation);
	//entity->tileIndex        = neighborTile->index;

	//// Disconnect from current tile and connect to neighbor tile:
	//for (int i = 0; i < 3; i++) { 
	//	if (currentTile->entityIndices[i] == entity->index) { 
	//		currentTile->entityIndices[i] = -1; 
	//		break; 
	//	} 
	//}
	//for (int i = 0; i < 3; i++) { 
	//	if (neighborTile->entityIndices[i] == -1) { 
	//		neighborTile->entityIndices[i] = entity->index; 
	//		break; 
	//	} 
	//}
	//return true;
}

bool EntityManager::moveEntityToNeighborInner(Entity* entity, LocalDirection side) {
	//Tile* currentTile                      = getTile(entity);
	//Tile* neighborTile                     = currentTile->getNeighbor(side);
	//LocalDirection neighborDir             = TileNavigator::dirToDirMap(currentTile->type, neighborTile->type, side);
	//LocalPosition neighborSidePos          = (LocalPosition)currentTile->sideInfos.connectedSideIndices[side];
	//LocalPosition neighborInnerPos         = TileNavigator::nextLocalPosition(neighborSidePos, neighborDir);
	//uint16_t newEdgeObstruction            = Entity::localPosToObstructionMask(neighborSidePos);
	//uint16_t newInnerObstruction           = Entity::localPosToObstructionMask(neighborInnerPos);
	//bool nextPosOccupied                   = ((neighborTile->entityObstructionMap &= ~newEdgeObstruction) &= newInnerObstruction) != 0;

	//if (nextPosOccupied) { return false; }

	//// Because the entity moved, we need to update the relevant obstruction maps:
	//currentTile->entityObstructionMap  &= ~Entity::localPosToObstructionMask(entity->localPosition);
	//neighborTile->entityObstructionMap |=  Entity::localPosToObstructionMask(LocalPosition(neighborInnerPos));

	//entity->localPosition = neighborInnerPos;
	//entity->localDirection = TileNavigator::dirToDirMap(currentTile->type, neighborTile->type, side);
	//entity->localOrientation = TileNavigator::orientationToOrientationMap(currentTile->type, neighborTile->type, side, entity->localOrientation);
	//entity->tileIndex = neighborTile->index;

	//// Disconnect from current tile and connect to neighbor tile:
	//for (int i = 0; i < 3; i++) {
	//	if (currentTile->entityIndices[i] == entity->index) {
	//		currentTile->entityIndices[i] = -1;
	//		break;
	//	}
	//}
	//for (int i = 0; i < 3; i++) {
	//	if (neighborTile->entityIndices[i] == -1) {
	//		neighborTile->entityIndices[i] = entity->index;
	//		break;
	//	}
	//}
	//return true;
}

bool EntityManager::tryMoveEntity(Entity* entity) {
	//Tile* currentTile = getTile(entity);
	//LocalPosition newPos   = TileNavigator::nextLocalPosition(entity->localPosition, entity->localDirection);
	//uint16_t currentObstructionMap = currentTile->entityObstructionMap;
	//uint16_t currentObstruction    = Entity::localPosToObstructionMask(entity->localPosition);
	//uint16_t newObstruction        = Entity::localPosToObstructionMask(newPos);


	//// Check if the entity has to check/change neighboring tile data:
	//if (entity->localPosition == entity->localDirection) {
	//	// entity is on an edge and headed into a neighboring tile.

	//	// check if neighboring tile is clear for movement:
	//	// if not, inverse entity/followers direction(s):

	//	// move over entity/followers
	//	// add obstruction to neighbor tile
	//	// remove obstruction from tail entity
	//}
	//else if (entity->localPosition == entity->localDirection + 4) {
	//	tryMoveEntityMiddleToEdge(entity);
	//}
	//else if (entity->localPosition == ((entity->localDirection + 2) % 4) + 4) {
	//	// entity is on an edge and headed into its current tile

	//	// check if current tile is clear for movement:
	//	// if not, inverse entity/followers direction(s):

	//	// move over entity/followers
	//	// add obstruction to current tile
	//	// remove obstruction from tail entity
	//}
	//else if (entity->localPosition == LocalPosition::LOCAL_POSITION_CENTER) {
	//	tryMoveEntityCenterToMiddle(entity);
	//}
	//else {
	//	tryMoveEntityMiddleToCenter(entity);
	//}

	//{
	//	//switch (newPos) {
	//	//case LocalPosition::EDGE_0: return moveEntityToEdge(entity, LocalDirection::_0);
	//	//case LocalPosition::EDGE_1: return moveEntityToEdge(entity, LocalDirection::_1);
	//	//case LocalPosition::EDGE_2: return moveEntityToEdge(entity, LocalDirection::_2);
	//	//case LocalPosition::EDGE_3: return moveEntityToEdge(entity, LocalDirection::_3);

	//	//case LocalPosition::_0_NEIGHBOR: return moveEntityToNeighborEdge(entity, LocalDirection::_0);
	//	//case LocalPosition::_1_NEIGHBOR: return moveEntityToNeighborEdge(entity, LocalDirection::_1);
	//	//case LocalPosition::_2_NEIGHBOR: return moveEntityToNeighborEdge(entity, LocalDirection::_2);
	//	//case LocalPosition::_3_NEIGHBOR: return moveEntityToNeighborEdge(entity, LocalDirection::_3);

	//	//case LocalPosition::_0_NEIGHBOR_INNER: return moveEntityToNeighborInner(entity, LocalDirection::_0);
	//	//case LocalPosition::_1_NEIGHBOR_INNER: return moveEntityToNeighborInner(entity, LocalDirection::_1);
	//	//case LocalPosition::_2_NEIGHBOR_INNER: return moveEntityToNeighborInner(entity, LocalDirection::_2);
	//	//case LocalPosition::_3_NEIGHBOR_INNER: return moveEntityToNeighborInner(entity, LocalDirection::_3);

	//	//case LocalPosition::INVALID: return false;

	//	//default: // No overlap with neighbors, positions 5, 6, 7, and 8:
	//	//	bool nextPosOccupied = ((currentObstructionMap &= ~currentObstruction) &= newObstruction) != 0;
	//	//	if (nextPosOccupied) { return false; }

	//	//	currentTile->entityObstructionMap &= ~Entity::localPosToObstructionMask(entity->localPosition);
	//	//	currentTile->entityObstructionMap |=  Entity::localPosToObstructionMask(LocalPosition(newPos));

	//	//	entity->localPosition = newPos;

	//	//	return true;
	//	//}
	//}
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
	//bool newPositionObstructed = (currentTile->entityObstructionMap &= ~oldObstructionMap) |= newObstructionMap;
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

	//currentTile->entityObstructionMap |= newObstructionMap;
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
	//currentTile->entityObstructionMap = (currentTile->entityObstructionMap & ~oldObstructionMap) | newObstructionMap;

	//return true;
}
bool EntityManager::tryMoveEntityMiddleToCenter(Entity* entity) {
	tryMoveEntityCenterToMiddle(entity);
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
	//bool newPositionObstructed = neighbor->entityObstructionMap & newObstructionMap;
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

	//currentTile->entityObstructionMap |= newObstructionMap;
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
	//currentTile->entityObstructionMap = (currentTile->entityObstructionMap & ~tailObstructionMap) | newObstructionMap;

	//return true;
}
bool EntityManager::tryMoveEntityEdgeToMiddle(Entity* entity) {

}
bool EntityManager::tryMoveEntityEdgeToNeighbor(Entity* entity) {

}