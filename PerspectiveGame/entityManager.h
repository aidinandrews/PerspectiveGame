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
	bool getTile(Entity* entity, bool index, Tile** output) {
		if (entity->tileIndex[index] != -1) {
			output = &p_tileManager->tiles[entity->tileIndex[index]];
			return true;
		}
		output = nullptr;
		return false;
	}

	void createEntity(int tileIndex, Entity::Type entityType, LocalDirection orientation, bool override) {
		
		Tile* tile = p_tileManager->tiles[tileIndex];
		
		if (!override && tile->hasEntity(LOCAL_POSITION_CENTER)) {
			return;
		}

		entities.push_back(Entity(entityType, true, LOCAL_POSITION_CENTER, LOCAL_DIRECTION_0, orientation, 
			0.0f, -1, -1, glm::vec4(randColor(), 1)));
		entities.back().index = (int)entities.size() - 1;
		entities.back().tileIndex[0] = tileIndex;

		tile->entityIndices[LOCAL_POSITION_CENTER] = entities.back().index;
		tile->entityObstructionMap |= TileNavigator::localPositionToObstructionMask(LOCAL_POSITION_CENTER);

		// Connect up to leader/follower line if necessary:
	}
	void deleteEntity(Entity* entity) {
		Tile* tile;
		// remove from tile(s):
		for (int i = 0; i < 2; i++) {
			if (!entity->connectedToTile(i)) { continue; }
			tile = p_tileManager->tiles[entity->tileIndex[i]];
			tile->entityIndices[entity->localPosition] = -1;
			tile->entityObstructionMap &= ~TileNavigator::localPositionToObstructionMask(entity->localPosition);
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
			tile = p_tileManager->tiles[entity->tileIndex[i]];
			tile->entityIndices[entity->localPosition] = newIndex;
		}
		entities.pop_back();
	}

	bool moveEntityToEdge(Entity* entity, LocalDirection side);
	bool moveEntityToNeighborEdge(Entity* entity, LocalDirection side);
	bool moveEntityToNeighborInner(Entity* entity, LocalDirection side);

	bool tryMoveEntity(Entity* entity);
	bool tryMoveEntityInterior(Entity* entity);

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
			tryMoveEntity(&entity);
		}
	}

	void updateGpuInfos() {
		entityGpuInfos.clear();
		for (Entity& entity : entities) {
			entityGpuInfos.push_back(EntityGpuInfo(&entity));
		}
	}
};