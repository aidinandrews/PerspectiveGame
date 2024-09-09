#pragma once
#include <iostream>
#include <bitset>

#include "tileNavigation.h"
#include "tileManager.h"
#include "entity.h"
#include "nextEntityInfo1.h"

struct EntityCollisionInfo {
	int tileIndex;
	LocalPosition position;
	LocalDirection direction;

	int collidingTileIndex;
	LocalPosition collidingPosition;
	LocalDirection collidingDirection;

	EntityCollisionInfo(int tileIndex, LocalPosition position) : tileIndex(tileIndex), position(position) {}
};

struct EntityDirectionComponentRemoval {
	int entityIndex;
	int entityInfoIndex;
	uint8_t componentToRemove;
	EntityDirectionComponentRemoval(int index, int infoIndex, uint8_t component) :
		entityIndex(index), entityInfoIndex(infoIndex), componentToRemove(component)
	{}
};
struct EntityDirectionComponentAddition {
	int entityIndex;
	int entityInfoIndex;
	uint8_t componentToAdd;
	EntityDirectionComponentAddition(int index, int infoIndex, uint8_t component) :
		entityIndex(index), entityInfoIndex(infoIndex), componentToAdd(component)
	{}
};

struct EntityAndTileInfoSwap {
	int entityIndex;
	int entityInfoIndex0;
	int entityInfoIndex1;
	EntityAndTileInfoSwap(int entityIndex, int infoIndex0, int infoIndex1) :
		entityIndex(entityIndex), entityInfoIndex0(infoIndex0), entityInfoIndex1(infoIndex1)
	{}
};

struct EntityManager {
public:
	TileManager* p_tileManager;

	std::vector<Entity> entities;
	std::vector<EntityCollisionInfo> entityCollisionInfos;
	std::vector<EntityDirectionComponentRemoval> entityDirComponentsToRemove;
	std::vector<EntityDirectionComponentAddition> entityDirComponentsToAdd;
	std::vector<EntityAndTileInfoSwap> entityAndTileInfosToSwap;

	// This vector eeps track of the number of entities in each tile (Max 9).
	// It is used when updating the tile GPU info structs.
	std::vector<int> entitiesInTiles;
	std::vector<GPU_EntityInfo> GPU_entityInfos;
	std::vector<GPU_EntityTileInfo> GPU_entityTileInfos;

	GLuint entityGpuBufferID;
	GLuint entityTileInfoGpuBufferID;

public:
	EntityManager(TileManager* tm);
	~EntityManager();

	// Returns one of the 4 possible tiles an entitiy can point to.
	Tile* getTile(Entity* entity, int index) { return p_tileManager->tiles[entity->getTileIndex(index)]; }

	// TODO: make it possible to create entities on middle edges and corners.
	// Entities are always created at the center of tiles, otherwise tie collisions happen.
	void createEntity(int tileIndex, EntityType type, LocalDirection direction, LocalOrientation orientation);
	void deleteEntity(Entity* entity);

	bool cornerCollision(Entity* entity, Tile* arrivingTile, int arriving, int edgeOffset);

	void moveEntity(Entity* entity);
	void moveEntities() { for (Entity& entity : entities) moveEntity(&entity); }

	// Leavings are only needed to make sure that the tile gpu info struct knows about leaving entities.
	// After they are added to the tile gpu info vector, they must be deleted so as not to confuse future
	// collision detection steps!
	void removeTileInfoLeavings();

	// At the end of each collision solving step, all the adjustments to the entity directions have to be
	// finalized.  They are not immediately finalized on discovery of a collision in case there is more than
	// one collision associated with an entity, and changing its direction would cause the other collision to
	// be lost.  This would mean that the way a collision is solved is dependant on the order it is found, which
	// is no good!
	void updateEntityLocalDirectionComponents()
	{
		for (EntityDirectionComponentRemoval sub : entityDirComponentsToRemove) {
			Entity* e = &entities[sub.entityIndex];
			e->removeDirectionFlag(sub.entityInfoIndex, sub.componentToRemove);
			e->setDirection(sub.entityInfoIndex, tnav::getDirection(e->getDirectionFlag(sub.entityInfoIndex)));
		}
		for (EntityDirectionComponentAddition add : entityDirComponentsToAdd) {
			Entity* e = &entities[add.entityIndex];
			e->addDirectionFlag(add.entityInfoIndex, add.componentToAdd);
			e->setDirection(add.entityInfoIndex, tnav::getDirection(e->getDirectionFlag(add.entityInfoIndex)));
		}
		for (EntityAndTileInfoSwap s : entityAndTileInfosToSwap) {
			Entity* e = &entities[s.entityIndex];

			Tile* tile0 = p_tileManager->tiles[e->getTileIndex(s.entityInfoIndex0)];
			int pos0 = e->getPosition(s.entityInfoIndex0);
			tile0->entityInfoIndices[pos0] = s.entityInfoIndex1;

			Tile* tile1 = p_tileManager->tiles[e->getTileIndex(s.entityInfoIndex1)];
			int pos1 = e->getPosition(s.entityInfoIndex1);
			tile1->entityInfoIndices[pos1] = s.entityInfoIndex0;

			e->swapTileInfos(s.entityInfoIndex0, s.entityInfoIndex1);
		}
		entityDirComponentsToRemove.clear();
		entityDirComponentsToAdd.clear();
		entityAndTileInfosToSwap.clear();
	}

