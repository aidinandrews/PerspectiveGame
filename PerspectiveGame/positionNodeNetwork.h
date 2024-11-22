#pragma once

#include <iostream>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "tileNavigation.h"
#include "positionNode.h"

struct PositionNodeNetwork {
private:
	std::vector<PositionNode> nodes;
	std::vector<int> freeNodeIndices;

	std::vector<TileInfo> tileInfos;
	std::vector<int> freeTileInfoIndices;

public: // Rendering:
	GLuint texID;
	GLuint positionNodeInfosBufferID;
	GLuint tileInfosBufferID;
	std::vector<glm::vec2> windowFrustum;

	std::vector<GPU_TileInfoNode> gpuTileInfos;
	std::vector<GPU_PositionNodeInfo> gpuPositionNodeInfos;

	PositionNode CurrentNode;
	int currentMapIndex = 0;
	int currentNodeIndex = 4;

public:

	PositionNodeNetwork()
	{
		// Initially, there must be at least one tile pair:
		/* east node:  */ nodes.push_back(PositionNode(0, -1, MAP_ID_0, glm::vec3(0.5, 0.0, 0.0), 5, 6, -1, -1, 4, 0, -1, -1, 1, 2, 1, 0, 3, 0, 3, 2));
		/* south node: */ nodes.push_back(PositionNode(1, -1, MAP_ID_0, glm::vec3(0.0, -0.5, 0.0), -1, -1, 5, 4, -1, -1, 4, 0, 0, 2, 2, 2, 2, 0, 0, 0));
		/* west node:  */ nodes.push_back(PositionNode(2, -1, MAP_ID_0, glm::vec3(-0.5, 0.0, 0.0), 4, 0, -1, -1, 5, 6, -1, -1, 1, 0, 1, 2, 5, 6, 3, 0));
		/* north node: */ nodes.push_back(PositionNode(3, -1, MAP_ID_0, glm::vec3(0.0, 0.5, 0.0), -1, -1, 4, 0, -1, -1, 5, 4, 0, 0, 2, 0, 2, 2, 0, 2));
		/* fore tile:  */ nodes.push_back(PositionNode(4, 0, MAP_ID_0, glm::vec3(0.0, 0.0, 0.0), 0, 0, 1, 0, 2, 0, 3, 0, -1, -1, -1, -1, -1, -1, -1, -1));
		/* back tile:  */ nodes.push_back(PositionNode(5, 1, MAP_ID_1, glm::vec3(0.0, 0.0, 0.0), 0, 6, 1, 4, 2, 6, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1));

		tileInfos.push_back(TileInfo(TILE_TYPE_XYF, 0, 4, glm::vec3(0, 0, 1)));
		tileInfos.push_back(TileInfo(TILE_TYPE_XYB, 1, 5, glm::vec3(0, 0, 0.5)));

		addTile(glm::vec3(0, 0, 0), TILE_TYPE_XY);

		// Rendering:
		glGenBuffers(1, &positionNodeInfosBufferID);
		glGenBuffers(1, &tileInfosBufferID);
	}

	void update()
	{
		gpuPositionNodeInfos.clear();
		for (PositionNode& p : nodes) {
			gpuPositionNodeInfos.push_back(GPU_PositionNodeInfo(p));
		}

		gpuTileInfos.clear();
		for (TileInfo& i : tileInfos) {
			gpuTileInfos.push_back(GPU_TileInfoNode(i));
		}
	}

	PositionNode* getNode(int index)
	{
		return &nodes[index];
	}

	TileInfo* getTileInfo(int index)
	{
		return &tileInfos[index];
	}

	// Gives the tile info of the neighbor tile in the direction of d.
	TileInfo* getTileInfo(TileInfo* info, LocalDirection d)
	{
		PositionNode* node = &nodes[info->nodeIndex];
		LocalDirection d2 = tnav::getMappedAlignment(node->getNeighborMapIndex(d), d);
		PositionNode sideNode = nodes[node->getNeighborIndex(d)];
		PositionNode neighborCenterNode = nodes[sideNode.getNeighborIndex(d2)];
		return &tileInfos[neighborCenterNode.getTileInfoIndex()];
	}

