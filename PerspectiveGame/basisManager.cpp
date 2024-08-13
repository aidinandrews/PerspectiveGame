#include "basisManager.h"

void BasisManager::update() {
	updateProducers();
	updateConsumers();
}

void BasisManager::addBasis(Tile* tile, LocalDirection orientation, BasisID basisType) {
	switch (basisType) {
	case BASIS_TYPE_NONE:
		deleteBasis(tile);
		break;
	case BASIS_TYPE_PRODUCER:
		createProducer(tile, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_0, false);
		break;
	case BASIS_TYPE_CONSUMER:
		createConsumer(tile, false);
		break;
	case BASIS_TYPE_FORCE_SINK:
		createForceSink(tile, false);
		break;
	case BASIS_TYPE_FORCE_GENERATOR:
		createForceGenerator(tile, orientation, false);
		break;
	}
}

void BasisManager::deleteBasis(Tile* tile) {
	switch (tile->basis.type) {
	case BASIS_TYPE_PRODUCER:        deleteProducer(tile);       break;
	case BASIS_TYPE_CONSUMER:        deleteConsumer(tile);       break;
	case BASIS_TYPE_FORCE_SINK:      deleteForceSink(tile);      break;
	case BASIS_TYPE_FORCE_GENERATOR: deleteForceGenerator(tile); break;
	}
}

bool BasisManager::createForceSink(Tile* tile, bool override) {
	if (!override && tile->basis.type != BASIS_TYPE_NONE) {
		return false;
	}

	tile->basis.type = BASIS_TYPE_FORCE_SINK;
	tile->forceLocalDirection = LOCAL_DIRECTION_INVALID;

	// The creation of a force sink may cut off a line of force from its 
	// force block, so we need to spawn a force eater to take care of it now:
	for (int i = 0; i < 4; i++) {
		Tile* neighbor = tile->sideInfos.connectedTiles[i];
		LocalDirection adjDir = TileNavigator::dirToDirMap(tile->type, neighbor->type, LocalDirection(i));

		if (neighbor->forceLocalDirection == adjDir) {
			p_forceManager->createForceEater(neighbor->index, adjDir);
		}
	}

	// It should not be possible for an entity to get inside a force sink, so if there are any entities in the
	// tile on placement of this sink, destroy them:
	for (int i = 0; i < 9; i++) {
		if (tile->entityIndices[i] != -1) {
			p_entityManager->deleteEntity(&p_entityManager->entities[tile->entityIndices[i]]);
		}
	}
	p_entityManager->obstructionMaskChanges.push_back(ObstructionMaskChange(tile->index, 0xffff, tile->index, 0xffff));

	return true;
}

void BasisManager::deleteForceSink(Tile* tile) {
	tile->basis.type = BASIS_TYPE_NONE;
	p_entityManager->obstructionMaskChanges.push_back(ObstructionMaskChange(tile->index, 0xffff, tile->index, 0));

	// When deleting a force sink, it may be that the sink was blocking a line of force.
	// Now that the sink is not blocking it, that line of force has to be propogated out:
	for (int i = 0; i < 4; i++) {
		Tile* neighbor = tile->sideInfos.connectedTiles[i];
		if (neighbor->hasForce() && neighbor->forceLocalDirection == tile->sideInfos.connectedSideIndices[i]) {
			p_forceManager->createForcePropogator(tile->index, LocalDirection((i + 2) % 4));
			continue;
		}
	}
}

bool BasisManager::createProducer(Tile* tile, EntityID producedEntityType, LocalOrientation orientation, bool override) {
	if (!override && tile->basis.type != BASIS_TYPE_NONE) {
		return false;
	}

	tile->basis.type = BASIS_TYPE_PRODUCER;

	producers.push_back(Producer(tile, producedEntityType, orientation));

	return true;
}

void BasisManager::deleteProducer(Tile* tile) {
	for (int i = 0; i < producers.size(); i++) {
		if (producers[i].tileIndex == tile->index) {
			producers.erase(producers.begin() + i);
			tile->basis.type = BASIS_TYPE_NONE;
			return;
		}
	}
}

bool BasisManager::createConsumer(Tile* tile, bool override) {
	if (!override && tile->basis.type != BASIS_TYPE_NONE) {
		return false;
	}

	tile->basis.type = BASIS_TYPE_CONSUMER;

	consumers.push_back(Consumer(tile));

	return true;
}

void BasisManager::deleteConsumer(Tile* tile) {
	for (int i = 0; i < consumers.size(); i++) {
		if (consumers[i].tileIndex == tile->index) {
			consumers.erase(consumers.begin() + i);
			tile->basis.type = BASIS_TYPE_NONE;
			return;
		}
	}
}

void BasisManager::updateProducers() {
	for (Producer& producer : producers) {
		p_entityManager->createEntity(producer.tileIndex, producer.producedEntityType, producer.producedEntityLocalOrientation, false);
	}
}

void BasisManager::updateConsumers() {
	for (Consumer& consumer : consumers) {
		consumer.update();

		Tile* consumerTile = p_tileManager->tiles[consumer.tileIndex];

		int centerEntityIndex = consumerTile->entityIndices[LOCAL_POSITION_CENTER];
		if (centerEntityIndex != -1) {
			// FIX THIS IT WILL LOOK ASS:
			p_entityManager->deleteEntity(&p_entityManager->entities[centerEntityIndex]);
		}

		/*if (consumerTile->hadEntity) {
			p_entityManager->deleteEntity(consumerTile->getNeighbor(consumerTile->lastEntityDir));
		}
		else if (consumerTile->hasEntity(LocalPosition::LOCAL_POSITION_CENTER)) {
			p_entityManager->deleteEntity(consumerTile);
		}*/
	}
}

bool BasisManager::createForceGenerator(Tile* tile, LocalDirection orientation, bool override) {
	if (!override && tile->hasBasis()) {
		return false;
	}
	tile->basis.type = BASIS_TYPE_FORCE_GENERATOR;
	tile->basis.localOrientation = orientation;
	p_forceManager->createForcePropogator(tile->index, orientation);

	return true;
}

void BasisManager::deleteForceGenerator(Tile* tile) {
	tile->basis.type = BASIS_TYPE_NONE;

	p_forceManager->createForceEater(tile->index, tile->basis.localOrientation);
}