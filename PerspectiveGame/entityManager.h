#pragma once
#include <iostream>
#include <bitset>

#include "tileManager.h"
#include "entity.h"
#include "tileNavigation.h"

const static uint16_t ENTITY_POS_OBSTRUCTION_MAP_MASKS[25] = {
	0b1000000000000000,
	0b1100000000000000,
	0b0110000000000000,
	0b0011000000000000,
	0b0001000000000000,
	0b1000100000000000,
	0b1100110000000000,
	0b0110011000000000,
	0b0011001100000000,
	0b0001000100000000,
	0b0000100010000000,
	0b0000110011000000,
	0b0000011001100000,
	0b0000001100110000,
	0b0000000100010000,
	0b0000000010001000,
	0b0000000011001100,
	0b0000000001100110,
	0b0000000000110011,
	0b0000000000010001,
	0b0000000000001000,
	0b0000000000001100,
	0b0000000000000110,
	0b0000000000000011,
	0b0000000000000001,
};

struct ObstructionMaskChange {
	int oldTileIndex;
	uint16_t oldMask;
	
	int newTileIndex;
	uint16_t newMask;

	ObstructionMaskChange(int oldTileIndex, uint16_t oldMask, int newTileIndex, uint16_t newMask):
		oldTileIndex(oldTileIndex), oldMask(oldMask), newTileIndex(newTileIndex), newMask(newMask)
	{}
};

struct LeaderInfo {
	int leaderIndex;
	int entityIndex;
	LocalDirection direction;
	int mass; // Total number of entities (and entity components in the case of larger entities) in the line.

	LeaderInfo(int leaderIndex, int entityIndex, LocalDirection direction, int mass) : 
		leaderIndex(leaderIndex), entityIndex(entityIndex), direction(direction), mass(mass) 
	{}

	void print() { std::cout << "leaderIndex: " << leaderIndex << "\nentityIndex: " << entityIndex << std::endl; }
};

struct EntityManager {
public:
	TileManager* p_tileManager;

	std::vector<Entity> entities;
	std::vector<LeaderInfo> leaderInfos; // Only these entities are updated on tick, others are followers or static.
	std::vector<int> staticEntityIndices;
	// This exists so that entities cannot 'cut' into each other's corners on movement:
	std::vector<ObstructionMaskChange> obstructionMaskChanges;
	std::vector<EntityGpuInfo> entityGpuInfos;

	GLuint entityGpuBufferID;

public:
	EntityManager(TileManager* tm) :p_tileManager(tm) {
		glGenBuffers(1, &entityGpuBufferID);
	}

	~EntityManager() {
		glDeleteBuffers(1, &entityGpuBufferID);
	}

	// Gives one of the 2 possible tiles an entity can be in (it is in 2 when on an edge).
	// If it is only in one tile, asking for the second tile it is in (index == 1) results in returning false
	// and output being nullptr.
	Tile* getTile(Entity* entity, bool index) { return p_tileManager->tiles[entity->tileIndices[index]]; }
	Tile* getTile0(Entity* entity) { return p_tileManager->tiles[entity->tileIndices[0]]; }
	Tile* getTile1(Entity* entity) { return p_tileManager->tiles[entity->tileIndices[1]]; }

	// Entities are always created at the center of tiles, otherwise tie-breaking collisions happen :(
	void createEntity(int tileIndex, EntityType entityType, LocalDirection orientation, bool override);
	void deleteEntity(Entity* entity);

	void removeEntityFromLeaderList(Entity* entity);
	void removeEntityFromStaticList(Entity* entity);
	void removeEntityFromEntityList(Entity* entity);

	void demoteLeader(int leaderIndex);

	Entity* getEntity(Tile* tile, LocalPosition position) { return &entities[tile->entityIndices[position]]; }
	Entity* getEntity(LeaderInfo* leaderInfo) { return &entities[leaderInfo->entityIndex]; }

	Entity* getFollower(Entity* entity) { return &entities[entity->followingEntityIndex]; }
	Entity* getLeader(Entity* entity) { return &entities[entity->leadingEntityIndex]; }
	LeaderInfo* reverseEntityLineDir(Entity* entity);

	// If a static entity is pushed/has force applied to it, it must be promoted to a leader or follower in order to move.
	void tryPromoteStaticEntities() {
		for (int i = 0; i < staticEntityIndices.size(); i++) {

			Entity* entity = &entities[staticEntityIndices[i]];
			Tile* tile = getTile0(entity);

			if (tile->hasForce() == false || tile->isObstructed(LocalPosition(tile->forceLocalDirection))) {
				return;
			}
			else {
				int staticIndex = entity->staticListIndex;
				std::swap(staticEntityIndices[staticIndex], staticEntityIndices.back());
				entities[staticEntityIndices[staticIndex]].staticListIndex = staticIndex;
				staticEntityIndices.pop_back();
				i--;

				entity->staticListIndex = -1;
				entity->localDirections[0] = tile->forceLocalDirection;
				entity->leaderListIndex = (int)leaderInfos.size();

				leaderInfos.push_back(LeaderInfo((int)leaderInfos.size(), entity->index, entity->localDirections[0], 1));
			}
		}
	}

	void manageEntityCollision(LeaderInfo* leaderInfo);

	bool tryChangeEntityDirection(LeaderInfo* leaderInfo);

	// An entity can only be made a leader while residing in the center or edge of a tile, and the logic is slightly different for each case.
	// returns the number of followers added to the entity.
	int tryAddFollowerCenter(Entity* entity);
	// An entity can only be made a follower while residing in the center or edge of a tile, and the logic is slightly different for each case.
	bool tryAddLeaderCenter(Entity* entity);

	// An entity can only be made a leader while residing in the center or edge of a tile, and the logic is slightly different for each case.
	bool tryMakeLeaderEdge(Entity* entity);
	// An entity can only be made a follower while residing in the center or edge of a tile, and the logic is slightly different for each case.
	bool tryMakeFollowerEdge(Entity* entity);

	bool tryMoveLeader(LeaderInfo* leaderInfo);
	bool tryMoveLeaderInterior(LeaderInfo* leaderInfo);
	bool tryMoveLeaderToEdge(LeaderInfo* leaderInfo);
	bool tryMoveLeaderFromEdge(LeaderInfo* leaderInfo);
	void moveFollowers(Entity* entity);

	bool cornerCollision(Entity* entity, Tile* arrivingTile, int arriving, int edgeOffset);

	void updateObstructionMasks() {
		for (ObstructionMaskChange omc : obstructionMaskChanges) {
			p_tileManager->tiles[omc.oldTileIndex]->obstructionMask &= ~omc.oldMask;
			p_tileManager->tiles[omc.newTileIndex]->obstructionMask |= omc.newMask;
		}
	}

	void update() {
		updateEntities();
		updateObstructionMasks();
		updateGpuInfos();
	}

	void updateEntities() {
		tryPromoteStaticEntities();
		for (int i = 0; i < leaderInfos.size(); i++) {
			LeaderInfo* info = &leaderInfos[i];
			// Forces can only change an entity's direction if it is in the central position of a tile,
			// else there is a possibility for tie-breaking conflicts!
			if (entities[info->entityIndex].inCenterPosition()) {
				tryChangeEntityDirection(info);
			}
			if (entities[info->entityIndex].hasDirection()) {
				tryMoveLeader(info);
			}
		}
	}

	void updateGpuInfos() {
		entityGpuInfos.clear();
		for (Entity& entity : entities) {
			entityGpuInfos.push_back(EntityGpuInfo(&entity));
		}
	}
};