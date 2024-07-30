#include "forceManager.h"

void ForceManager::removeForces() {
	for (int i = 0; i < forceEaters.size(); i++) {
		Tile* tile = p_tileManager->tiles[forceEaters[i].tileIndex];

		// Eat:
		tile->force.hasForce = 0;

		// Go to next tile:
		LocalDirection newHeading = TileNavigator::dirToDirMap(tile->type, tile->getNeighbor(forceEaters[i].heading)->type, forceEaters[i].heading);
		forceEaters[i].tileIndex = tile->sideInfos.connectedTiles[forceEaters[i].heading]->index;
		forceEaters[i].heading = newHeading;

		Tile* newTile = p_tileManager->tiles[forceEaters[i].tileIndex];

		// Kys if there is no more force to eat/we jumped to a new line of force:
		if ((forceEaters[i].heading != newTile->force.direction) || (newTile->force.hasForce == false)
			|| tile->basis.type == BasisType::FORCE_SINK || tile->basis.type == BasisType::FORCE_GENERATOR) {

			forceEaters.erase(forceEaters.begin() + i);
			i--;
			continue;
		}
	}
}

void ForceManager::propagateForces() {
	for (int i = 0; i < forcePropagators.size(); i++) {

		Tile* tile = p_tileManager->tiles[forcePropagators[i].tileIndex];

		// Kys if next tile already has a force in it:
		if (tile->force.hasForce || tile->basis.type == BasisType::FORCE_SINK) {
			forcePropagators.erase(forcePropagators.begin() + i);
			i--;
			continue;
		}

		LocalDirection heading = forcePropagators[i].heading;
		
		tile->force.direction = heading;
		tile->force.hasForce = true;

		Tile* nextTile = tile->sideInfos.connectedTiles[heading];

		// go to the next tile:
		//int newHeading = (forcePropagators[i].heading + 3 * adjustToLocalDirectionOffset(tile, forcePropagators[i].heading)) % 4;
		forcePropagators[i].tileIndex = nextTile->index;
		forcePropagators[i].heading = TileNavigator::dirToDirMap(tile->type, nextTile->type, heading);
	}
}