	bool tryAddCollisionDirectionComponent(Entity* entity, enav::OrthogonalEntityCollisionInfo* info)
	{
		if (info->entityIndex == -1) {
			return false;
		}

		Entity* collidingEntity = &entities[info->entityIndex];
		LocalDirection collidingEntityDir = collidingEntity->getDirection(info->entityInfoIndex);
		bool noCollision = tnav::alignmentHasComponent(collidingEntityDir, info->equivelantDirection);

		if (noCollision) {
			return false;
		}
		else {
			entityDirComponentsToAdd.push_back(EntityDirectionComponentAddition(
				collidingEntity->index, info->entityInfoIndex, tnav::getDirectionFlag(info->equivelantDirection)));
			return true;
		}
	}

	bool tryAddCollisionDirectionComponent(Entity* entity, enav::DiagonalEntityCollisionInfo* info)
	{
		if (info->entityIndex == -1) {
			return false;
		}

		Entity* collidingEntity = &entities[info->entityIndex];
		LocalDirection collidingEntityDir = collidingEntity->getDirection(info->entityInfoIndex);
		bool collision = tnav::alignmentHasComponent(collidingEntityDir, info->cornerDirectionOfCollision);

		if (collision) {
			entityDirComponentsToAdd.push_back(EntityDirectionComponentAddition(
				collidingEntity->index, info->entityInfoIndex, tnav::getDirectionFlag(info->equivelantDirection)));
			return true;
		}
		else {
			return false;
		}
	}

	void findCollision(Entity* entity)
	{
		const LocalDirection* entityDirComponents = tnav::getAlignmentComponents(entity->getDirection(0));
		LocalDirection direction = entityDirComponents[0];
		LocalPosition position = entity->getPosition(0);
		Tile* tile = getTile(entity, 0);
		enav::OrthogonalEntityCollisionInfo oInfo;
		enav::DiagonalEntityCollisionInfo dInfo;
		bool collision = false;

		// NOTE: Shifted collisions from the center of a tile are impossible!
		if (entity->getPosition(0) == LOCAL_POSITION_CENTER) {
			// Direct collision:
			oInfo = enav::getNextEntityInfoFromCenterToDirect(tile, direction);
			collision |= tryAddCollisionDirectionComponent(entity, &oInfo);
			
			if (collision == false) {
				// Corner collisions:
				dInfo = enav::getNextEntityInfoFromCenterToCornerA(tile, direction);
				collision |= tryAddCollisionDirectionComponent(entity, &dInfo);
				dInfo = enav::getNextEntityInfoFromCenterToCornerB(tile, direction);
				collision |= tryAddCollisionDirectionComponent(entity, &dInfo);
			}
			
			if (collision) {
				entityDirComponentsToRemove.push_back(EntityDirectionComponentRemoval(
					entity->index, 0, tnav::getDirectionFlag(direction)));
			}
		}
		// NOTE: Corner collisions on orthogonal edge-occupying entities are impossible!
		else { // Entity is positioned on an edge and moving orthogonally.
			// Direct collision:
			oInfo = enav::getNextEntityInfoFromEdgeToDirectA(tile, position);
			collision |= tryAddCollisionDirectionComponent(entity, &oInfo);
			if (collision) {
				oInfo = enav::getNextEntityInfoFromEdgeToDirectB(tile, position);
				tryAddCollisionDirectionComponent(entity, &oInfo);

				entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(entities[oInfo.entityIndex].index, 0, 1));
			}
			else {
				// Shifted collisions:
				enav::OrthogonalEntityCollisionInfo oInfo2;
				oInfo = enav::getNextEntityInfoFromEdgeToOffsetA1(tile, position);
				collision |= tryAddCollisionDirectionComponent(entity, &oInfo);
				if (collision) {
					oInfo2 = enav::getNextEntityInfoFromEdgeToOffsetA2(tile, position);
					tryAddCollisionDirectionComponent(entity, &oInfo2);
					entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(entities[oInfo.entityIndex].index,
																			 oInfo.entityInfoIndex,
																			 oInfo2.entityInfoIndex));

					oInfo = enav::getNextEntityInfoFromEdgeToOffsetA3(tile, position);
					tryAddCollisionDirectionComponent(entity, &oInfo);
					oInfo2 = enav::getNextEntityInfoFromEdgeToOffsetA4(tile, position);
					tryAddCollisionDirectionComponent(entity, &oInfo2);
					entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(entities[oInfo.entityIndex].index,
																			 oInfo.entityInfoIndex,
																			 oInfo2.entityInfoIndex));
				}
				oInfo = enav::getNextEntityInfoFromEdgeToOffsetB1(tile, position);
				collision |= tryAddCollisionDirectionComponent(entity, &oInfo);
				if (collision) {
					oInfo2 = enav::getNextEntityInfoFromEdgeToOffsetB2(tile, position);
					tryAddCollisionDirectionComponent(entity, &oInfo2);
					entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(entities[oInfo.entityIndex].index,
																			 oInfo.entityInfoIndex,
																			 oInfo2.entityInfoIndex));

