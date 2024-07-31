#include "basisManager.h"

void BasisManager::update() {
	updateProducers();
	updateConsumers();
}

void BasisManager::addBasis(Tile* tile, LocalDirection orientation, BasisType basisType) {
	switch (basisType) {
	case BasisType::NONE:
		deleteBasis(tile);
		break;
	case BasisType::PRODUCER:
		createProducer(tile, Entity::Type::MATERIAL_A, false);
		break;
	case BasisType::CONSUMER:
		createConsumer(tile, false);
		break;
	case BasisType::FORCE_SINK:
		createForceSink(tile, false);
		break;
	case BasisType::FORCE_GENERATOR:
		createForceGenerator(tile, orientation, false);
		break;
	}
}

void BasisManager::deleteBasis(Tile* tile) {
	switch (tile->basis.type) {
	case BasisType::PRODUCER:        deleteProducer(tile);       break;
	case BasisType::CONSUMER:        deleteConsumer(tile);       break;
	case BasisType::FORCE_SINK:      deleteForceSink(tile);      break;
	case BasisType::FORCE_GENERATOR: deleteForceGenerator(tile); break;
	}
}

bool BasisManager::createForceSink(Tile* tile, bool override) {
	if (!override && tile->basis.type != BasisType::NONE) {
		return false;
	}

	tile->basis.type = BasisType::FORCE_SINK;
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
	return true;
}

void BasisManager::deleteForceSink(Tile* tile) {
	tile->basis.type = BasisType::NONE;

	// When deleting a force sink, it may be that the sink was blocking a line of force.
	// Now that the sink is not blocking it, that line of force has to be propogated out:
	for (int i = 0; i < 4; i++) {
		Tile* neighbor = tile->sideInfos.connectedTiles[i];

		LocalDirection adjDir = LocalDirection((TileNavigator::dirToDirMap(tile->type, neighbor->type, LocalDirection(i)) + 2) % 4);

		if (neighbor->hasForce() && (neighbor->forceLocalDirection == adjDir)) {
			p_forceManager->createForcePropogator(tile->index, LocalDirection((i + 2) % 4));
			continue;
		}
	}
}

bool BasisManager::createProducer(Tile* tile, Entity::Type producedEntityType, bool override) {
	if (!override && tile->basis.type != BasisType::NONE) {
		return false;
	}

	tile->basis.type = BasisType::PRODUCER;

	producers.push_back(Producer(tile, producedEntityType));

	return true;
}

void BasisManager::deleteProducer(Tile* tile) {
	for (int i = 0; i < producers.size(); i++) {
		if (producers[i].tileIndex == tile->index) {
			producers.erase(producers.begin() + i);
			tile->basis.type = BasisType::NONE;
			return;
		}
	}
}

bool BasisManager::createConsumer(Tile* tile, bool override) {
	if (!override && tile->basis.type != BasisType::NONE) {
		return false;
	}

	tile->basis.type = BasisType::CONSUMER;

	consumers.push_back(Consumer(tile));

	return true;
}

void BasisManager::deleteConsumer(Tile* tile) {
	for (int i = 0; i < consumers.size(); i++) {
		if (consumers[i].tileIndex == tile->index) {
			consumers.erase(consumers.begin() + i);
			tile->basis.type = BasisType::NONE;
			return;
		}
	}
}

void BasisManager::updateProducers() {
	for (Producer& producer : producers) {
		p_entityManager->createEntity(producer.tileIndex, producer.producedEntityType, producer.producedEntityLocalDirection, false);
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
	tile->basis.type = BasisType::FORCE_GENERATOR;
	tile->basis.localOrientation = orientation;
	p_forceManager->createForcePropogator(tile->index, orientation);

	return true;
}

void BasisManager::deleteForceGenerator(Tile* tile) {
	tile->basis.type = BasisType::NONE;

	p_forceManager->createForceEater(tile->index, tile->basis.localOrientation);
}