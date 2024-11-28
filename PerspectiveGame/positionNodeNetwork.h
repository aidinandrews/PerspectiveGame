#pragma once

#include <iostream>
#include <vector>
#include <set>

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
		createTile(glm::vec3(0, 0, 0), TILE_TYPE_XY);

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

	int numTileInfos() { return (int)tileInfos.size(); }

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

	// center nodes inherantly require more information, hence the inputs:
	int addCenterNode(glm::vec3 pos, TileType type)
	{
		int i = addNode();
		nodes[i].setPosition(pos);
		nodes[i].setMappingID((MappingID)type);
		for (auto d : tnav::DIRECTION_SET) {
			//nodes[i].setNeighborIndex(-1, d);
			//nodes[i].setNeighborMapID(-1, d);
		}
		return i;
	}

	void removeNode(int index, bool removeConnections)
	{
		PositionNode* node = &nodes[index];

		if (removeConnections) {
			for (auto d : tnav::DIRECTION_SET) { // disconnect neighbor nodes:
				if (node->getNeighborIndex(d) == -1)
					continue;
				LocalDirection D = tnav::map(node->getNeighborMap(d), tnav::oppositeAlignment(d));
				PositionNode* neighbor = getNode(node->getNeighborIndex(d));
				neighbor->setNeighborIndex(-1, D);
				neighbor->setNeighborMap(-1, D);
			}
		}

		node->wipe();
		freeNodeIndices.push_back(index);
	}

	void colorTile(int index)
	{

		switch (tileInfos[index].type) {
		case TILE_TYPE_XYF: tileInfos[index].color = glm::vec3(1.0f, 0.0f, 0.0f); break;
		case TILE_TYPE_XYB: tileInfos[index].color = glm::vec3(0.5f, 0.0f, 0.0f); break;
		case TILE_TYPE_XZF: tileInfos[index].color = glm::vec3(0.0f, 1.0f, 0.0f); break;
		case TILE_TYPE_XZB: tileInfos[index].color = glm::vec3(0.0f, 0.5f, 0.0f); break;
		case TILE_TYPE_YZF: tileInfos[index].color = glm::vec3(0.0f, 0.0f, 1.0f); break;
		case TILE_TYPE_YZB: tileInfos[index].color = glm::vec3(0.0f, 0.0f, 0.5f); break;
		}
	}

	void removeTileInfo(int index)
	{
		tileInfos[index].wipe();
		freeTileInfoIndices.push_back(index);
	}

	// Adds an UNINITIALIZED tile info to the tileInfos list.  Returns a pointer to the added tileInfo.
	// frontIndex and backIndex are meant to be used as returns.
	void addTileInfoPair(int frontCenterNodeIndex, int backCenterNodeIndex, SuperTileType type, int& frontInfoIndex, int& backInfoIndex)
	{
		if (freeTileInfoIndices.size() > 1) {
			frontInfoIndex = freeTileInfoIndices.back();
			tileInfos[frontInfoIndex].index = freeTileInfoIndices.back();
			freeTileInfoIndices.pop_back();

			backInfoIndex = freeTileInfoIndices.back();
			tileInfos[backInfoIndex].index = freeTileInfoIndices.back();
			freeTileInfoIndices.pop_back();
		}
		else if (freeTileInfoIndices.size() == 1) {
			frontInfoIndex = freeTileInfoIndices.back();
			tileInfos[frontInfoIndex].index = freeTileInfoIndices.back();
			freeTileInfoIndices.pop_back();

			tileInfos.push_back(TileInfo());
			backInfoIndex = (int)tileInfos.size() - 1;
			tileInfos[backInfoIndex].index = (int)tileInfos.size() - 1;
		}
		else {
			tileInfos.push_back(TileInfo());
			frontInfoIndex = (int)tileInfos.size() - 1;
			tileInfos[frontInfoIndex].index = (int)tileInfos.size() - 1;

			tileInfos.push_back(TileInfo());
			backInfoIndex = (int)tileInfos.size() - 1;
			tileInfos[backInfoIndex].index = (int)tileInfos.size() - 1;
		}

		tileInfos[frontInfoIndex].nodeIndex = frontCenterNodeIndex;
		tileInfos[frontInfoIndex].type = tnav::getFrontTileType(type);
		tileInfos[frontInfoIndex].siblingIndex = backInfoIndex;

		tileInfos[backInfoIndex].nodeIndex = backCenterNodeIndex;
		tileInfos[backInfoIndex].type = tnav::getBackTileType(type);
		tileInfos[backInfoIndex].siblingIndex = frontInfoIndex;

		colorTile(frontInfoIndex);
		colorTile(backInfoIndex);
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

	// Given a tile pair, will connect OR reconnect all the side nodes of that tile to the world.
	void connectTilePair(TileInfo* frontTile)
	{
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			TileInfo* backTile = getTileInfo(frontTile->siblingIndex);
			PositionNode* frontCenterNode = getNode(frontTile->nodeIndex);
			PositionNode* backCenterNode = getNode(backTile->nodeIndex);
			PositionNode* newSideNode; 

			// add a new side node is none exists:
			if (frontCenterNode->getNeighborIndex(d) == -1) {
				const glm::vec3* toSidesOffsets = tnav::getNodePositionOffsets(tnav::getSuperTileType(frontTile->type));
				glm::vec3 sidePos = frontCenterNode->getPosition() + toSidesOffsets[d];
				newSideNode = &nodes[addNode()];
				newSideNode->setPosition(sidePos);

				frontCenterNode = getNode(frontTile->nodeIndex);
				backCenterNode = getNode(backTile->nodeIndex);
			}
			else { // if one exists, just use it:
				newSideNode = getNode(frontCenterNode->getNeighborIndex(d));
			}
			newSideNode->setMappingID(frontCenterNode->getMappingID()); // Arbitrary but convenient later.

			std::vector<TileInfo> linkedTiles = getConnectedTiles(newSideNode->getPosition(), newSideNode->getIndex());
			if (linkedTiles.size() == 0) {
				// connect the side to just the new tile pair:
				frontCenterNode->setNeighborIndex(newSideNode->getIndex(), d);
				frontCenterNode->setNeighborMap(MAP_ID_0, d);

				backCenterNode->setNeighborIndex(newSideNode->getIndex(), d);
				backCenterNode->setNeighborMap(tnav::getNeighborMap(d, d), d);

				newSideNode->setNeighborIndex(frontCenterNode->getIndex(), tnav::oppositeAlignment(d));
				newSideNode->setNeighborMap(MAP_ID_0, tnav::oppositeAlignment(d));
				newSideNode->setNeighborIndex(backCenterNode->getIndex(), d);
				newSideNode->setNeighborMap(tnav::getNeighborMap(d, d), d);
				continue;
			}

			// reconnect tiles based on heirarchy:
			for (TileInfo linkedTile : linkedTiles) {
				LocalDirection linkedTileDir;
				PositionNode* linkedTileSideNode = getNode(linkedTile.nodeIndex);

				for (LocalDirection dir : tnav::ORTHOGONAL_DIRECTION_SET) {
					if (nodes[linkedTileSideNode->getNeighborIndex(dir)].getPosition() == newSideNode->getPosition()) {
						linkedTileDir = dir;
						break;
					}
				}
				TileInfo* currentNeighbor = getTileInfo(&linkedTile, linkedTileDir);
				PositionNode* currentNeighborCenterNode = getNode(currentNeighbor->nodeIndex);

				int currentConnectionPrio = getConnectionPrio(&linkedTile, currentNeighbor);
				int newConnectionPrio1 = getConnectionPrio(&linkedTile, getTileInfo(frontTile->index));
				int newConnectionPrio2 = getConnectionPrio(&linkedTile, getTileInfo(backTile->index));
				if (currentConnectionPrio < newConnectionPrio1 && currentConnectionPrio < newConnectionPrio2) {
					// the current connection is of heavier prio, so we wont switch.
					continue;
				}

				PositionNode* linkedSideNode = getNode(linkedTileSideNode->getNeighborIndex(linkedTileDir));
				LocalDirection connectedSideOut = tnav::map(linkedTileSideNode->getNeighborMap(linkedTileDir), linkedTileDir);
				LocalDirection currentNeighborOut = tnav::map(linkedSideNode->getNeighborMap(connectedSideOut), tnav::oppositeAlignment(connectedSideOut));

				LocalDirection oppD = tnav::oppositeAlignment(d);

				// we need to connect this side node to the new tile pair:
				if (newConnectionPrio1 < newConnectionPrio2) {
					// connectedSide connects ti to the new front tile in the direction of connectedSideOut
					linkedSideNode->setNeighborIndex(frontCenterNode->getIndex(), connectedSideOut);
					linkedSideNode->setNeighborMap(tnav::getNeighborMap(connectedSideOut, d), connectedSideOut);

					frontCenterNode->setNeighborIndex(linkedSideNode->getIndex(), d);
					frontCenterNode->setNeighborMap(tnav::getNeighborMap(d, connectedSideOut), d);

					// n connects the new back tile to currentNeighbor
					newSideNode->setNeighborIndex(currentNeighborCenterNode->getIndex(), oppD); // n has same mapping as front tile center node.
					newSideNode->setNeighborMap(tnav::getNeighborMap(oppD, currentNeighborOut), oppD);
					newSideNode->setNeighborIndex(backCenterNode->getIndex(), d);
					newSideNode->setNeighborMap(tnav::getNeighborMap(d, d), d);

					currentNeighborCenterNode->setNeighborIndex(newSideNode->getIndex(), currentNeighborOut);
					currentNeighborCenterNode->setNeighborMap(tnav::getNeighborMap(currentNeighborOut, oppD), currentNeighborOut);

					backCenterNode->setNeighborIndex(newSideNode->getIndex(), d);
					backCenterNode->setNeighborMap(tnav::getNeighborMap(d, d), d);
				}
				else {
					// linkedSideNode connects linkedTile to the new back tile in the direction of connectedSideOut:
					linkedSideNode->setMappingID(backCenterNode->getMappingID());
					linkedSideNode->setNeighborIndex(backTile->nodeIndex, oppD);
					linkedSideNode->setNeighborMap(MAP_ID_0, oppD);
					linkedSideNode->setNeighborIndex(linkedTileSideNode->getIndex(), d);
					linkedSideNode->setNeighborMap(tnav::getNeighborMap(d, linkedTileDir), d);

					linkedTileSideNode->setNeighborMap(tnav::getNeighborMap(linkedTileDir, d), linkedTileDir);

					backCenterNode->setNeighborIndex(linkedSideNode->getIndex(), d);
					backCenterNode->setNeighborMap(MAP_ID_0, d);

					// newNode connects the new front tile to currentNeighbor:
					newSideNode->setNeighborIndex(currentNeighborCenterNode->getIndex(), d); // n has same mapping as front tile center node.
					newSideNode->setNeighborMap(tnav::getNeighborMap(d, currentNeighborOut), d);
					newSideNode->setNeighborIndex(frontCenterNode->getIndex(), oppD);
					newSideNode->setNeighborMap(MAP_ID_0, oppD);

					currentNeighborCenterNode->setNeighborIndex(newSideNode->getIndex(), currentNeighborOut);
					currentNeighborCenterNode->setNeighborMap(tnav::getNeighborMap(currentNeighborOut, d), currentNeighborOut);

					frontCenterNode->setNeighborIndex(newSideNode->getIndex(), d);
					frontCenterNode->setNeighborMap(MAP_ID_0, d);
				}
				break;
			}
		}
	}

	// returns a pointer to the center node of the newly created tile or nullptr if the tile could not be made.
	TileInfo* createTile(glm::vec3 pos, SuperTileType type)
	{
		// check if there is already a tile where we are trying to add one:
		// increment by 2s since we always add/remove 2 tiles at a time (front and back).
		for (int i = 0; i < tileInfos.size(); i += 2) {
			if (tileInfos[i].index == -1) continue;
			if (nodes[tileInfos[i].nodeIndex].getPosition() == pos)
				return nullptr;
		}

		// create the new tile pair and center nodes:
		// indices are used because apparently pointers are unsafe if the vector resizes itself.
		TileType frontType = tnav::getTileType(type, true);
		TileType backType = tnav::getTileType(type, false);
		int newFrontNodeIndex = addCenterNode(pos, frontType);
		int newBackNodeIndex = addCenterNode(pos, backType);
		int newFrontTileIndex, newBackTileIndex;
		addTileInfoPair(newFrontNodeIndex, newBackNodeIndex, type, newFrontTileIndex, newBackTileIndex);
		nodes[newFrontNodeIndex].setTileInfoIndex(newFrontTileIndex);
		nodes[newBackNodeIndex].setTileInfoIndex(newBackTileIndex);

		connectTilePair(&tileInfos[newFrontTileIndex]);		

		// TODO: add/remove corner nodes based on new geometry:

		return &tileInfos[newFrontTileIndex];
	}

	void deleteTilePair(int tileInfoIndex) { deleteTilePair(&tileInfos[tileInfoIndex]); }

	void deleteTilePair(TileInfo* t)
	{
		using namespace tnav;

		if (t == nullptr) return;

		TileInfo* sibT = getTileInfo(t->siblingIndex);
		int centerNode = (t->nodeIndex);
		int sibCenterNode = (sibT->nodeIndex);

		// reconnect edges:
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			// edges that dont connect to anything can be simply removed, as no reconnection is necessary:
			TileInfo* neighborTile = getTileInfo(t, d);

			if (neighborTile == sibT) {
				removeNode(getNode(centerNode)->getNeighborIndex(d), false);
				continue;
			}

			PositionNode* curr = getNode(centerNode);
			PositionNode* side = getNode(curr->getNeighborIndex(d));
			PositionNode* sCur = getNode(sibCenterNode);
			PositionNode* Ssid = getNode(sCur->getNeighborIndex(d));
			PositionNode* neig = getNode(neighborTile->nodeIndex);
			PositionNode* sibl = getNode(getTileInfo(neighborTile->siblingIndex)->nodeIndex);
			
			// While we could edit the existing nodes, its conceptually simpler to just make a fresh one:
			PositionNode newNode;
			newNode.setIndex(side->getIndex());
			newNode.setPosition(side->getPosition());
			newNode.setMappingID(neig->getMappingID());
			LocalDirection D = tnav::map(curr->getNeighborMap(d), d);
			D = tnav::map(side->getNeighborMap(D), D);
			newNode.setNeighborIndex(neig->getIndex(), D);
			newNode.setNeighborMap(0, D);
			LocalDirection D1 = tnav::map(sCur->getNeighborMap(d), d);
			D1 = tnav::map(Ssid->getNeighborMap(D1), D1);
			int m = tnav::getNeighborMap(tnav::oppositeAlignment(D), tnav::oppositeAlignment(D1));
			newNode.setNeighborIndex(sibl->getIndex(), tnav::oppositeAlignment(D));
			newNode.setNeighborMap(m, tnav::oppositeAlignment(D));
			(*side) = newNode; // arbitrary, one has to go, the other stays.

			// neigh or sibl should already connect to newNode, but just for completeness, reconnect both:
			neig->setNeighborIndex(newNode.getIndex(), tnav::oppositeAlignment(D));
			neig->setNeighborMap(0, tnav::oppositeAlignment(D));
			sibl->setNeighborIndex(newNode.getIndex(), tnav::oppositeAlignment(D1));
			sibl->setNeighborMap(tnav::inverseMapID(m), tnav::oppositeAlignment(D1));

			removeNode(Ssid->getIndex(), false);
		}

		removeNode(centerNode, false);
		removeNode(sibCenterNode, false);

		// TODO: add/remove corner nodes based on new geometry:
	}


};