	PositionNode* getNeighbor(PositionNode* node, LocalDirection toNeighbor)
	{
		return &nodes[node->getNeighborIndex(toNeighbor)];
	}

	// Will add a node to the nodes list that is unconnected and error prone if it is not connected up!
	// returns an index to the added node.
	int addNode()
	{
		PositionNode* node;
		if (freeNodeIndices.size() > 0) {
			node = &nodes[freeNodeIndices.back()];
			node->setIndex(freeNodeIndices.back());
			freeNodeIndices.pop_back();
		}
		else {
			nodes.push_back(PositionNode());
			node = &nodes[nodes.size() - 1];
			node->setIndex(nodes.size() - 1);
		}
		return node->getIndex();
	}

	// Adds an UNINITIALIZED tile info to the tileInfos list.  Returns a pointer to the added tileInfo.
	int addTileInfo()
	{
		TileInfo* info;
		if (freeTileInfoIndices.size() > 0) {
			info = &tileInfos[freeTileInfoIndices.back()];
			info->index = freeTileInfoIndices.back();
			freeTileInfoIndices.pop_back();
		}
		else {
			tileInfos.push_back(TileInfo());
			info = &tileInfos[tileInfos.size() - 1];
			info->index = tileInfos.size() - 1;
		}
		return info->index;
	}

	void addSideNodeToTile(TileInfo* tile, LocalDirection toSide)
	{
	}

	// given a position in space, returns all the tiles connected to that point in the network.
	std::vector<TileInfo> getConnectedTiles(glm::vec3 pos, int nodeIndex)
	{
		std::vector<TileInfo> connectedTiles;

		for (PositionNode n : nodes) {
			if (n.getPosition() != pos || n.getIndex() == nodeIndex)
				continue;

			for (auto d : tnav::ORTHOGONAL_DIRECTION_SET) {
				if (n.getNeighborIndex(d) == -1)
					continue;

				connectedTiles.push_back(tileInfos[nodes[n.getNeighborIndex(d)].getTileInfoIndex()]);
			}
		}

		return connectedTiles;
	}

	// returns how prioritized the connection between these two tiles should be.
	// 0 == high prio, 1 == medium, 2 = low, 3 = should not be connected in the first place!
	int getConnectionPrio(TileInfo* a, TileInfo* b)
	{
		glm::vec3 aN = tnav::getNormal(a->type);
		glm::vec3 aP = getNode(a->nodeIndex)->getPosition();
		glm::vec3 bN = tnav::getNormal(b->type);
		glm::vec3 bP = getNode(b->nodeIndex)->getPosition();
		if (aP + (0.5f * aN) == bP + (0.5f * bN)) {
			// | a
			// |-> 
			// |___^___ b     inner connection
			return 0;	
		}
		else if (aN == bN && glm::length(aP - bP) == 1.0f) {
			SuperTileType superType = tnav::getSuperTileType(a->type);
			if ((superType == TILE_TYPE_XY && aP.z == bP.z) ||
				(superType == TILE_TYPE_XZ && aP.y == bP.y) ||
				(superType == TILE_TYPE_YZ && aP.x == bP.x)) {
				//
				// a__^__.__^__b   flat connection
				return 1;
			}
		}
		else if (aP - (0.5f * aN) == bP - (0.5f * bN)) {
			//  a|
			// <-| 
			//   |_______
			//       V  b   outer connection
			return 2;
		}
		// no possible connection
		return INT_MAX;
	}

