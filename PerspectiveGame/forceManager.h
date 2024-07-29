#pragma once
#include <iostream>
#include <vector>

#include "tileManager.h"

struct ForceManager {
private:
	// When a force block is placed down, it creates a ForcePropagator, which will propagate the force it creates into the tiles
	// the block is facing toward.  This propogation spreads at 1 tile/tick, as to be visible and maniputate-able by the player.
	struct ForcePropagator {
		int tileIndex; // tile currently on.
		LocalDirection heading;

		ForcePropagator(int index, LocalDirection head) : tileIndex(index), heading(head) {
		}
	};

	// When a force block is destroyed or a line of force is impeded, the force propogated from that source needs to be destroyed
	// as well.  The force eater travels along the line of force at 1 tile/tick and 'eats' the force in each tile, if it is headed
	// in the same direction as the initially impeded force.
	struct ForceEater {
		int tileIndex;
		LocalDirection heading;

		ForceEater(int index, LocalDirection head) : tileIndex(index), heading(head) {}
	};

private:
	TileManager* p_tileManager;

	std::vector<ForcePropagator> forcePropagators;
	std::vector<ForceEater> forceEaters;

public:
	ForceManager(TileManager* tm) : p_tileManager(tm) {

	}

	// Spawns a force propogator that will extend a line of force at a rate of 1 tile/tick 
	// in a single direction until stopped.
	void createForcePropogator(int tileIndex, LocalDirection direction) {
		forcePropagators.push_back(ForcePropagator(tileIndex, direction));
	}

	void createForceEater(int tileIndex, LocalDirection direction) {
		forceEaters.push_back(ForceManager::ForceEater(tileIndex, direction));
	}

	// Spawns a force eater that will consume a line of force at a rate of 1 tile/tick.
	void removeForce();

	void update() {
		propagateForces();
		removeForces();
	}

private:
	// Loops through the force propagators and extends their lines of force by 1 tile.
	// if the line of foce hits a force sink or other line of force, it will kill the propogator.
	void propagateForces();

	// Loops through the force eaters and removes force from the tile they reside in before moving on.
	// If there is no longer any force (in the same direction) to eat, the eater will be terminated.
	void removeForces();
};