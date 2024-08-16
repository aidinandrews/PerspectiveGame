#include "entityManager.h"

void EntityManager::createEntity(int tileIndex, EntityType type, LocalDirection direction, LocalOrientation orientation)
{
	Tile* tile = p_tileManager->tiles[tileIndex];

	if (tile->obstructionMask != 0) { return; }

	tile->obstructionMask |= TileNavigator::getObstructionMask(LOCAL_POSITION_CENTER);

	entities.push_back(Entity(int(entities.size()), tile->index,
							  type, direction, orientation, glm::vec4(randColor(), 1)));
}

void EntityManager::deleteEntity(Entity* entity)
{
	obstructionMaskClears.push_back(ObstructionMaskSelectiveClear(
		getTile(entity)->index, ~TileNavigator::getObstructionMask(entity->getPosition())));
	if (entity->inEdgePosition()) {
		obstructionMaskClears.push_back(ObstructionMaskSelectiveClear(
			getLeavingTile(entity)->index, ~TileNavigator::getObstructionMask(entity->getLeavingPosition())));
	}
	std::swap(entities[entity->index], entities.back());
	entities.pop_back();
}

bool EntityManager::cornerCollision(Entity* entity, Tile* arrivingTile, int arrivingIndex, int edgeIndexOffset)
{
	/*LocalPosition potentialConflictPosition = LocalPosition((entity->localPositions[arrivingIndex] + edgeIndexOffset) % 4);
	int potentialConflictEntityIndex = arrivingTile->entityIndices[potentialConflictPosition];
	bool hasEntity = potentialConflictEntityIndex != -1;
	if(!hasEntity) {
		return false;
	}

	Entity* potentialConflictEntity = &entities[potentialConflictEntityIndex];
	int i = 0;
	if(i != arrivingTile->index) { i = 1; }
	if(potentialConflictEntity->localDirections[i] == ((potentialConflictPosition + 2) % 4)) {
		std::cout << "corner conflict" << std::endl;
		return true;
	}*/

	return false;
}

void EntityManager::moveEntity(Entity* entity)
{
	if (entity->inEdgePosition()) { moveEntityFromEdge(entity); }
	else if (entity->movingToEdge()) { moveEntityToEdge(entity); }
	else { moveEntityInterior(entity); }
}

void EntityManager::moveEntityInterior(Entity* entity)
{
	Tile* tile = p_tileManager->tiles[entity->getTileIndex()];
	LocalPosition currentPosition = entity->getPosition();
	uint16_t currentPositionMask = TileNavigator::getObstructionMask(entity->getPosition());
	LocalPosition targetPosition = TileNavigator::nextLocalPosition(entity->getPosition(), entity->getDirection());
	uint16_t targetPositionMask = TileNavigator::getObstructionMask(targetPosition);
	
	if ((tile->obstructionMask & ~currentPositionMask) & targetPosition) { // Movement is obstructed.
		// Due to placement restrictions, the entity will NEVER be obstructed moving in this direction:
		entity->setDirection(TileNavigator::oppositeDirection(entity->getDirection()));
		targetPosition = TileNavigator::nextLocalPosition(entity->getPosition(), entity->getDirection());
		targetPositionMask = TileNavigator::getObstructionMask(targetPosition);
	}

	entity->setPosition(targetPosition);
	obstructionMaskClears.push_back(ObstructionMaskSelectiveClear(tile->index, ~currentPositionMask));
	obstructionMaskAdditions.push_back(ObstructionMaskSelectiveAdd(tile->index, targetPositionMask));
}

