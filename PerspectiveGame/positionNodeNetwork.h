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
		addTile(glm::vec3(0, 0, 0), TILE_TYPE_XY);
		addTile(glm::vec3(0, 1, 0), TILE_TYPE_XY);

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
		LocalDirection d2 = tnav::map(node->getNeighborMap(d), d);
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
			node = &nodes[(int)nodes.size() - 1];
			node->setIndex((int)nodes.size() - 1);
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
			info = &tileInfos[(int)tileInfos.size() - 1];
			info->index = (int)tileInfos.size() - 1;
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
	TileInfo* addTile(glm::vec3 pos, SuperTileType type)
	{
		// check if there is already a tile where we are trying to add one:
		// increment by 2s since we always add/remove 2 tiles at a time (front and back).
		for (int i = 0; i < tileInfos.size(); i += 2)
			if (nodes[tileInfos[i].nodeIndex].getPosition() == pos) 
				return nullptr;

		// create the new tile pair and center nodes:
		// indices are used because apparently pointers are unsafe if the vector resizes itself.
		int newFrontNodeIndex = addNode();
		nodes[newFrontNodeIndex].setPosition(pos); 
		nodes[newFrontNodeIndex].setMappingID((MappingID)tnav::getTileType(type, true));
		
		int newBackNodeIndex = addNode();
		nodes[newBackNodeIndex].setPosition(pos);
		nodes[newBackNodeIndex].setMappingID((MappingID)tnav::getTileType(type, false));

		int newFrontTile = addTileInfo();
		tileInfos[newFrontTile].nodeIndex = getNode(newFrontNodeIndex)->getIndex();
		tileInfos[newFrontTile].type = tnav::getTileType(type, true);
		
		int newBackTile = addTileInfo();
		tileInfos[newBackTile].nodeIndex = getNode(newBackNodeIndex)->getIndex();
		tileInfos[newBackTile].type = tnav::getTileType(type, false);

		switch (type) {
		case TILE_TYPE_XY:
			tileInfos[newFrontTile].tileColor = glm::vec3(1, 0, 0);
			tileInfos[newBackTile].tileColor = glm::vec3(.5, 0, 0);
			break;
		case TILE_TYPE_XZ:
			tileInfos[newFrontTile].tileColor = glm::vec3(0, 1, 0);
			tileInfos[newBackTile].tileColor = glm::vec3(0,.5, 0);
			break;
		case TILE_TYPE_YZ:
			tileInfos[newFrontTile].tileColor = glm::vec3(0,0,1);
			tileInfos[newBackTile].tileColor = glm::vec3(0,0,.5);
			break;
		}

		nodes[newFrontNodeIndex].setTileInfoIndex(newFrontTile);
		nodes[newBackNodeIndex].setTileInfoIndex(newBackTile);

		// create side nodes and connect them:
		const glm::vec3* toSidesOffsets = tnav::getToSideNodePositionOffsets(type);
		for (LocalDirection newSideNodeDir : tnav::ORTHOGONAL_DIRECTION_SET) {
			glm::vec3 sidePos = pos + toSidesOffsets[newSideNodeDir];

			PositionNode* newNode = &nodes[addNode()];
			newNode->setPosition(sidePos);
			newNode->setMappingID(getNode(newFrontNodeIndex)->getMappingID()); // arbitrary.
			// now that we are done changing the vector size, we can be confident pointers are safe:
			PositionNode* newFrontNode = getNode(newFrontNodeIndex);
			PositionNode* newBackNode = getNode(newBackNodeIndex);

			std::vector<TileInfo> linkedTiles = getConnectedTiles(sidePos, newNode->getIndex());
			//std::cout << connectedTiles.size() << std::endl;
			if (linkedTiles.size() == 0) {
				// connect the side to just the new tile pair:
				newFrontNode->setNeighborIndex(newNode->getIndex(), newSideNodeDir);
				newFrontNode->setNeighborMapID(MAP_ID_0, newSideNodeDir);

				newBackNode->setNeighborIndex(newNode->getIndex(), newSideNodeDir);
				newBackNode->setNeighborMapID(tnav::getNeighborMap(newSideNodeDir, newSideNodeDir), newSideNodeDir);

				newNode->setNeighborIndex(newFrontNode->getIndex(), tnav::oppositeAlignment(newSideNodeDir));
				newNode->setNeighborMapID(MAP_ID_0, tnav::oppositeAlignment(newSideNodeDir));
				newNode->setNeighborIndex(newBackNode->getIndex(), newSideNodeDir);
				newNode->setNeighborMapID(tnav::getNeighborMap(newSideNodeDir, newSideNodeDir), newSideNodeDir);
				continue;
			}

			// reconnect tiles based on heirarchy:
			for (TileInfo linkedTile : linkedTiles) {
				LocalDirection linkedTileDir;
				PositionNode* linkedTileSideNode = getNode(linkedTile.nodeIndex);

				for (LocalDirection dir : tnav::ORTHOGONAL_DIRECTION_SET) {
					if (nodes[linkedTileSideNode->getNeighborIndex(dir)].getPosition() == sidePos) {
						linkedTileDir = dir; 
						break;
					}
				}
				TileInfo* currentNeighbor = getTileInfo(&linkedTile, linkedTileDir);
				PositionNode* currentNeighborCenterNode = getNode(currentNeighbor->nodeIndex);
				
				int currentConnectionPrio = getConnectionPrio(&linkedTile, currentNeighbor);
				int newConnectionPrio1 = getConnectionPrio(&linkedTile, getTileInfo(newFrontTile));
				int newConnectionPrio2 = getConnectionPrio(&linkedTile, getTileInfo(newBackTile));
				if (currentConnectionPrio < newConnectionPrio1 && currentConnectionPrio < newConnectionPrio2) {
					// the current connection is of heavier prio, so we wont switch.
					continue;
				}
				
				PositionNode* linkedSideNode = getNode(linkedTileSideNode->getNeighborIndex(linkedTileDir));
				LocalDirection connectedSideOut = tnav::map(linkedTileSideNode->getNeighborMap(linkedTileDir), linkedTileDir);
				LocalDirection currentNeighborOut = tnav::map(linkedSideNode->getNeighborMap(connectedSideOut), tnav::oppositeAlignment(connectedSideOut));
				
				LocalDirection oppD = tnav::oppositeAlignment(newSideNodeDir);

				// we need to connect this side node to the new tile pair:
				if (newConnectionPrio1 < newConnectionPrio2) {
					// connectedSide connects ti to the new front tile in the direction of connectedSideOut
					linkedSideNode->setNeighborIndex(newFrontNode->getIndex(), connectedSideOut);
					linkedSideNode->setNeighborMapID(tnav::getNeighborMap(connectedSideOut, newSideNodeDir), connectedSideOut);
					
					newFrontNode->setNeighborIndex(linkedSideNode->getIndex(), newSideNodeDir);
					newFrontNode->setNeighborMapID(tnav::getNeighborMap(newSideNodeDir, connectedSideOut), newSideNodeDir);
					
					// n connects the new back tile to currentNeighbor
					newNode->setNeighborIndex(currentNeighborCenterNode->getIndex(), oppD); // n has same mapping as front tile center node.
					newNode->setNeighborMapID(tnav::getNeighborMap(oppD, currentNeighborOut), oppD);
					newNode->setNeighborIndex(newBackNode->getIndex(), newSideNodeDir);
					newNode->setNeighborMapID(tnav::getNeighborMap(newSideNodeDir, newSideNodeDir),newSideNodeDir);
					
					currentNeighborCenterNode->setNeighborIndex(newNode->getIndex(), currentNeighborOut);
					currentNeighborCenterNode->setNeighborMapID(tnav::getNeighborMap(currentNeighborOut, oppD), currentNeighborOut);
					
					newBackNode->setNeighborIndex(newNode->getIndex(), newSideNodeDir);
					newBackNode->setNeighborMapID(tnav::getNeighborMap(newSideNodeDir, newSideNodeDir), newSideNodeDir);
				} 
				else {
					// linkedSideNode connects linkedTile to the new back tile in the direction of connectedSideOut:
					linkedSideNode->setMappingID(newBackNode->getMappingID());
					linkedSideNode->setNeighborIndex(newBackNodeIndex, oppD);
					linkedSideNode->setNeighborMapID(MAP_ID_0, oppD);
					linkedSideNode->setNeighborIndex(linkedTileSideNode->getIndex(), newSideNodeDir);
					linkedSideNode->setNeighborMapID(tnav::getNeighborMap(newSideNodeDir, linkedTileDir), newSideNodeDir);

					linkedTileSideNode->setNeighborMapID(tnav::getNeighborMap(linkedTileDir, newSideNodeDir), linkedTileDir);
					
					newBackNode->setNeighborIndex(linkedSideNode->getIndex(), newSideNodeDir);
					newBackNode->setNeighborMapID(MAP_ID_0, newSideNodeDir);
					
					// newNode connects the new front tile to currentNeighbor:
					newNode->setNeighborIndex(currentNeighborCenterNode->getIndex(), newSideNodeDir); // n has same mapping as front tile center node.
					newNode->setNeighborMapID(tnav::getNeighborMap(newSideNodeDir, currentNeighborOut), newSideNodeDir);
					newNode->setNeighborIndex(newFrontNode->getIndex(), oppD);
					newNode->setNeighborMapID(MAP_ID_0, oppD);
					
					currentNeighborCenterNode->setNeighborIndex(newNode->getIndex(), currentNeighborOut);
					currentNeighborCenterNode->setNeighborMapID(tnav::getNeighborMap(currentNeighborOut, newSideNodeDir), currentNeighborOut);
					
					newFrontNode->setNeighborIndex(newNode->getIndex(), newSideNodeDir);
					newFrontNode->setNeighborMapID(MAP_ID_0, newSideNodeDir);
				}
				break;
			}
		}
		return &tileInfos[newFrontTile];
	}
};