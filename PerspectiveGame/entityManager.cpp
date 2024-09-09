#include "entityManager.h"

EntityManager::EntityManager(TileManager* tm) :p_tileManager(tm)
{
	glGenBuffers(1, &entityGpuBufferID);
	glGenBuffers(1, &entityTileInfoGpuBufferID);
}

EntityManager::~EntityManager()
{
	glDeleteBuffers(1, &entityGpuBufferID);
	glDeleteBuffers(1, &entityTileInfoGpuBufferID);
}

void EntityManager::createEntity(int tileIndex, EntityType type, LocalDirection direction, LocalOrientation orientation)
{
	Tile* tile = p_tileManager->tiles[tileIndex];
	tile->entityIndices[LOCAL_POSITION_CENTER] = (int)entities.size();
	tile->entityInfoIndices[LOCAL_POSITION_CENTER] = 0;

	entities.push_back(Entity(int(entities.size()), tile->index, type, direction, orientation, glm::vec4(randColor(), 1)));
}

void EntityManager::deleteEntity(Entity* entity)
{
	for (int i = 0; i < 4; i++) {
		if (entity->getTileIndex(i) == -1) {
			continue;
		}
		p_tileManager->tiles[entity->getTileIndex(i)]->entityIndices[entity->getPosition(i)] = -1;
		p_tileManager->tiles[entity->getTileIndex(i)]->entityInfoIndices[entity->getPosition(i)] = -1;
	}
	int index = entity->index;
	std::swap(entities[entity->index], entities.back());
	entities.pop_back();

	if (entities.size() == 0 || index == entities.size()) {
		return;
	}

	Entity* swappedEntity = &entities[index];
	swappedEntity->index = index;
	for (int i = 0; i < 4; i++) {
		if (swappedEntity->getTileIndex(i) == -1) { break; }
		p_tileManager->tiles[swappedEntity->getTileIndex(i)]->entityIndices[swappedEntity->getPosition(i)] = index;
		p_tileManager->tiles[swappedEntity->getTileIndex(i)]->entityInfoIndices[swappedEntity->getPosition(i)] = i;
	}
}

bool EntityManager::cornerCollision(Entity* entity, Tile* arrivingTile, int arrivingIndex, int edgeIndexOffset)
{
	return false;
}

