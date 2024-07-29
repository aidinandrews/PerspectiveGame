#pragma once

#pragma once
#include"tile.h"
#include"tileManager.h"

#define NOT_A_LEADER -1

// Each global direction has a leader list:
std::vector<Tile*> leaderList[6];

// Finds the leader list associated with a tile's entity.
// The entity's direction determines its leader list.
std::vector<Tile*>* entityLeaderList(Tile* tile) {
	return &leaderList[Tile::localToGlobalDir(tile->type, tile->entity->direction)];
}

// Given a tile, this function will remove the entity in that tile from its associated leader list:
void demoteEntity(Tile* tile);

// Given a tile, its associated entity will be added to its associated leader list.
// If necessary, the highest prio neighboring entity will be added as a follower.
void makeEntityLeader(Tile* tile, Entity::Type entityType, int orientation, bool override);

void createEntity(Tile* tile);