					oInfo = enav::getNextEntityInfoFromEdgeToOffsetB3(tile, position);
					tryAddCollisionDirectionComponent(entity, &oInfo);
					oInfo2 = enav::getNextEntityInfoFromEdgeToOffsetB4(tile, position);
					tryAddCollisionDirectionComponent(entity, &oInfo2);
					entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(entities[oInfo.entityIndex].index,
																			 oInfo.entityInfoIndex,
																			 oInfo2.entityInfoIndex));
				}
			}

			if (collision) {
				entityDirComponentsToRemove.push_back(EntityDirectionComponentRemoval(
					entity->index, 0, tnav::getDirectionFlag(entity->getDirection(0))));
				entityDirComponentsToRemove.push_back(EntityDirectionComponentRemoval(
					entity->index, 1, tnav::getDirectionFlag(entity->getDirection(1))));
			}
		}

		if (entityDirComponents[0] == entityDirComponents[1]) {
			// Entity direction is orthogonal and we do not need to check the second direction component!
			return; 
		}

		direction = entityDirComponents[1];
		collision = false;

		// Direction is diagonal:
		// Entities with a diagonal direction can only be in the center or on the edge of a tile.

		if (entity->getPosition(0) == LOCAL_POSITION_CENTER) {
			oInfo = enav::getNextEntityInfoFromCenterToDirect(tile, direction);
			collision |= tryAddCollisionDirectionComponent(entity, &oInfo);
			if (!collision) {
				dInfo = enav::getNextEntityInfoFromCenterToCornerA(tile, direction);
				collision |= tryAddCollisionDirectionComponent(entity, &dInfo);

				dInfo = enav::getNextEntityInfoFromCenterToCornerB(tile, direction);
				collision |= tryAddCollisionDirectionComponent(entity, &dInfo);
			}

			if (collision) {
				entityDirComponentsToRemove.push_back(EntityDirectionComponentRemoval(
					entity->index, 0, tnav::getDirectionFlag(direction)));
			}
		}
		else { // entity is positioned in a corner:
			const LocalDirection* directionComponents = tnav::getAlignmentComponents(entity->getDirection(0));

			// direct collisions:

			// direction component1:

			// direction component2:

			// offset collision 1 of direction component 1:
			// 
			// offset collision 1 of direction component 2:

			// offset collision 2 of direction component 1:

			// offset collision 2 of direction component 2:

			// corner collisions:
		}
	}

	void findInitialCollisions()
	{
		for (Entity& entity : entities) {
			if (entity.getDirection(0) == LOCAL_DIRECTION_STATIC) {
				continue;
			}
			findCollision(&entity);
		}
	}

	void disperseAndFindCollisions()
	{

	}

	void solveCollisions()
	{
		findInitialCollisions();
		updateEntityLocalDirectionComponents();
	}

	void update()
	{
		solveCollisions();
		updateGpuInfos();
		moveEntities();
	}

	void updateGpuInfos();
};