void EntityManager::moveEntity(Entity* entity)
{
	if (entity->getDirection(0) == LOCAL_DIRECTION_STATIC) {
		return;
	}

	Tile* currentTile = getTile(entity, 0);
	const LocalDirection* entityDirectionComponents = tnav::getAlignmentComponents(entity->getDirection(0));

	if (tnav::isOrthogonal(entity->getDirection(0))) {
		// It can be trusted that the 0 index of entity info is defining the arriving tile info,
		// so it can be assumed if entity position[0] is an edge, it is moving to the center
		// and if entity position[1] is in the center, it is moving to an edge!
		if (entity->getPosition(0) == LOCAL_POSITION_CENTER) {
			currentTile->entityIndices[entity->getPosition(0)] = -1;
			currentTile->entityInfoIndices[entity->getPosition(0)] = -1;

			currentTile->entityIndices[entity->getDirection(0)] = entity->index;
			currentTile->entityInfoIndices[entity->getDirection(0)] = 1; // Current tile -> leaving tile.

			entity->setTileIndex(1, entity->getTileIndex(0));
			entity->setPosition(1, entity->getDirection(0));
			entity->setDirection(1, entity->getDirection(0));
			entity->setOrientation(1, entity->getOrientation(0));

			Tile* neighbor = currentTile->getNeighbor(entity->getDirection(0));
			entity->setOrientation(0, tnav::getMappedAlignment(entity->getDirection(0), entity->getOrientation(0)));
			entity->setDirection(0, tnav::getMappedAlignment(entity->getDirection(0), entity->getDirection(0)));
			entity->setPosition(0, (LocalPosition)currentTile->getNeighborConnectedSideIndex(entity->getDirection(0)));
			entity->setTileIndex(0, neighbor->index);

			neighbor->entityIndices[entity->getPosition(0)] = entity->index;
			neighbor->entityInfoIndices[entity->getPosition(0)] = 0;
		}
		// Because orthogonal movement on an edge is impossible, the entity must be on an edge moving to a center:
		else  {
			Tile* arrivingTile = getTile(entity, 0);

			arrivingTile->entityIndices[entity->getPosition(0)] = -1;
			arrivingTile->entityInfoIndices[entity->getPosition(0)] = -1;

			entity->makeTileInfoLeavings(1); // will be removed after entity gpu info update!
			entity->setPosition(0, LOCAL_POSITION_CENTER);

			arrivingTile->entityIndices[LOCAL_POSITION_CENTER] = entity->index;
			arrivingTile->entityInfoIndices[LOCAL_POSITION_CENTER] = 0;
		}
	}
	else { // Entity is moving diagonally.
		if (entity->getPosition(0) == LOCAL_POSITION_CENTER) {
			// entity is going to a corner!
			Tile* adjacentTile0 = currentTile->getNeighbor(entityDirectionComponents[0]);
			LocalDirection adjacentTile0Dir = currentTile->getMappedNeighborAlignment(entityDirectionComponents[0], entity->getDirection(0));
			LocalOrientation adjacentTile0Ori = currentTile->getMappedNeighborAlignment(entityDirectionComponents[0], entity->getOrientation(0));
			LocalPosition adjacentTile0Pos = currentTile->getMappedNeighborAlignment(entityDirectionComponents[0], tnav::combineAlignments(tnav::oppositeAlignment(entityDirectionComponents[0]), entityDirectionComponents[1]));
			
			Tile* adjacentTile1 = currentTile->getNeighbor(entityDirectionComponents[1]);
			LocalDirection adjacentTile1Dir = currentTile->getMappedNeighborAlignment(entityDirectionComponents[1], entity->getDirection(0));
			LocalOrientation adjacentTile1Ori = currentTile->getMappedNeighborAlignment(entityDirectionComponents[1], entity->getOrientation(0));
			LocalPosition adjacentTile1Pos = currentTile->getMappedNeighborAlignment(entityDirectionComponents[1], tnav::combineAlignments(tnav::oppositeAlignment(entityDirectionComponents[1]), entityDirectionComponents[0]));
			
			// Wow this is not going to be understandabe next time I look at it...
			LocalDirection toArrivingFromAdjacentDir = currentTile->getMappedNeighborAlignment(entityDirectionComponents[0], entityDirectionComponents[1]);
			Tile* arrivingTile = adjacentTile0->getNeighbor(toArrivingFromAdjacentDir);
			LocalDirection arrivingTileDir = adjacentTile0->getMappedNeighborAlignment(toArrivingFromAdjacentDir, adjacentTile0Dir);
			LocalDirection arrivingTileOri = adjacentTile0->getMappedNeighborAlignment(toArrivingFromAdjacentDir, adjacentTile0Ori);
			LocalPosition arrivingTilePos = LocalPosition((((entity->getDirection(0) - 4) + 2) % 4) + 4);
			arrivingTilePos = currentTile->getMappedNeighborAlignment(entityDirectionComponents[0], arrivingTilePos);
			arrivingTilePos = adjacentTile0->getMappedNeighborAlignment(toArrivingFromAdjacentDir, arrivingTilePos);
			
			// delete entity data from current tile old position:
			currentTile->entityIndices[entity->getPosition(0)] = -1;
			currentTile->entityInfoIndices[entity->getPosition(0)] = -1;

			// adjust entity tile info:
			entity->setTileInfo(3, currentTile->index, (LocalPosition)entity->getDirection(0), entity->getDirection(0), entity->getOrientation(0));
			entity->setTileInfo(0, arrivingTile->index, arrivingTilePos, arrivingTileDir, arrivingTileOri);
			entity->setTileInfo(1, adjacentTile0->index, adjacentTile0Pos, adjacentTile0Dir, adjacentTile0Ori);
			entity->setTileInfo(2, adjacentTile1->index, adjacentTile1Pos, adjacentTile1Dir, adjacentTile1Ori);

			// populate moved-to tiles with entity info:
			arrivingTile->entityIndices[entity->getPosition(0)] = entity->index;
			arrivingTile->entityInfoIndices[entity->getPosition(0)] = 0;
			adjacentTile0->entityIndices[entity->getPosition(1)] = entity->index;
			adjacentTile0->entityInfoIndices[entity->getPosition(1)] = 1;
			adjacentTile1->entityIndices[entity->getPosition(2)] = entity->index;
			adjacentTile1->entityInfoIndices[entity->getPosition(2)] = 2;
			currentTile->entityIndices[entity->getPosition(3)] = entity->index;
			currentTile->entityInfoIndices[entity->getPosition(3)] = 3;
		}
		else { // Entity is on a corner and moving to the center.
			getTile(entity, 0)->entityIndices[entity->getPosition(0)] = -1;
			getTile(entity, 0)->entityInfoIndices[entity->getPosition(0)] = -1;

			entity->makeTileInfoLeavings(1);
			entity->makeTileInfoLeavings(2);
			entity->makeTileInfoLeavings(3);

			entity->setPosition(0, LOCAL_POSITION_CENTER); // All other info is the same, as it is the arriving tile moving internally.
			getTile(entity, 0)->entityIndices[LOCAL_POSITION_CENTER] = entity->index;
			getTile(entity, 0)->entityInfoIndices[LOCAL_POSITION_CENTER] = 0;
		}
	}
}

void EntityManager::removeTileInfoLeavings()
{
	for (Entity& entity : entities) {
		int i = 1;
		while (i < 4 && entity.isTileInfoLeavings(i)) {
			getTile(&entity, i)->entityIndices[entity.getPosition(i)] = -1;
			getTile(&entity, i)->entityInfoIndices[entity.getPosition(i)] = -1;
			entity.clearTileInfo(i);
			i++;
		}
	}
}

void EntityManager::updateGpuInfos()
{
	GPU_entityInfos.clear();
	GPU_entityTileInfos.clear();

	for (Entity& entity : entities) {
		GPU_entityInfos.push_back(GPU_EntityInfo(&entity));
		for (int i = 0; i < 4; i++) {
			if (entity.getTileIndex(i) == -1) {
				GPU_entityInfos.back().tileInfoIndex[i] = -1;
			}
			else {
				GPU_entityInfos.back().tileInfoIndex[i] = (int)GPU_entityTileInfos.size();
				GPU_entityTileInfos.push_back(GPU_EntityTileInfo(
					entity.getPosition(i), entity.getDirection(i),
					entity.getOrientation(i), entity.isTileInfoLeavings(i)));
			}
		}
	}
	// Leaving infos will confuse next tick, so must be cleared here:
	// 'leavings' let the tile gpu infos know about entities that are no longer inside them but
	// not all the way gone (in the process of leaving).
	removeTileInfoLeavings();
}