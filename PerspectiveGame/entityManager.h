#pragma once
#include <iostream>

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

struct EntityManager {
public:
	TileManager* p_tileManager;

	std::vector<Entity> entities;
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
	Tile* getTile (Entity* entity, bool index) { return p_tileManager->tiles[entity->tileIndices[index]]; }
	Tile* getTile0(Entity* entity) { return p_tileManager->tiles[entity->tileIndices[0]]; }
	Tile* getTile1(Entity* entity) { return p_tileManager->tiles[entity->tileIndices[1]]; }

	void createEntity(int tileIndex, Entity::Type entityType, LocalDirection orientation, bool override);
	void deleteEntity(Entity* entity);

	bool moveEntityToEdge(Entity* entity, LocalDirection side);
	bool moveEntityToNeighborEdge(Entity* entity, LocalDirection side);
	bool moveEntityToNeighborInner(Entity* entity, LocalDirection side);

	bool tryMoveEntity(Entity* entity);
	bool tryMoveEntityInterior(Entity* entity);
	bool tryMoveEntityToEdge(Entity* entity);
	bool tryMoveEntityFromEdge(Entity* entity);

	bool tryMoveEntityCenterToMiddle(Entity* entity);
	bool tryMoveEntityMiddleToCenter(Entity* entity);
	bool tryMoveEntityMiddleToEdge(Entity* entity);
	bool tryMoveEntityEdgeToMiddle(Entity* entity);
	bool tryMoveEntityEdgeToNeighbor(Entity* entity);

	void update() {
		updateEntities();
		updateGpuInfos();
	}

	void updateEntities() {
		for (Entity& entity : entities) {
			// Forces can only change an entity's direction if it is in the central position of a tile,
			// else there is a possibility for tie-breaking conflicts!
			if (entity.inCenterPosition()) {
				entity.localDirections[0] = getTile0(&entity)->forceLocalDirection;
			}
			if (entity.hasDirection()) {
				tryMoveEntity(&entity);
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