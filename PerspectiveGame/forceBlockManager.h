#pragma once
#include <iostream>
#include <vector>

#include "tileManager.h"
#include "forceManager.h"

struct ForceBlockManager {
private:
	TileManager* p_tileManager;
	ForceManager* p_forceManager;


public:
	ForceBlockManager(TileManager* tm, ForceManager* fm) : p_tileManager(tm), p_forceManager(fm) {

	}

private:
	bool createForceBlock(int tileIndex, LocalDirection orientation, int magnitude) {
		Tile* tile = p_tileManager->tiles[tileIndex];

		if (tile->isObstructed(LocalPosition::LOCAL_POSITION_CENTER)) {
			return false;
		}

		// MOVE THIS STUFF TO createEntity()
		//tile->entity = new Entity;
		//tile->entity->type = Entity::Type::BUILDING_FORCE_BLOCK;
		////tile->entity.offset = 0; // Centered initially.
		//tile->entity->orientation = orientation;
		//tile->entity->direction = (tile->force.magnitude > 0) ? tile->force.direction : 0;
		//tile->entity->mass = 3;

		Tile* neighborTile = p_tileManager->tiles[tile->sideInfos.connectedTiles[orientation]->index];
		LocalDirection neighborOrientation = TileNavigator::dirToDirMap(tile->type, neighborTile->type, orientation);
		p_forceManager->createForcePropogator(neighborTile->index, neighborOrientation);

		return true;
	}
};