#pragma once
#include <iostream>
#include <bitset>

#include "tileManager.h"
#include "entity.h"
#include "tileNavigation.h"

struct EntityCollisionInfo {
	int tileIndex;
	LocalPosition position;
	LocalDirection direction;

	int collidingTileIndex;
	LocalPosition collidingPosition;
	LocalDirection collidingDirection;

	EntityCollisionInfo(int tileIndex, LocalPosition position) : tileIndex(tileIndex), position(position) {}
};

struct EntityLocalDirectionComponentRemoval {
	int entityIndex;
	int entityInfoIndex;
	uint8_t componentToRemove;
	EntityLocalDirectionComponentRemoval(int index, int infoIndex, uint8_t component) :
		entityIndex(index), entityInfoIndex(infoIndex), componentToRemove(component)
	{}
};
struct EntityLocalDirectionComponentAddition {
	int entityIndex;
	int entityInfoIndex;
	uint8_t componentToAdd;
	EntityLocalDirectionComponentAddition(int index, int infoIndex, uint8_t component) :
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
	std::vector<EntityLocalDirectionComponentRemoval> entityDirComponentsToRemove;
	std::vector<EntityLocalDirectionComponentAddition> entityDirComponentsToAdd;
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
		for (EntityLocalDirectionComponentRemoval sub : entityDirComponentsToRemove) {
			Entity* e = &entities[sub.entityIndex];
			e->removeDirectionFlag(sub.entityInfoIndex, sub.componentToRemove);
			e->setDirection(sub.entityInfoIndex, tnav::getLocalDirection(e->getDirectionFlag(sub.entityInfoIndex)));
		}
		for (EntityLocalDirectionComponentAddition add : entityDirComponentsToAdd) {
			Entity* e = &entities[add.entityIndex];
			e->addDirectionFlag(add.entityInfoIndex, add.componentToAdd);
			e->setDirection(add.entityInfoIndex, tnav::getLocalDirection(e->getDirectionFlag(add.entityInfoIndex)));
		}
		for (EntityAndTileInfoSwap s : entityAndTileInfosToSwap) {
			Entity* e = &entities[s.entityIndex];
			Tile* tile0 = p_tileManager->tiles[e->getTileIndex(s.entityInfoIndex0)];
			Tile* tile1 = p_tileManager->tiles[e->getTileIndex(s.entityInfoIndex1)];
			int pos0 = e->getPosition(s.entityInfoIndex0);
			int pos1 = e->getPosition(s.entityInfoIndex1);
			tile0->entityInfoIndices[pos0] = s.entityInfoIndex1;
			tile1->entityInfoIndices[pos1] = s.entityInfoIndex0;
			e->swapTileInfos(s.entityInfoIndex0, s.entityInfoIndex1);
		}
		entityDirComponentsToRemove.clear();
		entityDirComponentsToAdd.clear();
		entityAndTileInfosToSwap.clear();
	}

	void findInitialCollisions()
	{
		for (Entity& entity : entities) {
			if (entity.getDirection(0) == LOCAL_DIRECTION_STATIC) {
				continue;
			}

			Tile* currentTile = getTile(&entity, 0);

			if (tnav::isOrthogonal(entity.getDirection(0))) {
				// Entities with an orthogonal direction can only be in the center of a tile or in the middle of a tile edge.
				if (entity.getPosition(0) == LOCAL_POSITION_CENTER) {
					Tile* neighbor = currentTile->getNeighbor(entity.getDirection(0));
					LocalDirection directNonCollisionDirection = tnav::directionToDirectionMap(currentTile->type, neighbor->type, entity.getDirection(0), entity.getDirection(0));
					
					// direct collision:
					if (neighbor->hasEntity(LOCAL_POSITION_CENTER)) {
						LocalDirection nighborCollidingEntityDir = entities[neighbor->getEntityIndex(LOCAL_POSITION_CENTER)].getDirection(0);
						if (tnav::localDirectionHasComponent(nighborCollidingEntityDir, directNonCollisionDirection) == false) {
							// there is a collision!
							entityDirComponentsToRemove.push_back(EntityLocalDirectionComponentRemoval(
								entity.index, 0, tnav::getLocalDirectionFlag(entity.getDirection(0))));
							
							Entity* neighborEntity = &entities[neighbor->getEntityIndex(LOCAL_POSITION_CENTER)];
							entityDirComponentsToAdd.push_back(EntityLocalDirectionComponentAddition(
								neighborEntity->index, 0, tnav::getLocalDirectionFlag(directNonCollisionDirection)));
							
							continue;
						}
					}
					// shifted collisions are impossible!
					
					// corner collisions:
					LocalDirection toCornerTile = LocalDirection((entity.getDirection(0) + 1) % 4);
					toCornerTile = tnav::directionToDirectionMap(currentTile->type, neighbor->type, entity.getDirection(0), toCornerTile);
					Tile* neighborNeighbor = neighbor->getNeighbor(toCornerTile);
					LocalDirection cornerTileOpposingDir = tnav::directionToDirectionMap(neighbor->type, neighborNeighbor->type, toCornerTile, LocalDirection((toCornerTile + 2) % 4));
					if (neighborNeighbor->hasEntity(LOCAL_POSITION_CENTER)) {
						LocalDirection nighborNeighborCollidingEntityDir = entities[neighborNeighbor->getEntityIndex(LOCAL_POSITION_CENTER)].getDirection(0);
						if (tnav::localDirectionHasComponent(nighborNeighborCollidingEntityDir, cornerTileOpposingDir)) {
							// there is a collision!
							entityDirComponentsToRemove.push_back(EntityLocalDirectionComponentRemoval(
								entity.index, 0, tnav::getLocalDirectionFlag(entity.getDirection(0))));

							Entity* neighborEntity = &entities[neighborNeighbor->getEntityIndex(LOCAL_POSITION_CENTER)];
							LocalDirection dirToAddToCornerEntity = tnav::directionToDirectionMap(currentTile->type, neighbor->type, entity.getDirection(0), entity.getDirection(0));
							dirToAddToCornerEntity = tnav::directionToDirectionMap(neighbor->type, neighborNeighbor->type, toCornerTile, dirToAddToCornerEntity);
							entityDirComponentsToAdd.push_back(EntityLocalDirectionComponentAddition(
								neighborEntity->index, 0, tnav::getLocalDirectionFlag(dirToAddToCornerEntity)));
						}
					}
					// other possible corner collision:
					toCornerTile = LocalDirection((entity.getDirection(0) + 3) % 4);
					toCornerTile = tnav::directionToDirectionMap(currentTile->type, neighbor->type, entity.getDirection(0), toCornerTile);
					neighborNeighbor = neighbor->getNeighbor(toCornerTile);
					cornerTileOpposingDir = tnav::directionToDirectionMap(neighbor->type, neighborNeighbor->type, toCornerTile, LocalDirection((toCornerTile + 2) % 4));
					if (neighborNeighbor->hasEntity(LOCAL_POSITION_CENTER)) {
						LocalDirection nighborNeighborCollidingEntityDir = entities[neighborNeighbor->getEntityIndex(LOCAL_POSITION_CENTER)].getDirection(0);
						if (tnav::localDirectionHasComponent(nighborNeighborCollidingEntityDir, cornerTileOpposingDir)) {
							// there is a collision!
							entityDirComponentsToRemove.push_back(EntityLocalDirectionComponentRemoval(
								entity.index, 0, tnav::getLocalDirectionFlag(entity.getDirection(0))));

							Entity* neighborEntity = &entities[neighborNeighbor->getEntityIndex(LOCAL_POSITION_CENTER)];
							LocalDirection dirToAddToCornerEntity = tnav::directionToDirectionMap(currentTile->type, neighbor->type, entity.getDirection(0), entity.getDirection(0));
							dirToAddToCornerEntity = tnav::directionToDirectionMap(neighbor->type, neighborNeighbor->type, toCornerTile, dirToAddToCornerEntity);
							entityDirComponentsToAdd.push_back(EntityLocalDirectionComponentAddition(
								neighborEntity->index, 0, tnav::getLocalDirectionFlag(dirToAddToCornerEntity)));
						}
					}
				}
				else { // entity is positioned on an edge.
					// Direct collision:
					if (currentTile->entityIndices[entity.getDirection(0)] != -1) {
						// There is a colliding entity as it is impossible for a static entitiy to be on an edge!
						Entity* collidingEntity = &entities[currentTile->entityIndices[entity.getDirection(0)]];
						Tile* neighborTile = currentTile->getNeighbor(entity.getDirection(0));
						LocalDirection translatedDir = tnav::directionToDirectionMap(currentTile->type, neighborTile->type, entity.getDirection(0), entity.getDirection(0));
						
						entityDirComponentsToRemove.push_back(EntityLocalDirectionComponentRemoval(
							entity.index, 0, tnav::getLocalDirectionFlag(entity.getDirection(0))));
						entityDirComponentsToRemove.push_back(EntityLocalDirectionComponentRemoval(
							entity.index, 1, tnav::getLocalDirectionFlag(entity.getDirection(1))));

						// because the colliding entity is on the edge of a tile, it has 2 tile infos, so we need
						// to add the direction change info to the other info as well:
						entityDirComponentsToAdd.push_back(EntityLocalDirectionComponentAddition(
							collidingEntity->index, 0, tnav::getLocalDirectionFlag(entity.getDirection(0))));
						entityDirComponentsToAdd.push_back(EntityLocalDirectionComponentAddition(
							collidingEntity->index, 1, tnav::getLocalDirectionFlag(translatedDir)));
						
						// Because the algorithms expect entity tile info to be in order of arrival (arriving tile is 
						// first, then leaving tile, then adjacent tiles), we have to reorder the infos here since the 
						// direction is changing!
						entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(collidingEntity->index, 0, 1));
						
						continue;
					}
					// shifted collision 1:
					// maps local EDGE position to local CORNER position as we know that the direciton is 'pointing to'
					// an edge!
					LocalPosition otherPositionComponent = LocalPosition((entity.getDirection(0) + 1) % 4);
					LocalPosition cornerPosition = tnav::combineLocalDirections(entity.getDirection(0), otherPositionComponent);
					LocalDirection directionOfNonCollision = entity.getDirection(0);
					if (currentTile->entityIndices[cornerPosition] != -1) { // unsafe corners are auto-skipped due to no entities being able to 'get' to them.
						
						int collidingEntityIndex = currentTile->entityIndices[cornerPosition];
						int collidingEntityInfoIndex = currentTile->entityInfoIndices[cornerPosition];
						Entity* collidingEntity = &entities[collidingEntityIndex];

						if (tnav::localDirectionHasComponent(collidingEntity->getDirection(collidingEntityInfoIndex), directionOfNonCollision) == false) {
							// There is a collision, and the collision MUST be with a diagonally moving entity due to restrictions on possible entity setups!
							
							entityDirComponentsToRemove.push_back(EntityLocalDirectionComponentRemoval(
								entity.index, 0, tnav::getLocalDirectionFlag(entity.getDirection(0))));
							entityDirComponentsToRemove.push_back(EntityLocalDirectionComponentRemoval(
								entity.index, 1, tnav::getLocalDirectionFlag(entity.getDirection(1))));


							entityDirComponentsToAdd.push_back(EntityLocalDirectionComponentAddition(
								collidingEntity->index, collidingEntityInfoIndex, tnav::getLocalDirectionFlag(directionOfNonCollision)));
							
							Tile* neighbor = currentTile->getNeighbor(directionOfNonCollision);
							LocalDirection neighborDirOfNonCollision = tnav::directionToDirectionMap(currentTile->type, neighbor->type, directionOfNonCollision, directionOfNonCollision);
							LocalPosition neighborEntityPos = tnav::combineLocalDirections(entity.getPosition(0), otherPositionComponent);
							neighborEntityPos = tnav::positionToPositionMap(currentTile->type, neighbor->type, directionOfNonCollision, neighborEntityPos);
							int neighborEntityInfoIndex = neighbor->entityInfoIndices[neighborEntityPos];
							entityDirComponentsToAdd.push_back(EntityLocalDirectionComponentAddition(
								collidingEntity->index, neighborEntityInfoIndex, tnav::getLocalDirectionFlag(neighborDirOfNonCollision)));
							
							entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(collidingEntity->index, collidingEntityInfoIndex, neighborEntityInfoIndex));
							
							neighbor = currentTile->getNeighbor(otherPositionComponent);
							neighborDirOfNonCollision = tnav::directionToDirectionMap(currentTile->type, neighbor->type, otherPositionComponent, directionOfNonCollision);
							neighborEntityPos = tnav::combineLocalDirections(entity.getDirection(0), tnav::oppositeDirection(otherPositionComponent));
							neighborEntityPos = tnav::positionToPositionMap(currentTile->type, neighbor->type, otherPositionComponent, neighborEntityPos);
							neighborEntityInfoIndex = neighbor->entityInfoIndices[neighborEntityPos];
							entityDirComponentsToAdd.push_back(EntityLocalDirectionComponentAddition(
								collidingEntity->index, neighborEntityInfoIndex, tnav::getLocalDirectionFlag(neighborDirOfNonCollision)));

							Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirOfNonCollision);
							LocalDirection neighborNeighborDirOfNonCollision = tnav::directionToDirectionMap(neighbor->type, neighborNeighbor->type, neighborDirOfNonCollision, neighborDirOfNonCollision);
							LocalPosition neighborNeighborEntityPos = tnav::combineLocalDirections(entity.getPosition(0), tnav::oppositeDirection(otherPositionComponent));
							neighborNeighborEntityPos = tnav::positionToPositionMap(currentTile->type, neighbor->type, otherPositionComponent, neighborEntityPos);
							neighborNeighborEntityPos = tnav::positionToPositionMap(neighbor->type, neighborNeighbor->type, neighborNeighborDirOfNonCollision, neighborEntityPos);
							int neighborNeighborEntityInfoIndex = neighborNeighbor->entityInfoIndices[neighborNeighborEntityPos];
							entityDirComponentsToAdd.push_back(EntityLocalDirectionComponentAddition(
								collidingEntity->index, neighborNeighborEntityInfoIndex, tnav::getLocalDirectionFlag(neighborNeighborDirOfNonCollision)));
						
							entityAndTileInfosToSwap.push_back(EntityAndTileInfoSwap(collidingEntity->index, neighborEntityInfoIndex, neighborNeighborEntityInfoIndex));
						}
					}
					// shifted collision 2:
					LocalPosition cornerPosition2 = LocalPosition((entity.getDirection(0) + 3) % 4 + 4);
					
					// corner collisions on orthogonal offset entities are impossible!
				}
			}
			else { // Direction is diagonal:
				// Entities with a diagonal direction can only be in the center or on the edge of a tile.
				const LocalDirection* directionComponents = tnav::localDirectionComponents(entity.getDirection(0));

				if (entity.getPosition(0) == LOCAL_POSITION_CENTER) {

				}
				else { // entity is positioned in a corner:
					const LocalDirection* directionComponents = tnav::localDirectionComponents(entity.getDirection(0));

					// direct collisions:
					// direction component1:

					// direction component2:
					
					// offset collision 1 of direction component 1:
					LocalPosition collisionPosition = directionComponents[0];
					LocalDirection directionOfNonCollision = directionComponents[0];
					if (currentTile->entityIndices[collisionPosition] != -1) {
						int collidingEntityIndex = currentTile->entityIndices[collisionPosition];
						int collidingEntityInfoIndex = currentTile->entityInfoIndices[collisionPosition];
						Entity* collidingEntity = &entities[collidingEntityIndex];
						if (collidingEntity->getDirection(collidingEntityInfoIndex) != directionOfNonCollision) {
							// There is a collision!

						}
					}
					// offset collision 1 of direction component 2:
					LocalDirection toNeighborDir = tnav::oppositeDirection(directionComponents[1]);
					Tile* neighbor = currentTile->getNeighbor(toNeighborDir);
					collisionPosition = tnav::positionToPositionMap(currentTile->type, neighbor->type, toNeighborDir, collisionPosition);
					LocalDirection neighborDirectionOfNonCollision = tnav::directionToDirectionMap(currentTile->type, neighbor->type, toNeighborDir, directionOfNonCollision);
					if (neighbor->entityIndices[collisionPosition] != -1) {
						int collidingEntityIndex = neighbor->entityIndices[collisionPosition];
						int collidingEntityInfoIndex = neighbor->entityInfoIndices[collisionPosition];
						Entity* collidingEntity = &entities[collidingEntityIndex];
						// Colliding entity MUST have orthogonal direction, as it is on an edge!
						if (collidingEntity->getDirection(collidingEntityInfoIndex) != neighborDirectionOfNonCollision) {
							// There is a collision!

							// remove forces from entity:
							entityDirComponentsToRemove.push_back(EntityLocalDirectionComponentRemoval(
								entity.index, 0, tnav::getLocalDirectionFlag(neighborDirectionOfNonCollision)));
							
							// because the entity is in a corner, we have to find the other directions of non-collision:
							LocalPosition neighborEntityPos = tnav::combineLocalDirections(directionComponents[1], tnav::oppositeDirection(directionComponents[0]));
							neighborEntityPos = tnav::positionToPositionMap(currentTile->type, neighbor->type, toNeighborDir, neighborEntityPos);
							int neighborEntityInfoIndex = neighbor->entityInfoIndices[neighborEntityPos];
							entityDirComponentsToRemove.push_back(EntityLocalDirectionComponentRemoval(
								entity.index, neighborEntityInfoIndex, tnav::getLocalDirectionFlag(neighborDirectionOfNonCollision)));

							LocalDirection toOtherNeighborDir = tnav::oppositeDirection(directionComponents[1]);
							Tile* otherNeighbor = currentTile->getNeighbor(toOtherNeighborDir);
							LocalPosition otherNeighborEntityPos = tnav::combineLocalDirections(directionComponents[0], toOtherNeighborDir);
							otherNeighborEntityPos = tnav::positionToPositionMap(currentTile->type, otherNeighbor->type, toOtherNeighborDir, otherNeighborEntityPos);
							int otherNeighborEntityInfoIndex = otherNeighbor->entityInfoIndices[otherNeighborEntityPos];
							LocalDirection otherNeighborEntityDirComp0 = tnav::directionToDirectionMap(currentTile->type, otherNeighbor->type, toOtherNeighborDir, directionComponents[0]);
							entityDirComponentsToRemove.push_back(EntityLocalDirectionComponentRemoval(
								entity.index, neighborEntityInfoIndex, tnav::getLocalDirectionFlag(otherNeighborEntityDirComp0)));

							LocalDirection toOtherNeighborDir = tnav::oppositeDirection(directionComponents[1]);
							Tile* otherNeighbor = currentTile->getNeighbor(toOtherNeighborDir);
							LocalPosition otherNeighborEntityPos = tnav::combineLocalDirections(directionComponents[0], toOtherNeighborDir);
							otherNeighborEntityPos = tnav::positionToPositionMap(currentTile->type, otherNeighbor->type, toOtherNeighborDir, otherNeighborEntityPos);
							int otherNeighborEntityInfoIndex = otherNeighbor->entityInfoIndices[otherNeighborEntityPos];
							LocalDirection otherNeighborEntityDirComp0 = tnav::directionToDirectionMap(currentTile->type, otherNeighbor->type, toOtherNeighborDir, directionComponents[0]);
							entityDirComponentsToRemove.push_back(EntityLocalDirectionComponentRemoval(
								entity.index, neighborEntityInfoIndex, tnav::getLocalDirectionFlag(otherNeighborEntityDirComp0)));
						}
					}


					// offset collision 2 of direction component 1:
					LocalDirection collisionPosition = tnav::oppositeDirection(directionComponents[1]);

					// offset collision 2 of direction component 2:

					// corner collisions:
				}
			}
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