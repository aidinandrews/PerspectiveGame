#pragma once
#include <iostream>

#include "entity.h"
#include "tileNodeNetwork.h"
#include "collisionSolver.h"

struct EntityManager
{
	std::vector<Entity> entities;
	TileNodeNetwork* p_nodeNetwork;

	ForceManager* p_forceManager;
	std::vector<OrthCollisionSolver> orthSolvers;
	std::vector<DiagCollisionSolver> diagSolvers;
	std::vector<PeekCollisionSolver> peekSolvers;
	std::vector<TriACollisionSolver> triASolvers;
	std::vector<TriBCollisionSolver> triBSolvers;
	std::vector<QuadCollisionSolver> quadSolvers;

	EntityManager(TileNodeNetwork* tnn,
				  ForceManager* fm)
		: p_nodeNetwork(tnn)
		, p_forceManager(fm)
	{

	}

	void update()
	{
	}

	void updateAllGpuTiles()
	{
		for (Entity& e : entities) {
			updateGpuTiles(e);
		}
	}

	void moveEntities()
	{
		for (Entity& e : entities) {
			moveEntity(e);
			//p_forceManager->setForce(e.forceListIndex, LOCAL_DIRECTION_STATIC);
		}
	}

	bool createEntity(CenterNode* node, LocalDirection entityDir)
	{
		if (node->hasEntity) return false;

		entities.push_back(Entity(Entity::Type::ENTITY_TYPE_DEFAULT,
								  glm::vec3(0.5, 0.5, 0.5),
								  node, (int)p_forceManager->forceList.size()));
		p_forceManager->addForce(entityDir, node->index);

		return true;
	}

	void moveEntity(Entity& e)
	{
		LocalDirection d = p_forceManager->getForce(e.forceListIndex);
		if (d == LOCAL_DIRECTION_STATIC) return;
		p_forceManager->setForce(e.forceListIndex, tnav::map(e.node->getNeighborMap(d), d));
		e.node = p_nodeNetwork->getNeighbor(e.node, d);
	}

	void updateGpuTiles(Entity& e)
	{
		SideNode* sideNode;
		int ti;
		LocalDirection toTile;
		MapType m;
		LocalPosition p;


		std::vector<GPU_Tile>* gpuTiles = &(p_nodeNetwork->gpuTiles);
		switch (e.node->type) {
		case NODE_TYPE_CENTER:
			(*gpuTiles)[static_cast<CenterNode*>(e.node)->getTileIndex()].addEntity(
				LOCAL_POSITION_CENTER, p_forceManager->getForce(e.forceListIndex));
			return;
		case NODE_TYPE_SIDE:
			sideNode = static_cast<SideNode*>(e.node);
			ti = static_cast<CenterNode*>(p_nodeNetwork->getNode(sideNode->getNeighborIndexDirect(0)))->getTileIndex();
			toTile = sideNode->getLocalDirDirect(0);
			m = sideNode->getNeighborMapDirect(0);
			p = tnav::map(m, tnav::inverse(toTile));
			(*gpuTiles)[ti].addEntity(p, tnav::map(m, p_forceManager->getForce(e.forceListIndex)));

			ti = static_cast<CenterNode*>(p_nodeNetwork->getNode(sideNode->getNeighborIndexDirect(1)))->getTileIndex();
			toTile = sideNode->getLocalDirDirect(1);
			m = sideNode->getNeighborMapDirect(1);
			p = tnav::map(m, tnav::inverse(toTile));
			(*gpuTiles)[ti].addEntity(p, tnav::map(m, p_forceManager->getForce(e.forceListIndex)));
			return;
		case NODE_TYPE_CORNER:

			return;
		default: return;
		}
	}
};