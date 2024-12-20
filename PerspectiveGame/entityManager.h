#pragma once
struct EntityManager {};

//#pragma once
//#include <iostream>
//#include <bitset>
//
//#include "tileNavigation.h"
//#include "tileManager.h"
//#include "entity.h"
//
//struct EntityEditDirectionInfo {
//	int tileIndex;
//	LocalPosition position;
//	LocalDirection componentToEdit;
//
//	EntityEditDirectionInfo(int tileIndex, LocalPosition position, LocalDirection componentToEdit) :
//		tileIndex(tileIndex), position(position), componentToEdit(componentToEdit)
//	{}
//};
//
//struct EntityAndTileInfoSwap {
//	int entityIndex;
//	int entityInfoIndex0;
//	int entityInfoIndex1;
//	EntityAndTileInfoSwap(int entityIndex, int infoIndex0, int infoIndex1) :
//		entityIndex(entityIndex), entityInfoIndex0(infoIndex0), entityInfoIndex1(infoIndex1)
//	{}
//};
//
//struct EntityManager {
//public:
//	TileManager* p_tileManager;
//
//	std::vector<Entity> entities;
//
//	std::vector<EntityEditDirectionInfo> componentsToAdd;
//	std::vector<EntityEditDirectionInfo> componentsToRemove;
//	std::vector<EntityAndTileInfoSwap> entityAndTileInfosToSwap;
//
//	// This vector eeps track of the number of entities in each tile (Max 9).
//	// It is used when updating the tile GPU info structs.
//	std::vector<int> entitiesInTiles;
//	std::vector<GPU_EntityInfo> GPU_entityInfos;
//	std::vector<GPU_EntityTileInfo> GPU_entityTileInfos;
//
//	GLuint entityGpuBufferID;
//	GLuint entityTileInfoGpuBufferID;
//
//public:
//	EntityManager(TileManager* tm);
//	~EntityManager();
//
//	// Returns one of the 4 possible tiles an entitiy can point to.
//	Tile* getTile(Entity* entity, int index) { return p_tileManager->tiles[entity->getTileIndex(index)]; }
//
//	// TODO: make it possible to create entities on middle edges and corners.
//	// Entities are always created at the center of tiles, otherwise tie collisions happen.
//	void createEntity(int tileIndex, EntityType type, LocalDirection direction, LocalOrientation orientation);
//	void deleteEntity(Entity* entity);
//
//	bool cornerCollision(Entity* entity, Tile* arrivingTile, int arriving, int edgeOffset);
//
//	void moveEntity(Entity* entity);
//	void moveEntities() { for (Entity& entity : entities) moveEntity(&entity); }
//
//	// Leavings are only needed to make sure that the tile gpu info struct knows about leaving entities.
//	// After they are added to the tile gpu info vector, they must be deleted so as not to confuse future
//	// collision detection steps!
//	void removeTileInfoLeavings();
//
//	void getSideOtherInfo(EntityEditDirectionInfo editInfo, Tile* node, int& neighborInfoIndex, LocalDirection& neighborComponent)
//	{
//		LocalDirection toNeighbor = editInfo.position;
//		Tile* neighbor = node->neighbors[toNeighbor];
//		LocalPosition neighborPosition = node->mapAlignmentToNeighbor(toNeighbor, tnav::oppositeAlignment(toNeighbor));
//		//neighborInfoIndex = neighbor->entityInfoIndices[neighborPosition];
//		neighborComponent = node->mapAlignmentToNeighbor(toNeighbor, editInfo.componentToEdit);
//	}
//
//	void removeOtherDirComponentSide(EntityEditDirectionInfo removeInfo, Tile* node, Entity* entity)
//	{
//		int neighborInfoIndex; LocalDirection neighborComponent;
//		getSideOtherInfo(removeInfo, node, neighborInfoIndex, neighborComponent);
//		entity->removeDirectionFlag(neighborInfoIndex, tnav::getDirectionFlag(neighborComponent));
//		entity->setDirection(neighborInfoIndex, tnav::getDirection(entity->getDirectionFlag(neighborInfoIndex)));
//	}
//
//	void addOtherDirComponentSide(EntityEditDirectionInfo addInfo, Tile* node, Entity* entity)
//	{
//		int neighborInfoIndex; LocalDirection neighborComponent;
//		getSideOtherInfo(addInfo, node, neighborInfoIndex, neighborComponent);
//		entity->addDirectionFlag(neighborInfoIndex, tnav::getDirectionFlag(neighborComponent));
//		entity->setDirection(neighborInfoIndex, tnav::getDirection(entity->getDirectionFlag(neighborInfoIndex)));
//	}
//
//	// At the end of each collision solving step, all the adjustments to the entity directions have to be
//	// finalized.  They are not immediately finalized on discovery of a collision in case there is more than
//	// one collision associated with an entity, and changing its direction would cause the other collision to
//	// be lost.  This would mean that the way a collision is solved is dependant on the order it is found, which
//	// is no good!
//	void updateEntityLocalDirectionComponents()
//	{
//		/*for (EntityEditDirectionInfo removeInfo : componentsToRemove) {
//			Tile* tile = p_tileManager->tiles[removeInfo.tileIndex];
//			Entity* entity = &entities[tile->entityIndices[removeInfo.position]];
//			int infoIndex = tile->entityInfoIndices[removeInfo.position];
//
//			entity->removeDirectionFlag(infoIndex, tnav::getDirectionFlag(removeInfo.componentToEdit));
//			entity->setDirection(infoIndex, tnav::getDirection(entity->getDirectionFlag(infoIndex)));
//
//			switch (removeInfo.position) {
//			case LOCAL_POSITION_0: removeOtherDirComponentSide(removeInfo, tile, entity); continue;
//			case LOCAL_POSITION_1: removeOtherDirComponentSide(removeInfo, tile, entity); continue;
//			case LOCAL_POSITION_2: removeOtherDirComponentSide(removeInfo, tile, entity); continue;
//			case LOCAL_POSITION_3: removeOtherDirComponentSide(removeInfo, tile, entity); continue;
//			case LOCAL_POSITION_0_1:
//			case LOCAL_POSITION_1_2:
//			case LOCAL_POSITION_2_3:
//			case LOCAL_POSITION_3_0:
//				continue;
//			}
//		}
//		for (EntityEditDirectionInfo addInfo : componentsToAdd) {
//			Tile* tile = p_tileManager->tiles[addInfo.tileIndex];
//			Entity* entity = &entities[tile->entityIndices[addInfo.position]];
//			int infoIndex = tile->entityInfoIndices[addInfo.position];
//
//			entity->addDirectionFlag(infoIndex, tnav::getDirectionFlag(addInfo.componentToEdit));
//			entity->setDirection(infoIndex, tnav::getDirection(entity->getDirectionFlag(infoIndex)));
//			
//			switch (addInfo.position) {
//			case LOCAL_POSITION_0: addOtherDirComponentSide(addInfo, tile, entity); continue;
//			case LOCAL_POSITION_1: addOtherDirComponentSide(addInfo, tile, entity); continue;
//			case LOCAL_POSITION_2: addOtherDirComponentSide(addInfo, tile, entity); continue;
//			case LOCAL_POSITION_3: addOtherDirComponentSide(addInfo, tile, entity); continue;
//			case LOCAL_POSITION_0_1:
//			case LOCAL_POSITION_1_2:
//			case LOCAL_POSITION_2_3:
//			case LOCAL_POSITION_3_0: continue;
//			}
//		}
//		for (EntityAndTileInfoSwap s : entityAndTileInfosToSwap) {
//			Entity* e = &entities[s.entityIndex];
//
//			Tile* tile0 = p_tileManager->tiles[e->getTileIndex(s.entityInfoIndex0)];
//			int pos0 = e->getPosition(s.entityInfoIndex0);
//			tile0->entityInfoIndices[pos0] = s.entityInfoIndex1;
//
//			Tile* tile1 = p_tileManager->tiles[e->getTileIndex(s.entityInfoIndex1)];
//			int pos1 = e->getPosition(s.entityInfoIndex1);
//			tile1->entityInfoIndices[pos1] = s.entityInfoIndex0;
//
//			e->swapTileInfos(s.entityInfoIndex0, s.entityInfoIndex1);
//		}
//		componentsToRemove.clear();
//		componentsToAdd.clear();
//		entityAndTileInfosToSwap.clear();*/
//	}
//
//	/*bool tryAddDirectionComponent(Entity* entity, enav::OrthogonalEntityCollisionInfo* info)
//	{
//		if (info->entityIndex == -1) {
//			return false;
//		}
//
//		Entity* collidingEntity = &entities[info->entityIndex];
//		LocalDirection collidingEntityDir = collidingEntity->getDirection(info->entityInfoIndex);
//		bool noCollision = tnav::alignmentHasComponent(collidingEntityDir, info->componentToEdit);
//
//		if (noCollision) {
//			return false;
//		}
//		else {
//			entityDirComponentsToAdd.push_back(EntityDirectionComponentAddition(
//				collidingEntity->index, info->entityInfoIndex, tnav::getDirectionFlag(info->componentToEdit)));
//			return true;
//		}
//	}
//
//	bool tryAddDirectionComponent(Entity* entity, enav::DiagonalEntityCollisionInfo* info)
//	{
//		if (info->entityIndex == -1) {
//			return false;
//		}
//
//		Entity* collidingEntity = &entities[info->entityIndex];
//		LocalDirection collidingEntityDir = collidingEntity->getDirection(info->entityInfoIndex);
//		bool collision = tnav::alignmentHasComponent(collidingEntityDir, info->cornerDirectionOfCollision);
//		
//		if (collidingEntity->isStatic() &&
//			tnav::alignmentHasComponent(entity->getDirection(0), tnav::oppositeAlignment(info->cornerDirectionOfCollision))) {
//			collision = true;
//		}
//
//		if (collision) {
//			entityDirComponentsToAdd.push_back(EntityDirectionComponentAddition(
//				collidingEntity->index, info->entityInfoIndex, tnav::getDirectionFlag(info->componentToEdit)));
//			return true;
//		}
//		else {
//			return false;
//		}
//	}*/
//	
//	
//	
//	bool getCollidingInfoFromCenterToOffsetAndCorner(
//		Entity* entity, Tile* neighbor, int offset, LocalDirection heading, LocalDirection neighborHeading)
//	{
//		//using namespace tnav;
//
//		//Tile* tile = getTile(entity, 0);
//
//		//LocalPosition neighborPosition = LocalPosition((heading + offset) % 4);
//		//neighborPosition = tile->mapAlignmentToNeighbor(heading, neighborPosition);
//		//
//		//// Offset collision:
//		//if (neighbor->hasEntity(neighborPosition)) {
//		//	Entity* collider = &entities[neighbor->entityIndices[neighborPosition]];
//		//	if (collider->getDirection(0) == neighborHeading) {
//		//		// There is a collision!
//		//		componentsToAdd.push_back(EntityEditDirectionInfo(neighbor->index, neighborPosition, neighborHeading));
//		//		return true;
//		//	}
//		//}
//		//// Corner collision:
//		//Tile* neighborNeighbor = neighbor->neighbors[neighborPosition];
//		//if (neighborNeighbor->hasEntity(LOCAL_POSITION_CENTER)) {
//		//	Entity* collider = &entities[neighborNeighbor->entityIndices[LOCAL_POSITION_CENTER]];
//		//	LocalDirection dirOfCollision = oppositeAlignment(neighbor->mapAlignmentToNeighbor(LocalDirection(neighborPosition), LocalDirection(neighborPosition)));
//
//		//	if (alignmentHasComponent(collider->getDirection(0), dirOfCollision)) {
//		//		// There is a collision!
//		//		LocalDirection neighborNeighborDirection = neighbor->mapAlignmentToNeighbor(LocalDirection(neighborPosition), neighborHeading);
//		//		componentsToAdd.push_back(EntityEditDirectionInfo(neighborNeighbor->index, LOCAL_POSITION_CENTER, neighborNeighborDirection));
//		//		return true;
//		//	}
//		//}
//		return false;
//	}
//
//	// 'infos' == pointer to array of size 2 and represents the component to remove.
//	// returns the number of entities collided with, either 0, 1, or 2.
//	void solveCollidingInfoFromCenterOrtho(Entity* entity, LocalDirection heading)
//	{
//		//using namespace tnav;
//
//		//Tile* tile = getTile(entity, 0);
//		//
//		//Tile* neighbor = tile->neighbors[heading];
//		//LocalDirection neighborHeading = tile->mapAlignmentToNeighbor(heading, heading);
//
//		//// Direct collision:
//		//if (neighbor->hasEntity(LOCAL_POSITION_CENTER)) {
//		//	Entity* collider = &entities[neighbor->entityIndices[LOCAL_POSITION_CENTER]];
//		//	if (alignmentHasComponent(collider->getDirection(0), neighborHeading) == false) {
//		//		// There is a collision!
//		//		componentsToAdd.push_back(EntityEditDirectionInfo(neighbor->index, LOCAL_POSITION_CENTER, neighborHeading));
//		//		componentsToRemove.push_back(EntityEditDirectionInfo(tile->index, LOCAL_POSITION_CENTER, heading));
//		//		return;
//		//	}
//		//}
//
//		//bool collision = false;
//		//collision |= getCollidingInfoFromCenterToOffsetAndCorner(entity, neighbor, 3, heading, neighborHeading);
//		//collision |= getCollidingInfoFromCenterToOffsetAndCorner(entity, neighbor, 1, heading, neighborHeading);
//		//if (collision) {
//		//	componentsToRemove.push_back(EntityEditDirectionInfo(tile->index, LOCAL_POSITION_CENTER, heading));
//		//}
//	}
//
//	void findCollisionFromCenter(Entity* entity)
//	{
//		using namespace tnav;
//
//		Tile* currentTile = getTile(entity, 0);
//		if (isDiagonal(entity->getDirection(0))) {
//			const LocalDirection* components = getAlignmentComponents(entity->getDirection(0));
//			solveCollidingInfoFromCenterOrtho(entity, components[0]);
//			solveCollidingInfoFromCenterOrtho(entity, components[1]);
//		}
//		else {
//			solveCollidingInfoFromCenterOrtho(entity, entity->getDirection(0));
//		}
//	}
//
//	bool getCollidingInfoFromSideToOffsetAndCorner(Entity* entity, int offset)
//	{
//		//using namespace tnav;
//
//		//Tile* tile = getTile(entity, 0);
//		//LocalDirection heading = entity->getDirection(0);
//
//		//LocalPosition collidingPosition = combineAlignments(heading, LocalPosition((heading + offset) % 4));
//		//if (tile->hasEntity(collidingPosition) == false) {
//		//	return false;
//		//}
//		//Entity* collider = &entities[tile->entityIndices[collidingPosition]];
//		//int colliderInfoIndex = tile->entityInfoIndices[collidingPosition];
//		//if (alignmentHasComponent(collider->getDirection(colliderInfoIndex), heading)) {
//		//	return false;
//		//}
//		//// there is a collision!
//		//componentsToAdd.push_back(EntityEditDirectionInfo(tile->index, collidingPosition, heading));
//		//return true;
//		// corner collisions from this position are impossible so the check is skipped.
//	}
//
//	// only orthogonal directions are valid from a side position.
//	void findCollisionFromSide(Entity* entity)
//	{
//		//using namespace tnav;
//
//		//Tile* tile = getTile(entity, 0);
//		//LocalDirection heading = entity->getDirection(0);
//		//LocalPosition position = entity->getPosition(0);
//
//		//// Direct collision:
//		//LocalPosition colliderPosition = getNextNextPosition(position, heading);
//		//if (tile->hasEntity(colliderPosition)) {
//		//	Entity* collider = &entities[tile->entityIndices[colliderPosition]];
//		//	if (alignmentHasComponent(collider->getDirection(0), heading) == false) {
//		//		// There is a collision!
//		//		componentsToAdd.push_back(EntityEditDirectionInfo(tile->index, colliderPosition, heading));
//		//		componentsToRemove.push_back(EntityEditDirectionInfo(tile->index, position, heading));
//		//		entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(collider->index, 0, 1));
//		//		return;
//		//	}
//		//}
//
//		//bool collision = false;
//		//collision |= getCollidingInfoFromSideToOffsetAndCorner(entity, 1);
//		//collision |= getCollidingInfoFromSideToOffsetAndCorner(entity, 3);
//		//if (collision) {
//		//	componentsToRemove.push_back(EntityEditDirectionInfo(tile->index, position, heading));
//		//}
//	}
//
//	bool findCollisionComponentFromCornerToOffsetAndCornerInternal(Entity* entity, LocalDirection heading)
//	{
//		//using namespace tnav;
//
//		//Tile* tile = getTile(entity, 0);
//		//LocalPosition position = entity->getPosition(0);
//		//LocalPosition colliderPosition = heading;
//		//
//		//if (tile->hasEntity(colliderPosition)) {
//		//	Entity* collider = &entities[tile->entityIndices[colliderPosition]];
//		//	int colliderInfoIndex = tile->entityInfoIndices[colliderPosition];
//		//	// side-positioned entities must have an orthogonal direction:
//		//	if (collider->getDirection(colliderInfoIndex) != heading) {
//		//		// collision spotted!
//		//		componentsToAdd.push_back(EntityEditDirectionInfo(tile->index, colliderPosition, heading));
//		//		return true;
//		//	}
//		//}
//
//		//colliderPosition = oppositeAlignment(position);
//		//if (tile->hasEntity(colliderPosition)) {
//		//	Entity* collider = &entities[tile->entityIndices[colliderPosition]];
//		//	int colliderInfoIndex = tile->entityInfoIndices[colliderPosition];
//		//	LocalDirection directionOfCollision = oppositeAlignment(getOtherComponent(colliderPosition, heading));
//		//	// corner-positioned entities must have a diagonal direction:
//		//	if (alignmentHasComponent(collider->getDirection(colliderInfoIndex), directionOfCollision)) {
//		//		// collision spotted!
//		//		componentsToAdd.push_back(EntityEditDirectionInfo(tile->index, colliderPosition, heading));
//		//		return true;
//		//	}
//		//}
//		return false;
//	}
//
//	bool findCollisionComponentFromCornerToOffsetAndCornerExternal(Entity* entity, LocalDirection heading)
//	{
//		//using namespace tnav;
//
//		//Tile* tile = getTile(entity, 0);
//		//LocalPosition position = entity->getPosition(0);
//
//		//LocalDirection toNeighborDirection = getOtherComponent(position, oppositeAlignment(heading));
//		//Tile* neighbor = tile->neighbors[toNeighborDirection];
//		//LocalDirection neighborHeading = tile->mapAlignmentToNeighbor(toNeighborDirection, heading);
//		//LocalPosition colliderPosition = neighborHeading;
//
//		//if (neighbor->hasEntity(colliderPosition)) {
//		//	Entity* collider = &entities[neighbor->entityIndices[colliderPosition]];
//		//	int colliderInfoIndex = neighbor->entityInfoIndices[colliderPosition];
//		//	// side-positioned entities must have an orthogonal direction:
//		//	if (collider->getDirection(colliderInfoIndex) != neighborHeading) {
//		//		// collision spotted!
//		//		componentsToAdd.push_back(EntityEditDirectionInfo(neighbor->index, colliderPosition, neighborHeading));
//		//		return true;
//		//	}
//		//}
//
//		//colliderPosition = combineAlignments(heading, toNeighborDirection);
//		//colliderPosition = tile->mapAlignmentToNeighbor(toNeighborDirection, colliderPosition);
//		//if (neighbor->hasEntity(colliderPosition)) {
//		//	Entity* collider = &entities[neighbor->entityIndices[colliderPosition]];
//		//	int colliderInfoIndex = neighbor->entityInfoIndices[colliderPosition];
//		//	LocalDirection directionOfCollision = oppositeAlignment(toNeighborDirection);
//		//	directionOfCollision = tile->mapAlignmentToNeighbor(toNeighborDirection, directionOfCollision);
//		//	// corner-positioned entities must have a diagonal direction:
//		//	if (alignmentHasComponent(collider->getDirection(colliderInfoIndex), directionOfCollision)) {
//		//		// collision spotted!
//		//		componentsToAdd.push_back(EntityEditDirectionInfo(neighbor->index, colliderPosition, neighborHeading));
//		//		return true;
//		//	}
//		//}
//		return false;
//	}
//
//
//	void findCollisionComponentFromCorner(Entity* entity, LocalDirection heading)
//	{
//		//using namespace tnav;
//
//		//Tile* tile = getTile(entity, 0);
//		//LocalPosition position = entity->getPosition(0);
//
//		//// direct collision:
//		//LocalPosition colliderPosition = getNextNextPosition(position, heading);
//		//if (tile->hasEntity(colliderPosition)) {
//		//	Entity* collider = &entities[tile->entityIndices[colliderPosition]];
//		//	int colliderInfoIndex = tile->entityInfoIndices[colliderPosition];
//		//	if (alignmentHasComponent(collider->getDirection(colliderInfoIndex), heading) == false) {
//		//		// collision spotted!
//		//		componentsToAdd.push_back(EntityEditDirectionInfo(tile->index, colliderPosition, heading));
//		//		componentsToRemove.push_back(EntityEditDirectionInfo(tile->index, position, heading));
//		//		return;
//		//	}
//		//}
//
//		//bool collision = findCollisionComponentFromCornerToOffsetAndCornerInternal(entity, heading);
//		//collision |= findCollisionComponentFromCornerToOffsetAndCornerExternal(entity, heading);
//		//if (collision) {
//		//	componentsToRemove.push_back(EntityEditDirectionInfo(tile->index, position, heading));
//		//}
//	}
//
//	// Entity MUST have diagonal direction
//	void findCollisionFromCorner(Entity* entity)
//	{
//		const LocalDirection* components = tnav::getAlignmentComponents(entity->getDirection(0));
//		findCollisionComponentFromCorner(entity, components[0]);
//		findCollisionComponentFromCorner(entity, components[1]);
//	}
//
//	void findCollision(Entity* entity)
//	{
//		if (entity->getDirection(0) == LOCAL_DIRECTION_STATIC) {
//			return;
//		}
//
//		switch (entity->getPosition(0)) {
//		case LOCAL_POSITION_CENTER: findCollisionFromCenter(entity); break;
//		case LOCAL_POSITION_0: findCollisionFromSide(entity); break;
//		case LOCAL_POSITION_1: findCollisionFromSide(entity); break;
//		case LOCAL_POSITION_2: findCollisionFromSide(entity); break;
//		case LOCAL_POSITION_3: findCollisionFromSide(entity); break;
//		case LOCAL_POSITION_0_1: findCollisionFromCorner(entity); break;
//		case LOCAL_POSITION_1_2: findCollisionFromCorner(entity); break;
//		case LOCAL_POSITION_2_3: findCollisionFromCorner(entity); break;
//		case LOCAL_POSITION_3_0: findCollisionFromCorner(entity); break;
//		}
//	}
//
//	void findInitialCollisions()
//	{
//		for (Entity& entity : entities) {
//			if (entity.getDirection(0) == LOCAL_DIRECTION_STATIC) {
//				continue;
//			}
//			findCollision(&entity);
//		}
//	}
//
//	void disperseAndFindCollisions()
//	{
//
//	}
//
//	void solveCollisions()
//	{
//		findInitialCollisions();
//		updateEntityLocalDirectionComponents();
//	}
//
//	void update()
//	{
//		solveCollisions();
//		updateGpuInfos();
//		moveEntities();
//	}
//
//	void updateGpuInfos();
//};