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

struct ObstructionMaskSelectiveAdd {
	int tileIndex;
	uint16_t mask; // Will |= target obstruction mask.

	ObstructionMaskSelectiveAdd(int tileIndex, uint16_t mask) :
		tileIndex(tileIndex), mask(mask)
	{}
};

struct ObstructionMaskSelectiveClear {
	int tileIndex;
	uint16_t mask; // Will &= target obstruction mask.

	ObstructionMaskSelectiveClear(int tileIndex, uint16_t mask) :
		tileIndex(tileIndex), mask(mask)
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

	// If obstruction masks are edited while entities are updated, the second entity of an entity collision will
	// not see that it is obstructed after the first one is updated.  Obstruction masks are therefore updated after
	// entities are updated, and these updates are kept track of here:
	std::vector<ObstructionMaskSelectiveAdd> obstructionMaskAdditions;
	std::vector<ObstructionMaskSelectiveClear> obstructionMaskClears;
	
	// This vector eeps track of the number of entities in each tile (Max 9).
	// It is used when updating the tile GPU info structs.
	std::vector<int> entitiesInTiles;
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
	Tile* getTile(Entity* entity) { return p_tileManager->tiles[entity->getTileIndex()]; }
	Tile* getLeavingTile(Entity* entity) { return p_tileManager->tiles[entity->getLeavingTileIndex()]; }

	// Entities are always created at the center of tiles, otherwise tie collisions happen.
	void createEntity(int tileIndex, EntityType type, LocalDirection direction, LocalOrientation orientation);
	void deleteEntity(Entity* entity);

	//void demoteLeader(int leaderIndex);

	//Entity* getEntity(LeaderInfo* leaderInfo) { return &entities[leaderInfo->entityIndex]; }

	//Entity* getFollower(Entity* entity) { return &entities[entity->followingEntityIndex]; }
	//Entity* getLeader(Entity* entity) { return &entities[entity->leadingEntityIndex]; }
	
	void manageEntityCollision(LeaderInfo* leaderInfo);

	bool tryChangeEntityDirection(LeaderInfo* leaderInfo);

	// An entity can only be made a leader while residing in the center or edge of a tile, and the logic is slightly different for each case.
	// returns the number of followers added to the entity.
	//int tryAddFollowerCenter(Entity* entity);
	// An entity can only be made a follower while residing in the center or edge of a tile, and the logic is slightly different for each case.
	//bool tryAddLeaderCenter(Entity* entity);

	// An entity can only be made a leader while residing in the center or edge of a tile, and the logic is slightly different for each case.
	//bool tryMakeLeaderEdge(Entity* entity);
	// An entity can only be made a follower while residing in the center or edge of a tile, and the logic is slightly different for each case.
	//bool tryMakeFollowerEdge(Entity* entity);

	/*void removeEntityFromLeaderList(Entity* entity);
	void removeEntityFromStaticList(Entity* entity);
	void removeEntityFromEntityList(Entity* entity);

	bool tryMoveLeader(LeaderInfo* leaderInfo);
	bool tryMoveLeaderInterior(LeaderInfo* leaderInfo);
	bool tryMoveLeaderToEdge(LeaderInfo* leaderInfo);
	bool tryMoveLeaderFromEdge(LeaderInfo* leaderInfo);
	void moveFollowers(Entity* entity);*/

	void moveEntity(Entity* entity);
	void moveEntityInterior(Entity* entity);
	void moveEntityFromEdge(Entity* entity);
	void moveEntityToEdge(Entity* entity);

	bool cornerCollision(Entity* entity, Tile* arrivingTile, int arriving, int edgeOffset);

	void update() {
		updateEntities();
		updateObstructionMasks();
		updateGpuInfos();
	}

	void updateEntities() {
		for (Entity& entity : entities) {
			moveEntity(&entity);
		}
	}

	void updateObstructionMasks() {
		for (ObstructionMaskSelectiveClear clear : obstructionMaskClears) {
			p_tileManager->tiles[clear.tileIndex]->obstructionMask &= clear.mask;
		}
		for (ObstructionMaskSelectiveAdd add : obstructionMaskAdditions) {
			p_tileManager->tiles[add.tileIndex]->obstructionMask |= add.mask;
		}
		obstructionMaskClears.clear();
		obstructionMaskAdditions.clear();
	}

	void updateGpuInfos() {
		int tileIndex;
		int numEntitiesInTile;

		entityGpuInfos.clear();
		entitiesInTiles.assign(p_tileManager->tiles.size(), 0); // Start with 0 entities/tile.
		p_tileManager->clearEntityIndices();

		for (Entity& entity : entities) {
			tileIndex = entity.getTileIndex();
			numEntitiesInTile = entitiesInTiles[tileIndex];

			p_tileManager->tileGpuInfos[tileIndex].entityIndices[numEntitiesInTile] = entity.index;
			p_tileManager->tiles[tileIndex]->entityIndices[numEntitiesInTile] = entity.index;
			entitiesInTiles[tileIndex]++;

			if (entity.inEdgePosition()) { // Entities on edges effect 2 tiles:
				tileIndex = entity.getLeavingTileIndex();
				numEntitiesInTile = entitiesInTiles[tileIndex];
				p_tileManager->tileGpuInfos[tileIndex].entityIndices[numEntitiesInTile] = entity.index;
				p_tileManager->tiles[tileIndex]->entityIndices[numEntitiesInTile] = entity.index;
				entitiesInTiles[tileIndex]++;
			}

			if (numEntitiesInTile > 9 || numEntitiesInTile < 0) {
				std::cout << "OH NO 2" << std::endl;
			}

			entityGpuInfos.push_back(EntityGpuInfo(&entity));
		}
	}
};