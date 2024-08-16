#pragma once
#include <iostream>

#include "forceManager.h"
#include "entityManager.h"

struct BasisManager {
private:
	struct Producer {
		EntityType targetType;
		LocalDirection targetDirection;
		LocalOrientation targetOrientation;
		int tileIndex;

		Producer(int tileIndex, EntityType type, LocalDirection direction, LocalOrientation orientation) :
			tileIndex(tileIndex), targetType(type), targetDirection(direction), targetOrientation(orientation)
		{}
	};

	struct Consumer {
		int tileIndex;

		Consumer()
		{
			tileIndex = 0;
		}

		Consumer(Tile* tile) : tileIndex(tile->index)
		{}

		void update()
		{}
	};

private:
	TileManager* p_tileManager;
	ForceManager* p_forceManager;
	EntityManager* p_entityManager;

	std::vector<Producer> producers;
	std::vector<Consumer> consumers;

public:
	BasisManager(TileManager* tm, ForceManager* fm, EntityManager* em) :
		p_tileManager(tm), p_forceManager(fm), p_entityManager(em)
	{}

	void update();

	void addBasis(Tile* tile, LocalDirection orientation, BasisType basisType);
	void deleteBasis(Tile* tile);

	//bool createForceSink(Tile* tile, bool override);
	//void deleteForceSink(Tile* tile);

	bool createProducer(Tile* tile, EntityType targetType, LocalDirection targetDirection, LocalOrientation targetOrientation, bool override);
	void updateProducers();
	void deleteProducer(Tile* tile);

	bool createConsumer(Tile* tile, bool override);
	void updateConsumers();
	void deleteConsumer(Tile* tile);

	bool createForceGenerator(Tile* tile, LocalDirection orientation, bool override);
	void deleteForceGenerator(Tile* tile);
};