	// returns a pointer to the center node of the newly created tile or nullptr if the tile could not be made.
	PositionNode* addTile(glm::vec3 pos, SuperTileType type)
	{
		// check if there is already a tile where we are trying to add one:
		// increment by 2s since we always add/remove 2 tiles at a time (front and back).
		for (int i = 0; i < tileInfos.size(); i += 2)
			if (nodes[tileInfos[i].nodeIndex].getPosition() == pos) 
				return nullptr;

		// create the new tile pair and center nodes:
		// indices are used because apparently pointers are unsafe if the vector resizes itself.
		int n1i = addNode();
		nodes[n1i].setPosition(pos); 
		nodes[n1i].setMappingID((MappingID)tnav::getTileType(type, true));
		
		int n2i = addNode();
		nodes[n2i].setPosition(pos);
		nodes[n2i].setMappingID((MappingID)tnav::getTileType(type, false));

		int t1i = addTileInfo();
		tileInfos[t1i].nodeIndex = getNode(n1i)->getIndex();
		tileInfos[t1i].type = tnav::getTileType(type, true);
		
		int  t2i = addTileInfo();
		tileInfos[t2i].nodeIndex = getNode(n2i)->getIndex();
		tileInfos[t2i].type = tnav::getTileType(type, false);

		switch (type) {
		case TILE_TYPE_XY:
			tileInfos[t1i].tileColor = glm::vec3(1, 0, 0);
			tileInfos[t2i].tileColor = glm::vec3(.5, 0, 0);
			break;
		case TILE_TYPE_XZ:
			tileInfos[t1i].tileColor = glm::vec3(0, 1, 0);
			tileInfos[t2i].tileColor = glm::vec3(0,.5, 0);
			break;
		case TILE_TYPE_YZ:
			tileInfos[t1i].tileColor = glm::vec3(0,0,1);
			tileInfos[t2i].tileColor = glm::vec3(0,0,.5);
			break;
		}

		nodes[n1i].setTileInfoIndex(t1i);
		nodes[n2i].setTileInfoIndex(t2i);

		// create side nodes and connect them:
		const glm::vec3* toSidesOffsets = tnav::getToSideNodePositionOffsets(type);
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			glm::vec3 sidePos = pos + toSidesOffsets[d];

			int ni = addNode();
			PositionNode* n = &nodes[ni];
			n->setPosition(sidePos);
			n->setMappingID(getNode(n1i)->getMappingID()); // arbitrary.

			std::vector<TileInfo> connectedTiles = getConnectedTiles(sidePos, n->getIndex());
			//std::cout << connectedTiles.size() << std::endl;
			if (connectedTiles.size() == 0) {
				// connect the side to just the new tile pair:
				getNode(n1i)->setNeighborIndex(n->getIndex(), d);
				getNode(n1i)->setNeighborMapID(MAP_ID_0, d);

				getNode(n2i)->setNeighborIndex(n->getIndex(), d);
				getNode(n2i)->setNeighborMapID(tnav::getNeighborMap(d, d), d);

				n->setNeighborIndex(getNode(n1i)->getIndex(), tnav::oppositeAlignment(d));
				n->setNeighborMapID(MAP_ID_0, tnav::oppositeAlignment(d));
				n->setNeighborIndex(getNode(n2i)->getIndex(), d);
				n->setNeighborMapID(tnav::getNeighborMap(d, d), d);
				continue;
			}

			// reconnect tiles based on heirarchy:
			for (TileInfo ti : connectedTiles) {
				LocalDirection tiD;
				PositionNode* tiN = getNode(ti.nodeIndex);

				for (LocalDirection dir : tnav::ORTHOGONAL_DIRECTION_SET) {
					if (nodes[tiN->getNeighborIndex(dir)].getPosition() == sidePos) {
						tiD = dir; break;
					}
				}
				TileInfo* currentNeighbor = getTileInfo(&ti, tiD);
				PositionNode* currentNeighborCenterNode = getNode(currentNeighbor->nodeIndex);
				
				int currentConnectionPrio = getConnectionPrio(&ti, currentNeighbor);
				int newConnectionPrio1 = getConnectionPrio(&ti, getTileInfo(t1i));
				int newConnectionPrio2 = getConnectionPrio(&ti, getTileInfo(t2i));
				std::cout << "og: tile " << ti.index << " to " << currentNeighbor->index << std::endl;
				std::cout << "p1: tile " << ti.index << " to " << getTileInfo(t1i)->index << std::endl;
				std::cout << "p2: tile " << ti.index << " to " << getTileInfo(t2i)->index << std::endl;
				std::cout << "og prio: " << currentConnectionPrio << ", new prio1: " << newConnectionPrio1 << ", new prio 2: " << newConnectionPrio2 << std::endl;
				if (currentConnectionPrio < newConnectionPrio1 && currentConnectionPrio < newConnectionPrio2) {
					// the current connection is of heavier prio, so we wont switch.
					continue;
				}
				
				PositionNode* connectedSide = getNode(tiN->getNeighborIndex(tiD));
				LocalDirection connectedSideOut = tnav::getMappedAlignment(tiN->getNeighborMapIndex(tiD), tiD);
				LocalDirection currentNeighborOut = tnav::getMappedAlignment(connectedSide->getNeighborMapIndex(connectedSideOut), tnav::oppositeAlignment(connectedSideOut));
				
				// we need to connect this side node to the new tile pair:

				if (newConnectionPrio1 < newConnectionPrio2) {
					// connectedSide connects ti to the new front tile in the direction of connectedSideOut
					connectedSide->setNeighborIndex(getNode(n1i)->getIndex(), connectedSideOut);
					connectedSide->setNeighborMapID(tnav::getNeighborMap(connectedSideOut, d), connectedSideOut);
					getNode(n1i)->setNeighborIndex(connectedSide->getIndex(), d);
					getNode(n1i)->setNeighborMapID(tnav::getNeighborMap(d, connectedSideOut), d);
					// n connects the new back tile to currentNeighbor
					n->setNeighborIndex(currentNeighborCenterNode->getIndex(), tnav::oppositeAlignment(d)); // n has same mapping as front tile center node.
					n->setNeighborMapID(tnav::getNeighborMap(tnav::oppositeAlignment(d), currentNeighborOut), tnav::oppositeAlignment(d));
					n->setNeighborIndex(getNode(n2i)->getIndex(), d);
					n->setNeighborMapID(tnav::getNeighborMap(d, d),d);
					currentNeighborCenterNode->setNeighborIndex(n->getIndex(), currentNeighborOut);
					currentNeighborCenterNode->setNeighborMapID(tnav::getNeighborMap(currentNeighborOut, tnav::oppositeAlignment(d)), currentNeighborOut);
					getNode(n2i)->setNeighborIndex(n->getIndex(), d);
					getNode(n2i)->setNeighborMapID(tnav::getNeighborMap(d, d), d);
				}
				else {
					// connectedSide connects ti to the new back tile in the direction of connectedSideOut
					PositionNode c;
					LocalDirection oppD = tnav::oppositeAlignment(d);
					c.setMappingID(getNode(n2i)->getMappingID());
					c.setIndex(connectedSide->getIndex());
					c.setPosition(connectedSide->getPosition());
					c.setNeighborIndex(n2i, oppD);
					c.setNeighborMapID(MAP_ID_0, oppD);
					c.setNeighborIndex(tiN->getIndex(), d);
					c.setNeighborMapID(tnav::getNeighborMap(d, tiD), d);
					(*connectedSide) = c;
					tiN->setNeighborMapID(tnav::getNeighborMap(tiD, d), tiD);
					getNode(n2i)->setNeighborIndex(connectedSide->getIndex(), d);
					getNode(n2i)->setNeighborMapID(MAP_ID_0, d);
					// n connects the new front tile to currentNeighbor
					
					n->setNeighborIndex(currentNeighborCenterNode->getIndex(), d); // n has same mapping as front tile center node.
					n->setNeighborMapID(tnav::getNeighborMap(d, currentNeighborOut), d);
					n->setNeighborIndex(getNode(n1i)->getIndex(), tnav::oppositeAlignment(d));
					n->setNeighborMapID(MAP_ID_0, tnav::oppositeAlignment(d));
					currentNeighborCenterNode->setNeighborIndex(n->getIndex(), currentNeighborOut);
					currentNeighborCenterNode->setNeighborMapID(tnav::getNeighborMap(currentNeighborOut, d), currentNeighborOut);
					getNode(n1i)->setNeighborIndex(n->getIndex(), d);
					getNode(n1i)->setNeighborMapID(MAP_ID_0, d);
				}
				break;
			}
		}
	}
};