void EntityManager::moveEntityFromEdge(Entity* entity)
{
	Tile* arrivingTile = getTile(entity);
	uint16_t arrivingCurrentPositionMask = TileNavigator::getObstructionMask(entity->getPosition());

	Tile* leavingTile = getLeavingTile(entity);
	uint16_t leavingCurrentPositionMask = TileNavigator::getObstructionMask(entity->getLeavingPosition());

	LocalPosition targetPosition = LocalPosition(entity->getPosition() + 4);
	uint16_t targetPositionMask = TileNavigator::getObstructionMask(targetPosition);

	if ((arrivingTile->obstructionMask & ~arrivingCurrentPositionMask) & targetPositionMask) { // Movement is obstructed.
		entity->swapArrivingAndLeavingInfos();
		entity->setDirection(TileNavigator::oppositeDirection(entity->getDirection()));
		std::swap(arrivingCurrentPositionMask, leavingCurrentPositionMask);
		targetPosition = LocalPosition(entity->getPosition() + 4);
		targetPositionMask = TileNavigator::getObstructionMask(targetPosition);
	}

	// Edge->middle position movement has the potential for conflicts.  Conflict check here:
	// TODO: fix this it no work I think
	//if (cornerCollision(entity, arrivingTile, 0, 1)) { return false; }
	//if (cornerCollision(entity, arrivingTile, 0, 3)) { return false; }

	entity->setPosition(targetPosition);
	entity->setLeavingTileIndex(-1);
	entity->setLeavingPosition(LOCAL_POSITION_INVALID);
	entity->setLeavingDirection( LOCAL_DIRECTION_INVALID);

	obstructionMaskClears.push_back(ObstructionMaskSelectiveClear(leavingTile->index, ~leavingCurrentPositionMask));
	obstructionMaskAdditions.push_back(ObstructionMaskSelectiveAdd(arrivingTile->index, targetPositionMask));
}

void EntityManager::moveEntityToEdge(Entity* entity)
{
	Tile* leavingTile = getTile(entity);
	uint16_t leavingCurrentPositionMask = TileNavigator::getObstructionMask(entity->getPosition());
	LocalPosition leavingTargetPosition = (LocalPosition)entity->getDirection(); // WILL NOT WORK WITH DIAGONAL MOVEMENT
	uint16_t leavingTargetPositionMask = TileNavigator::getObstructionMask(leavingTargetPosition);

	Tile* arrivingTile = leavingTile->getNeighbor(entity->getDirection());
	LocalPosition arrivingTargetPosition = (LocalPosition)leavingTile->sideInfos.connectedSideIndices[entity->getDirection()];
	uint16_t arrivingTargetPositionMask = TileNavigator::getObstructionMask(arrivingTargetPosition);

	if (arrivingTile->obstructionMask & arrivingTargetPositionMask) { // Movement is obstructed.
		// The movement changes into an internal one:
		entity->setDirection(TileNavigator::oppositeDirection(entity->getDirection()));
		leavingTargetPosition = TileNavigator::nextLocalPosition(entity->getPosition(), entity->getDirection());
		uint16_t targetPositionMask = TileNavigator::getObstructionMask(leavingTargetPosition);
		entity->setPosition(leavingTargetPosition);

		obstructionMaskClears.push_back(ObstructionMaskSelectiveClear(leavingTile->index, ~leavingCurrentPositionMask));
		obstructionMaskAdditions.push_back(ObstructionMaskSelectiveAdd(leavingTile->index, targetPositionMask));
	}
	else { // Movement is unobstructed:
		entity->setLeavingTileIndex(entity->getTileIndex());
		entity->setLeavingPosition(leavingTargetPosition);
		entity->setLeavingDirection(entity->getDirection());
		entity->setLeavingOrientation(entity->getOrientation());

		entity->setTileIndex(arrivingTile->index);
		entity->setPosition(arrivingTargetPosition);
		entity->setDirection(LocalDirection((arrivingTargetPosition + 2) % 4)); // WONT WORK WITH DIAGONAL MOVEMENT
		entity->setOrientation(TileNavigator::orientationToOrientationMap(leavingTile->type, arrivingTile->type, entity->getLeavingDirection(), entity->getOrientation()));

		obstructionMaskClears.push_back(ObstructionMaskSelectiveClear(leavingTile->index, ~leavingCurrentPositionMask));
		obstructionMaskAdditions.push_back(ObstructionMaskSelectiveAdd(leavingTile->index, leavingTargetPositionMask));
		obstructionMaskAdditions.push_back(ObstructionMaskSelectiveAdd(arrivingTile->index, arrivingTargetPositionMask));
	}
}