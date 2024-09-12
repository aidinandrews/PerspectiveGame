#pragma once
#include <iostream>
#include <bitset>

#include "tileNavigation.h"
#include "tileManager.h"
#include "entity.h"
#include "entityNavigation.h"

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
	EntityDirectionComponentRemoval(int entityIndex, int entityInfoIndex, uint8_t component) :
		entityIndex(entityIndex), entityInfoIndex(entityInfoIndex), componentToRemove(component)
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
	{

	}
	EntityAndTileInfoSwap(enav::OrthogonalEntityCollisionInfo &oInfo1, enav::OrthogonalEntityCollisionInfo &oInfo2) :
		entityIndex(oInfo1.entityIndex), entityInfoIndex0(oInfo1.entityInfoIndex), entityInfoIndex1(oInfo2.entityInfoIndex)
	{
	
	}
	EntityAndTileInfoSwap(enav::DiagonalEntityCollisionInfo& dInfo1, enav::DiagonalEntityCollisionInfo& dInfo2) :
		entityIndex(dInfo1.entityIndex), entityInfoIndex0(dInfo1.entityInfoIndex), entityInfoIndex1(dInfo2.entityInfoIndex)
	{
	
	}
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

	bool tryAddDirectionComponent(Entity* entity, enav::OrthogonalEntityCollisionInfo* info)
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

	bool tryAddDirectionComponent(Entity* entity, enav::DiagonalEntityCollisionInfo* info)
	{
		if (info->entityIndex == -1) {
			return false;
		}

		Entity* collidingEntity = &entities[info->entityIndex];
		LocalDirection collidingEntityDir = collidingEntity->getDirection(info->entityInfoIndex);
		bool collision = tnav::alignmentHasComponent(collidingEntityDir, info->cornerDirectionOfCollision);
		
		if (collidingEntity->isStatic() &&
			tnav::alignmentHasComponent(entity->getDirection(0), tnav::oppositeAlignment(info->cornerDirectionOfCollision))) {
			collision = true;
		}

		if (collision) {
			entityDirComponentsToAdd.push_back(EntityDirectionComponentAddition(
				collidingEntity->index, info->entityInfoIndex, tnav::getDirectionFlag(info->equivelantDirection)));
			return true;
		}
		else {
			return false;
		}
	}

	Entity* getEntityIndex(enav::OrthogonalEntityCollisionInfo oInfo)
	{
		return &entities[oInfo.entityIndex];
	}

	void findCollision(Entity* entity)
	{
		const LocalDirection* entityDirComponents = tnav::getAlignmentComponents(entity->getDirection(0));
		LocalDirection direction = entityDirComponents[0];
		LocalPosition position = entity->getPosition(0);
		Tile* tile = getTile(entity, 0);
		enav::OrthogonalEntityCollisionInfo oInfo0;
		enav::OrthogonalEntityCollisionInfo oInfo1;
		enav::DiagonalEntityCollisionInfo dInfo0;
		enav::DiagonalEntityCollisionInfo dInfo1;
		bool collision = false;

		// NOTE: Shifted collisions from the center of a tile are impossible!
		if (entity->getPosition(0) == LOCAL_POSITION_CENTER) {
			// Direct collision:
			oInfo0 = enav::getEntityInfoCenterToDirect(tile, direction);
			collision |= tryAddDirectionComponent(entity, &oInfo0);
			if (!collision) {
				// Corner collisions:
				dInfo0 = enav::getEntityInfoCenterToCornerA(tile, direction);
				if (dInfo0.cornerDirectionOfCollision != entityDirComponents[1]) {
					collision |= tryAddDirectionComponent(entity, &dInfo0);
				}
				dInfo0 = enav::getEntityInfoCenterToCornerB(tile, direction);
				if (dInfo0.cornerDirectionOfCollision != entityDirComponents[1]) {
					collision |= tryAddDirectionComponent(entity, &dInfo0);
				}
			}
			if (collision) {
				entityDirComponentsToRemove.push_back(EntityDirectionComponentRemoval(
					entity->index, 0, tnav::getDirectionFlag(direction)));
			}
		}
		// NOTE: Corner collisions on orthogonal edge-occupying entities are impossible!
		else if(entity->isInTileCorner() == false) { // Entity is positioned on an edge and moving orthogonally.
			// Direct collision:
			oInfo0 = enav::getEntityInfoEdgeToDirectA(tile, position);
			if (tryAddDirectionComponent(entity, &oInfo0)) {
				oInfo0 = enav::getEntityInfoEdgeToDirectB(tile, position);
				tryAddDirectionComponent(entity, &oInfo0);

				entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(oInfo0.entityIndex, 0, 1));
				collision = true;
			}
			else {
				// Offset collisions:
				oInfo0 = enav::getEntityInfoEdgeToOffsetA1(tile, position);
				if (tryAddDirectionComponent(entity, &oInfo0)) {
					oInfo1 = enav::getEntityInfoEdgeToOffsetA2(tile, position);
					tryAddDirectionComponent(entity, &oInfo1);
					entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(oInfo0, oInfo1));

					oInfo0 = enav::getEntityInfoEdgeToOffsetA3(tile, position);
					tryAddDirectionComponent(entity, &oInfo0);
					oInfo1 = enav::getEntityInfoEdgeToOffsetA4(tile, position);
					tryAddDirectionComponent(entity, &oInfo1);
					entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(oInfo0, oInfo1));
					collision = true;
				}
				oInfo0 = enav::getEntityInfoEdgeToOffsetB1(tile, position);
				if (tryAddDirectionComponent(entity, &oInfo0)) {
					oInfo1 = enav::getEntityInfoEdgeToOffsetB2(tile, position);
					tryAddDirectionComponent(entity, &oInfo1);
					entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(oInfo0, oInfo1));

					oInfo0 = enav::getEntityInfoEdgeToOffsetB3(tile, position);
					tryAddDirectionComponent(entity, &oInfo0);
					oInfo1 = enav::getEntityInfoEdgeToOffsetB4(tile, position);
					tryAddDirectionComponent(entity, &oInfo1);
					entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(oInfo0, oInfo1));
					collision = true;
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

		collision = false;

		// Direction is diagonal:
		// Entities with a diagonal direction can only be in the center or on the corner of a tile.
		// Half of this collision chack has already happend via the orthogonal direct collision check above.
		// This check only needs to take the second direction component into account.
		if (entity->getPosition(0) == LOCAL_POSITION_CENTER) {
			// Unsafe corner will reverse entity direction:
			if (tile->cornerSafety[(entity->getDirection(0) + 1) % 4] == Tile::CORNER_UNSAFE) {
				// reverse dir and get outta there:
				entity->setDirection(0, tnav::oppositeAlignment(entity->getDirection(0)));
			}

			oInfo0 = enav::getEntityInfoCenterToDirect(tile, entityDirComponents[1]);
			collision |= tryAddDirectionComponent(entity, &oInfo0);
			// Corner collisions:
			dInfo0 = enav::getEntityInfoCenterToCornerA(tile, entityDirComponents[1]);
			if (dInfo0.cornerDirectionOfCollision != entityDirComponents[0]) {
				collision |= tryAddDirectionComponent(entity, &dInfo0);
			}
			dInfo0 = enav::getEntityInfoCenterToCornerB(tile, entityDirComponents[1]);
			if (dInfo0.cornerDirectionOfCollision != entityDirComponents[0]) {
				collision |= tryAddDirectionComponent(entity, &dInfo0);
			}

			if (collision) {
				entityDirComponentsToRemove.push_back(EntityDirectionComponentRemoval(
					entity->index, 0, tnav::getDirectionFlag(entityDirComponents[1])));
			}
		}
		else { // entity is positioned in a corner:
			// We have to check both components here as the other checks assume things that make checking before uneccessary.
			for (int i = 0; i < 2; i++) {
				collision = false;
				direction = entityDirComponents[i];

				// direct collisions:
				oInfo0 = enav::getEntityInfoCornerToDirectA(tile, position, direction);
				if (tryAddDirectionComponent(entity, &oInfo0)) {
					oInfo1 = enav::getEntityInfoCornerToDirectB(tile, position, direction);
					tryAddDirectionComponent(entity, &oInfo1);
					entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(oInfo0, oInfo1));

					oInfo0 = enav::getEntityInfoCornerToDirectC(tile, position, direction);
					tryAddDirectionComponent(entity, &oInfo0);
					oInfo1 = enav::getEntityInfoCornerToDirectD(tile, position, direction);
					tryAddDirectionComponent(entity, &oInfo1);
					entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(oInfo0, oInfo1));
					
					collision = true;
				}
				else {
					// offset collisions:
					oInfo0 = enav::getEntityInfoCornerToOffsetA1(tile, position, direction);
					if (tryAddDirectionComponent(entity, &oInfo0)) {
						oInfo0 = enav::getEntityInfoCornerToOffsetA2(tile, position, direction);
						tryAddDirectionComponent(entity, &oInfo0);
						entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(oInfo0.entityIndex, 0, 1));
						collision = true;
					}
					oInfo0 = enav::getEntityInfoCornerToOffsetB1(tile, position, direction);
					if (tryAddDirectionComponent(entity, &oInfo0)) {
						oInfo0 = enav::getEntityInfoCornerToOffsetB2(tile, position, direction);
						tryAddDirectionComponent(entity, &oInfo0);
						entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(oInfo0.entityIndex, 0, 1));
						collision = true;
					}

					// corner collisions:
					dInfo0 = enav::getEntityInfoCornerToCornerA1(tile, position, direction);
					if (tryAddDirectionComponent(entity, &dInfo0)) {
						dInfo1 = enav::getEntityInfoCornerToCornerA2(tile, position, direction);
						tryAddDirectionComponent(entity, &dInfo1);
						entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(dInfo0, dInfo1));

						dInfo0 = enav::getEntityInfoCornerToCornerA3(tile, position, direction);
						tryAddDirectionComponent(entity, &dInfo0);
						dInfo1 = enav::getEntityInfoCornerToCornerA4(tile, position, direction);
						tryAddDirectionComponent(entity, &dInfo1);
						entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(dInfo0, dInfo1));

						collision = true;
					}
					dInfo0 = enav::getEntityInfoCornerToCornerB1(tile, position, direction);
					if (tryAddDirectionComponent(entity, &dInfo0)) {
						dInfo1 = enav::getEntityInfoCornerToCornerB2(tile, position, direction);
						tryAddDirectionComponent(entity, &dInfo1);
						entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(dInfo0, dInfo1));

						dInfo0 = enav::getEntityInfoCornerToCornerB3(tile, position, direction);
						tryAddDirectionComponent(entity, &dInfo0);
						dInfo1 = enav::getEntityInfoCornerToCornerB4(tile, position, direction);
						tryAddDirectionComponent(entity, &dInfo1);
						entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(dInfo0, dInfo1));

						collision = true;
					}
				}

				if (collision) {
					LocalDirection equivDirs[3];
					equivDirs[0] = tile->mapAlignmentTo1stDegreeNeighbor(tnav::oppositeAlignment(entityDirComponents[0]), entityDirComponents[i]);
					equivDirs[1] = tile->mapAlignmentTo1stDegreeNeighbor(tnav::oppositeAlignment(entityDirComponents[1]), entityDirComponents[i]);
					Tile* neighbor;
					if (i == 0) {
						neighbor = tile->getNeighbor(tnav::oppositeAlignment(entityDirComponents[1]));
						equivDirs[2] = neighbor->mapAlignmentTo1stDegreeNeighbor(tnav::oppositeAlignment(equivDirs[1]), equivDirs[1]);
					}
					else {
						neighbor = tile->getNeighbor(tnav::oppositeAlignment(entityDirComponents[0]));
						equivDirs[2] = neighbor->mapAlignmentTo1stDegreeNeighbor(tnav::oppositeAlignment(equivDirs[0]), equivDirs[0]);
					}
					
					// Because the entity is in a corner, we need to remove direction information from 4 places, 1 for each inhabited tile.
					entityDirComponentsToRemove.push_back(EntityDirectionComponentRemoval(entity->index, 0, tnav::getDirectionFlag(entityDirComponents[i])));
					entityDirComponentsToRemove.push_back(EntityDirectionComponentRemoval(entity->index, 1, tnav::getDirectionFlag(equivDirs[2])));
					
					if (neighbor->index == entity->getTileIndex(2)) {
						entityDirComponentsToRemove.push_back(EntityDirectionComponentRemoval(entity->index, 2, tnav::getDirectionFlag(equivDirs[1])));
						entityDirComponentsToRemove.push_back(EntityDirectionComponentRemoval(entity->index, 3, tnav::getDirectionFlag(equivDirs[0])));
					}
					else {
						entityDirComponentsToRemove.push_back(EntityDirectionComponentRemoval(entity->index, 2, tnav::getDirectionFlag(equivDirs[0])));
						entityDirComponentsToRemove.push_back(EntityDirectionComponentRemoval(entity->index, 3, tnav::getDirectionFlag(equivDirs[1])));
					}
				}
			}
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