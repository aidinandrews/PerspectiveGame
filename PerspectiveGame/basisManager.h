#pragma once
#include <iostream>

#include "forceManager.h"

struct BasisManager {
private:
	struct Producer {
		Entity::Type producedEntityType;
		LocalDirection producedEntityLocalDirection;
		int tileIndex;

		Producer() {
			producedEntityType = Entity::Type::NONE;
			tileIndex = 0;
		}

		Producer(int ti, Entity::Type et) : tileIndex(ti), producedEntityType(et) {
		}
	};

	struct Consumer {
		int cooldown;
		int maxCooldown;
		int tileIndex;

		Consumer() {
			cooldown = 0;
			tileIndex = 0;
		}

		Consumer(int tileIndex) : tileIndex(tileIndex) {
			cooldown = 0;
			maxCooldown = 1;
		}

		void update() {
			if (cooldown > 0.0f) { cooldown--; }
		}
	};

private:
	TileManager* p_tileManager;
	ForceManager* p_forceManager;
	EntityManager* p_entityManager;

	std::vector<Producer> producers;
	std::vector<Consumer> consumers;

public:
	BasisManager(TileManager* tm, ForceManager* fm, EntityManager*em) :p_tileManager(tm), p_forceManager(fm),p_entityManager(em) {}

	void update() {
		updateProducers();
		updateConsumers();

		//std::cout << "producers:" << producers.size() << std::endl;
		std::cout <<"entities:"<< p_entityManager->entities.size() << std::endl;
	}

	void addBasis(Tile* tile, Tile::Basis::Type basisType) {
		switch (basisType) {
		case Tile::Basis::Type::NONE:
			deleteBasis(tile);
			break;
		case Tile::Basis::Type::PRODUCER:
			createProducer(tile->index, Entity::Type::MATERIAL_A, false);
			break;
		case Tile::Basis::Type::CONSUMER:
			createConsumer(tile->index, false);
			break;
		case Tile::Basis::Type::FORCE_SINK:
			createForceSink(tile->index, false);
			break;
		}
	}

	void deleteBasis(Tile* tile) {
		switch (tile->basis.type) {
		case Tile::Basis::Type::PRODUCER:
			deleteProducer(tile);
			break;
		case Tile::Basis::Type::CONSUMER:
			deleteConsumer(tile);
			break;
		case Tile::Basis::Type::FORCE_SINK:
			deleteForceSink(tile);
			break;
		}
	}

	bool createForceSink(int tileIndex, bool override) {
		if (!override && p_tileManager->tiles[tileIndex]->basis.type != Tile::Basis::Type::NONE) {
			return false;
		}

		Tile* tile = p_tileManager->tiles[tileIndex];

		tile->basis.type = Tile::Basis::Type::FORCE_SINK;
		//delete tile->entity;
		//tile->entity = nullptr;
		tile->force.magnitude = 0;

		// The creation of a force sink may cut off a line of force from its 
		// force block, so we need to spawn a force eater to take care of it now:
		for (int i = 0; i < 4; i++) {
			Tile* neighbor = tile->sideInfos.connectedTiles[i];
			LocalDirection adjDir = TileNavigator::dirToDirMap(tile->type, neighbor->type, LocalDirection(i));

			if (neighbor->force.direction == adjDir) {
				p_forceManager->createForceEater(neighbor->index, adjDir);
			}
		}
		return true;
	}

	void deleteForceSink(Tile* tile) {

		tile->basis.type = Tile::Basis::Type::NONE;

		// When deleting a force sink, it may be that the sink was blocking a line of force.
		// Now that the sink is not blocking it, that line of force has to be propogated out:
		for (int i = 0; i < 4; i++) {
			Tile* neighbor = tile->sideInfos.connectedTiles[i];

			LocalDirection adjDir = LocalDirection((TileNavigator::dirToDirMap(tile->type, neighbor->type, LocalDirection(i)) + 2) % 4);

			if ((neighbor->force.direction == adjDir)) {
				p_forceManager->createForcePropogator(tile->index, LocalDirection((i + 2) % 4));
				continue;
			}
		}
	}

	bool createProducer(int tileIndex, Entity::Type producedEntityType, bool override) {
	if (!override && p_tileManager->tiles[tileIndex]->basis.type != Tile::Basis::Type::NONE) {
		return false;
	}

	p_tileManager->tiles[tileIndex]->basis.type = Tile::Basis::Type::PRODUCER;

	producers.push_back(Producer(tileIndex, producedEntityType));

	return true;
}

	void deleteProducer(Tile* tile) {
	for (int i = 0; i < producers.size(); i++) {
		if (producers[i].tileIndex == tile->index) {
			producers.erase(producers.begin() + i);
			tile->basis.type = Tile::Basis::Type::NONE;
			return;
		}
	}
}

	bool createConsumer(int tileIndex, bool override) {
	if (!override && p_tileManager->tiles[tileIndex]->basis.type != Tile::Basis::Type::NONE) {
		return false;
	}

	p_tileManager->tiles[tileIndex]->basis.type = Tile::Basis::Type::CONSUMER;

	consumers.push_back(Consumer(tileIndex));

	return true;
}

	void deleteConsumer(Tile* tile) {
	for (int i = 0; i < consumers.size(); i++) {
		if (consumers[i].tileIndex == tile->index) {
			consumers.erase(consumers.begin() + i);
			tile->basis.type = Tile::Basis::Type::NONE;
			return;
		}
	}
}

	void updateProducers() {
	for (Producer& producer : producers) {
		p_entityManager->createEntity(producer.tileIndex, producer.producedEntityType, producer.producedEntityLocalDirection, false);
	}
}
	
	void updateConsumers() {
	for (Consumer& consumer : consumers) {
		consumer.update();
		if (consumer.cooldown > 0) { continue; }

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
};