#pragma once
#include "entityLeaderFunctions.h"

bool neighborEntityHeadedTowardTile(Tile* tile, int neighborSideIndex) {
	Tile* neighbor = tile->neighbor(neighborSideIndex);
	if (neighbor->hasNoEntity()) { return false; }

	int dir1 = Tile::dirToDirMap(tile->type, neighbor->type, neighborSideIndex);
	int dir2 = Tile::oppositeLocalDirection(neighbor->entity->direction);
	if (dir1 == dir2) { return true; }
	return false;
}

void demoteEntity(Tile* tile) {
	if (tile->entity->leaderListIndex = NOT_A_LEADER) { return; }

	Tile* lastLeaderTile = entityLeaderList(tile)->back();

	lastLeaderTile->entity->leaderListIndex = tile->entity->leaderListIndex;
	tile->entity->leaderListIndex = NOT_A_LEADER;

	(*entityLeaderList(tile))[tile->entity->leaderListIndex] = lastLeaderTile;
	entityLeaderList(tile)->pop_back();
}

void makeEntityLeader(Tile* tile) {
	entityLeaderList(tile)->push_back(tile);

	// Create a list of directions in order of highest global directional priority:
	int orderedDirList[3];
	for (int i = 1; i < 4; i++) { orderedDirList[0] = (tile->entity->direction + i) % 4; }
	#define dirPrio(localDir) Tile::directionPriority(Tile::localToGlobalDir(tile->type, orderedDirList[localDir]))
	if (dirPrio(0) > dirPrio(1)) { std::swap(orderedDirList[0], orderedDirList[1]); }
	if (dirPrio(1) > dirPrio(2)) { std::swap(orderedDirList[1], orderedDirList[2]); }
	if (dirPrio(0) > dirPrio(1)) { std::swap(orderedDirList[0], orderedDirList[1]); }
	#undef dirPrio

	// Update neighbors:
	Tile* follower = nullptr;
	for (int i = 0; i < 3; i++) {
		// Because we want to be able to adjust what directions prio others, we need to reference DIR_PRIO_LIST here:
		int dir = orderedDirList[i];
		Tile* neighbor = tile->neighbor(dir);
		Entity* leader = tile->entity;

		if (neighborEntityHeadedTowardTile(tile, dir)) {
			follower = tile->neighbor(dir);
			break;
		}

		if (follower == nullptr) { continue; }


		while (follower != nullptr) {
			leader->followingEntityTileIndex = follower->index;
		}
		demoteEntity(neighbor);
		leader->followingEntityTileIndex = neighbor->index;
		// No need to check further, as this is the highest prio neighbor.
	}
}

void tryCreateBuilding(Tile* tile) {
	switch (tile->entity->type) {
	case Entity::Type::BUILDING_FORCE_BLOCK:
		TileManager::createForceBlock(tile->index, (Tile::Edge)tile->entity->orientation, 1);
		break;
	case Entity::Type::BUILDING_COMPRESSOR:

		break;
	case Entity::Type::BUILDING_FORCE_MIRROR:

		break;
	}
}

void createEntity(Tile* tile, Entity::Type entityType, int entityOrientation, bool override) {
	if (!override && tile->hasEntity()) { return; }

	Entity* newEntity = new Entity;
	newEntity->orientation = entityOrientation;
	newEntity->type = entityType;
	tile->entity = newEntity;

	// Some entities are also buildings and have more complex initializations:
	tryCreateBuilding(tile);

	if (tile->hasNoForce()) {
		// Update neighbors:
		for (int i = 0; i < 4; i++) {
			if (tile->hasNoEntity()) { continue; }
			if (tile->neighbor(i)->entity->direction == tile->sideInfos.connectedSideIndices[i]) { demoteEntity(tile->neighbor(i)); }
		}
		return;
	}
	
	newEntity->direction = tile->force.direction;
	Tile* neighbor = tile->neighbor(tile->force.direction);

	if (neighbor->hasNoEntity()) {
		makeEntityLeader(tile);
		return;
	}

	if (neighbor->entity->hasNoFollowers()) {
		neighbor->entity->followingEntityTileIndex = tile->index;
		return;
	}

	int newEntityDirPrio = Tile::directionPriority(Tile::localToGlobalDir(tile->type, newEntity->direction));
	Tile* neighborFollower = TileManager::tiles[neighbor->entity->followingEntityTileIndex];
	int oldEntityDirPrio = Tile::directionPriority(Tile::localToGlobalDir(neighborFollower->type, neighborFollower->entity->direction));

	if (newEntityDirPrio < oldEntityDirPrio) {
		neighbor->entity->followingEntityTileIndex = tile->index;
		return;
	}
}