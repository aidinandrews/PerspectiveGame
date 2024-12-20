//#include "basisManager.h"
//
//void BasisManager::update() {
//	updateProducers();
//	updateConsumers();
//}
//
//void BasisManager::addBasis(Tile* node, LocalDirection orientation, BasisType basisType) {
//	switch (basisType) {
//	case BASIS_TYPE_NONE:
//		deleteBasis(node);
//		break;
//	case BASIS_TYPE_PRODUCER:
//		createProducer(node, ENTITY_TYPE_OMNI, orientation, LOCAL_ORIENTATION_0, false);
//		break;
//	case BASIS_TYPE_CONSUMER:
//		createConsumer(node, false);
//		break;
//	case BASIS_TYPE_FORCE_GENERATOR:
//		createForceGenerator(node, orientation, false);
//		break;
//	}
//}
//
//void BasisManager::deleteBasis(Tile* node) {
//	switch (node->basis.type) {
//	case BASIS_TYPE_PRODUCER:        deleteProducer(node);       break;
//	case BASIS_TYPE_CONSUMER:        deleteConsumer(node);       break;
//	case BASIS_TYPE_FORCE_GENERATOR: deleteForceGenerator(node); break;
//	}
//}
//
////bool BasisManager::createForceSink(Tile* tile, bool override) {
////	if (!override && tile->basis.type != BASIS_TYPE_NONE) {
////		return false;
////	}
////
////	tile->basis.type = BASIS_TYPE_FORCE_SINK;
////	tile->forceLocalDirection = LOCAL_DIRECTION_INVALID;
////
////	// The creation of a force sink may cut off a line of force from its 
////	// force block, so we need to spawn a force eater to take care of it now:
////	for (int i = 0; i < 4; i++) {
////		Tile* neighbor = tile->sideInfos.connectedTiles[i];
////		LocalDirection adjDir = TileNavigator::dirToDirMap(tile->type, neighbor->type, LocalDirection(i));
////
////		if (neighbor->forceLocalDirection == adjDir) {
////			p_forceManager->createForceEater(neighbor->index, adjDir);
////		}
////	}
////
////	// It should not be possible for an entity to get inside a force sink, so if there are any entities in the
////	// tile on placement of this sink, destroy them:
////	for (int i = 0; i < 9; i++) {
////		if (tile->entityIndices[i] != -1) {
////			p_entityManager->deleteEntity(&p_entityManager->entities[tile->entityIndices[i]]);
////		}
////	}
////	p_entityManager->internals.obstructionInfoChanges.push_back(internals.obstructionInfoSelectiveShift(tile->index, 0xffff, tile->index, 0xffff));
////
////	return true;
////}
////
////void BasisManager::deleteForceSink(Tile* tile) {
////	tile->basis.type = BASIS_TYPE_NONE;
////	p_entityManager->internals.obstructionInfoChanges.push_back(internals.obstructionInfoSelectiveShift(tile->index, 0xffff, tile->index, 0));
////
////	// When deleting a force sink, it may be that the sink was blocking a line of force.
////	// Now that the sink is not blocking it, that line of force has to be propogated out:
////	for (int i = 0; i < 4; i++) {
////		Tile* neighbor = tile->sideInfos.connectedTiles[i];
////		if (neighbor->hasForce() && neighbor->forceLocalDirection == tile->sideInfos.connectedSideIndices[i]) {
////			p_forceManager->createForcePropogator(tile->index, LocalDirection((i + 2) % 4));
////			continue;
////		}
////	}
////}
//
//bool BasisManager::createProducer(Tile* node, EntityType targetType, LocalDirection targetDirection, LocalOrientation targetOrientation, bool override) {
//	if (!override && node->basis.type != BASIS_TYPE_NONE) {
//		return false;
//	}
//	node->basis.type = BASIS_TYPE_PRODUCER;
//	producers.push_back(Producer(node->index, targetType, targetDirection, targetOrientation));
//	return true;
//}
//
//void BasisManager::deleteProducer(Tile* node) {
//	for (int i = 0; i < producers.size(); i++) {
//		if (producers[i].tileIndex == node->index) {
//			producers.erase(producers.begin() + i);
//			node->basis.type = BASIS_TYPE_NONE;
//			return;
//		}
//	}
//}
//
//bool BasisManager::createConsumer(Tile* node, bool override) {
//	if (!override && node->basis.type != BASIS_TYPE_NONE) {
//		return false;
//	}
//
//	node->basis.type = BASIS_TYPE_CONSUMER;
//
//	consumers.push_back(Consumer(node));
//
//	return true;
//}
//
//void BasisManager::deleteConsumer(Tile* node) {
//	for (int i = 0; i < consumers.size(); i++) {
//		if (consumers[i].tileIndex == node->index) {
//			consumers.erase(consumers.begin() + i);
//			node->basis.type = BASIS_TYPE_NONE;
//			return;
//		}
//	}
//}
//
//void BasisManager::updateProducers() {
//	for (Producer& producer : producers) {
//		p_entityManager->createEntity(producer.tileIndex, 
//									  producer.targetType, 
//									  producer.targetDirection, 
//									  producer.targetOrientation);
//	}
//}
//
//void BasisManager::updateConsumers() {
//	for (Consumer& consumer : consumers) {
//		consumer.update();
//
//		Tile* consumerTile = p_tileManager->tiles[consumer.tileIndex];
//
//		//int centerEntityIndex = consumerTile->entityIndices[LOCAL_POSITION_CENTER];
//		//if (centerEntityIndex != -1) {
//		//	// FIX THIS IT WILL LOOK ASS:
//		//	p_entityManager->deleteEntity(&p_entityManager->entities[centerEntityIndex]);
//		//}
//
//		/*if (consumerTile->hadEntity) {
//			p_entityManager->deleteEntity(consumerTile->getNeighbor(consumerTile->lastEntityDir));
//		}
//		else if (consumerTile->hasEntity(LocalPosition::OLD_LOCAL_POSITIONCENTER)) {
//			p_entityManager->deleteEntity(consumerTile);
//		}*/
//	}
//}
//
//bool BasisManager::createForceGenerator(Tile* node, LocalDirection orientation, bool override) {
//	/*if (!override && tile->hasBasis()) {
//		return false;
//	}
//	tile->basis.type = BASIS_TYPE_FORCE_GENERATOR;
//	tile->basis.localOrientation = orientation;
//	p_forceManager->createForcePropogator(tile->index, orientation);
//*/
//	return true;
//}
//
//void BasisManager::deleteForceGenerator(Tile* node) {
//	node->basis.type = BASIS_TYPE_NONE;
//
//	p_forceManager->createForceEater(node->index, node->basis.localOrientation);
//}