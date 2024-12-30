#pragma once

#include <iostream>
#include <vector>
#include <set>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "tileNavigation.h"
#include "positionNode.h"
#include "cameraManager.h"

struct PositionNodeNetwork {
private:
	std::vector<PositionNode*> nodes;
	std::vector<int> freeNodeIndices;

	std::vector<TileInfo> tileInfos;
	std::vector<int> freeTileInfoIndices;

	Camera* p_camera;

public: // Rendering:
	GLuint texID;
	GLuint positionNodeInfosBufferID;
	GLuint tileInfosBufferID;
	std::vector<glm::vec2> windowFrustum;

	std::vector<GPU_TileInfoNode> gpuTileInfos;
	std::vector<GPU_PositionNodeInfo> gpuPositionNodeInfos;

	CenterNode CurrentNode;
	int currentMapIndex = 0;
	int currentNodeIndex = 4;

public:
	PositionNodeNetwork(Camera* c) : p_camera(c)
	{
		createTilePair(glm::vec3(0, 0, 0), TILE_TYPE_XY);

		// Rendering:
		glGenBuffers(1, &positionNodeInfosBufferID);
		glGenBuffers(1, &tileInfosBufferID);
	}

	void update()
	{
		gpuPositionNodeInfos.clear();
		for (PositionNode* p : nodes) {
			gpuPositionNodeInfos.push_back(GPU_PositionNodeInfo(*p));
		}

		gpuTileInfos.clear();
		for (TileInfo& i : tileInfos) {
			gpuTileInfos.push_back(GPU_TileInfoNode(i));
		}
	}

	PositionNode* getNode(int index)
	{
		return nodes[index];
	}

	CenterNode* getNode(TileInfo* info)
	{
		return static_cast<CenterNode*>(nodes[info->centerNodeIndex]);
	}

	TileInfo* getTileInfo(int index)
	{
		return &tileInfos[index];
	}

	int numTileInfos() { return (int)tileInfos.size(); }

	TileInfo* getTileInfo(int tileInfoIndex, LocalDirection d)
	{
		return getTileInfo(&tileInfos[tileInfoIndex], d);
	}

	// Gives the tile info of the neighbor tile in the direction of d.
	TileInfo* getTileInfo(TileInfo* info, LocalDirection d)
	{
		CenterNode* node = static_cast<CenterNode*>(nodes[info->centerNodeIndex]);
		LocalDirection d2 = tnav::map(node->getNeighborMap(d), d);
		SideNode* sideNode = static_cast<SideNode*>(nodes[node->getNeighborIndex(d)]);
		CenterNode* neighborCenterNode = static_cast<CenterNode*>(nodes[sideNode->getNeighborIndex(d2)]);
		return &tileInfos[neighborCenterNode->getTileInfoIndex()];
	}

	PositionNode* getNeighbor(PositionNode* node, LocalDirection toNeighbor)
	{
		return nodes[node->getNeighborIndex(toNeighbor)];
	}

	CenterNode* getSecondNeighbor(CenterNode* node, LocalDirection toNeighbor)
	{
		SideNode* sideNode = static_cast<SideNode*>(nodes[node->getNeighborIndex(toNeighbor)]);
		LocalDirection d = tnav::map(node->getNeighborMap(toNeighbor), toNeighbor);
		return static_cast<CenterNode*>(nodes[sideNode->getNeighborIndex(d)]);
	}

	LocalDirection mapToSecondNeighbor(CenterNode* node, LocalDirection toNeighbor, LocalAlignment alignment)
	{
		SideNode* sideNode = static_cast<SideNode*>(nodes[node->getNeighborIndex(toNeighbor)]);
		alignment = tnav::map(node->getNeighborMap(toNeighbor), alignment);
		
		toNeighbor = tnav::map(node->getNeighborMap(toNeighbor), toNeighbor);
		return tnav::map(sideNode->getNeighborMap(toNeighbor), alignment);;
	}

	// Will add a node to the nodes list that is unconnected and error prone if it is not connected up!
	// returns an index to the added node.
	int addNode(PositionNodeType type)
	{
		PositionNode* node = nullptr;
		switch (type) {
		case NODE_TYPE_CENTER:
			node = new CenterNode();
			break;
		case NODE_TYPE_SIDE:
			node = (new SideNode(SIDE_NODE_TYPE_ERROR));
			break;
		default: //case NODE_TYPE_CORNER:
			node = (new CornerNode());
			break;
		}

		if (freeNodeIndices.size() > 0) {
			nodes[freeNodeIndices.back()] = node;
			node->setIndex(freeNodeIndices.back()); // all freeNode nodes are wiped
			freeNodeIndices.pop_back();
		}
		else {
			nodes.push_back(node);
			nodes.back()->setIndex((int)nodes.size() - 1);
		}

		return node->getIndex();
	}

	// center nodes inherantly require more information, hence the inputs:
	int addCenterNode(glm::vec3 pos, TileType type)
	{
		int i = addNode(NODE_TYPE_CENTER);
		nodes[i]->setPosition(pos);
		nodes[i]->type = NODE_TYPE_CENTER;
		nodes[i]->oriType = type;
		return i;
	}

	int addSideNode(glm::vec3 pos, OrientationType type)
	{
		int i = addNode(NODE_TYPE_SIDE);
		nodes[i]->setPosition(pos);
		nodes[i]->type = NODE_TYPE_SIDE;
		nodes[i]->oriType = type;
		return i;
	}

	int addCornerNode(glm::vec3 pos, OrientationType type)
	{
		int i = addNode(NODE_TYPE_CORNER);
		nodes[i]->setPosition(pos);
		nodes[i]->type = NODE_TYPE_CORNER;
		nodes[i]->oriType = type;
		return i;
	}

	void removeNode(int index)
	{
		freeNodeIndices.push_back(index);
		delete nodes[index];
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

		tileInfos[frontInfoIndex].centerNodeIndex = frontCenterNodeIndex;
		tileInfos[frontInfoIndex].type = tnav::getFrontTileType(type);
		tileInfos[frontInfoIndex].siblingIndex = backInfoIndex;

		tileInfos[backInfoIndex].centerNodeIndex = backCenterNodeIndex;
		tileInfos[backInfoIndex].type = tnav::getBackTileType(type);
		tileInfos[backInfoIndex].siblingIndex = frontInfoIndex;

		colorTile(frontInfoIndex);
		colorTile(backInfoIndex);
	}

	// given a position in space, returns all the tiles connected to that point in the network.
	// currently assumes the given position is of a side node!  will error if given a position 
	// relating to a center of corner node.
	std::vector<TileInfo> getConnectedTiles(SideNode* node)
	{
		std::vector<TileInfo> connectedTiles;
		int sideNodeIndex = node->index;
		glm::vec3 pos = nodes[sideNodeIndex]->getPosition();

		for (PositionNode* n : nodes) {
			if (n->getPosition() != pos || n->getIndex() == sideNodeIndex)
				continue;

			SideNode* s = static_cast<SideNode*>(n);
			for (int i = 0; i < 2; i++) { // side nodes only have 2 neighbors
				if (s->getNeighborIndexDirect(i) == -1)
					continue;

				connectedTiles.push_back(
					tileInfos[
						static_cast<CenterNode*>(nodes[s->getNeighborIndexDirect(i)])->getTileInfoIndex()
					]
				);
			}
		}

		return connectedTiles;
	}

	// returns how prioritized the connection between these two tiles should be.
	// 0 == high prio, 1 == medium, 2 = low, 3 = should not be connected in the first place!
	int getConnectionPrio(TileInfo* a, TileInfo* b)
	{
		glm::vec3 aN = tnav::getNormal(a->type);
		glm::vec3 aP = getNode(a->centerNodeIndex)->getPosition();
		glm::vec3 bN = tnav::getNormal(b->type);
		glm::vec3 bP = getNode(b->centerNodeIndex)->getPosition();
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
		TileInfo* backTile = getTileInfo(frontTile->siblingIndex);
		CenterNode* frontCenterNode = static_cast<CenterNode*>(getNode(frontTile->centerNodeIndex));
		CenterNode* backCenterNode = static_cast<CenterNode*>(getNode(backTile->centerNodeIndex));
		SideNode* newSideNode; 
		
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			// add a new side node is none exists:
			if (frontCenterNode->getNeighborIndex(d) == -1) {
				const glm::vec3* toSidesOffsets = tnav::getNodePositionOffsets(tnav::getSuperTileType(frontTile->type));
				glm::vec3 sidePos = frontCenterNode->getPosition() + toSidesOffsets[d];
				newSideNode = static_cast<SideNode*>(nodes[addSideNode(sidePos, ORIENTATION_TYPE_ERROR)]);
				newSideNode->setPosition(sidePos);
			}
			else { // if one exists, just use it:
				newSideNode = static_cast<SideNode*>(getNode(frontCenterNode->getNeighborIndex(d)));
			}
			newSideNode->oriType = frontCenterNode->oriType; // Arbitrary but convenient later.
			newSideNode->setSideNodeType(
				(d == LOCAL_DIRECTION_0 || d == LOCAL_DIRECTION_2)
				? SIDE_NODE_TYPE_HORIZONTAL
				: SIDE_NODE_TYPE_VERTICAL);

			std::vector<TileInfo> linkedTiles = getConnectedTiles(newSideNode);
			if (linkedTiles.size() == 0) {
				// connect the side to just the new tile pair:
				frontCenterNode->setNeighborIndex(d, newSideNode->getIndex());
				frontCenterNode->setNeighborMap(d, MAP_TYPE_IDENTITY);

				backCenterNode->setNeighborIndex(d, newSideNode->getIndex());
				backCenterNode->setNeighborMap(d, tnav::getNeighborMap(d, d));

				newSideNode->setNeighborIndex(tnav::inverse(d), frontCenterNode->getIndex());
				newSideNode->setNeighborMap(tnav::inverse(d), MAP_TYPE_IDENTITY);
				newSideNode->setNeighborIndex(d, backCenterNode->getIndex());
				newSideNode->setNeighborMap(d, tnav::getNeighborMap(d, d));
				continue;
			}

			// reconnect tiles based on heirarchy:
			for (TileInfo linkedTile : linkedTiles) {
				LocalDirection linkedTileDir;
				CenterNode* linkedTileCenterNode = static_cast<CenterNode*>(getNode(linkedTile.centerNodeIndex));

				for (LocalDirection dir : tnav::ORTHOGONAL_DIRECTION_SET) {
					if (nodes[linkedTileCenterNode->getNeighborIndex(dir)]->getPosition() == newSideNode->getPosition()) {
						linkedTileDir = dir;
						break;
					}
				}
				TileInfo* currentNeighbor = getTileInfo(&linkedTile, linkedTileDir);
				CenterNode* currentNeighborCenterNode = getNode(currentNeighbor);

				int currentConnectionPrio = getConnectionPrio(&linkedTile, currentNeighbor);
				int newConnectionPrio1 = getConnectionPrio(&linkedTile, getTileInfo(frontTile->index));
				int newConnectionPrio2 = getConnectionPrio(&linkedTile, getTileInfo(backTile->index));
				if (currentConnectionPrio < newConnectionPrio1 && currentConnectionPrio < newConnectionPrio2) {
					// the current connection is of heavier prio, so we wont switch.
					continue;
				}

				SideNode* linkedSideNode = static_cast<SideNode*>(getNode(linkedTileCenterNode->getNeighborIndex(linkedTileDir)));
				LocalDirection linkedSideNodeOut = tnav::map(linkedTileCenterNode->getNeighborMap(linkedTileDir), linkedTileDir);
				LocalDirection currentNeighborOut = tnav::map(linkedSideNode->getNeighborMap(linkedSideNodeOut), tnav::inverse(linkedSideNodeOut));

				LocalDirection oppD = tnav::inverse(d);

				// we need to connect this side node to the new tile pair:
				if (newConnectionPrio1 < newConnectionPrio2) {
					// connectedSide connects ti to the new front tile in the direction of connectedSideOut
					linkedSideNode->setNeighborIndex(linkedSideNodeOut, frontCenterNode->getIndex());
					linkedSideNode->setNeighborMap(linkedSideNodeOut, tnav::getNeighborMap(linkedSideNodeOut, d));

					frontCenterNode->setNeighborIndex(d, linkedSideNode->getIndex());
					frontCenterNode->setNeighborMap(d, tnav::getNeighborMap(d, linkedSideNodeOut));

					// n connects the new back tile to currentNeighbor
					newSideNode->setNeighborIndex(oppD, currentNeighborCenterNode->getIndex()); // n has same mapping as front tile center node.
					newSideNode->setNeighborMap(oppD, tnav::getNeighborMap(oppD, currentNeighborOut));
					newSideNode->setNeighborIndex(d, backCenterNode->getIndex());
					newSideNode->setNeighborMap(d, tnav::getNeighborMap(d, d));

					currentNeighborCenterNode->setNeighborIndex(currentNeighborOut, newSideNode->getIndex());
					currentNeighborCenterNode->setNeighborMap(currentNeighborOut, tnav::getNeighborMap(currentNeighborOut, oppD));

					backCenterNode->setNeighborIndex(d, newSideNode->getIndex());
					backCenterNode->setNeighborMap(d, tnav::getNeighborMap(d, d));
				}
				else {
					// linkedSideNode connects linkedTile to the new back tile in the direction of connectedSideOut:
					linkedSideNode->oriType = backCenterNode->oriType;
					linkedSideNode->setSideNodeType(
						(d == LOCAL_DIRECTION_0 || d == LOCAL_DIRECTION_2)
						? SIDE_NODE_TYPE_HORIZONTAL
						: SIDE_NODE_TYPE_VERTICAL);
					linkedSideNode->setNeighborIndex(oppD, backCenterNode->index);
					linkedSideNode->setNeighborMap(oppD, MAP_TYPE_IDENTITY);
					linkedSideNode->setNeighborIndex(d, linkedTileCenterNode->getIndex());
					linkedSideNode->setNeighborMap(d, tnav::getNeighborMap(d, linkedTileDir));

					linkedTileCenterNode->setNeighborMap(linkedTileDir, tnav::getNeighborMap(linkedTileDir, d));

					backCenterNode->setNeighborIndex(d, linkedSideNode->getIndex());
					backCenterNode->setNeighborMap(d, MAP_TYPE_IDENTITY);

					// newNode connects the new front tile to currentNeighbor:
					newSideNode->setNeighborIndex(d, currentNeighborCenterNode->getIndex()); // n has same mapping as front tile center node.
					newSideNode->setNeighborMap(d, tnav::getNeighborMap(d, currentNeighborOut));
					newSideNode->setNeighborIndex(oppD, frontCenterNode->getIndex());
					newSideNode->setNeighborMap(oppD, MAP_TYPE_IDENTITY);

					currentNeighborCenterNode->setNeighborIndex(currentNeighborOut, newSideNode->getIndex());
					currentNeighborCenterNode->setNeighborMap(currentNeighborOut, tnav::getNeighborMap(currentNeighborOut, d));

					frontCenterNode->setNeighborIndex(d, newSideNode->getIndex());
					frontCenterNode->setNeighborMap(d, MAP_TYPE_IDENTITY);
				}
				break;
			}
		}
	}

	// returns a pointer to the center node of the newly created tile or nullptr if the tile could not be made.
	TileInfo* createTilePair(glm::vec3 pos, SuperTileType type)
	{
		// check if there is already a tile where we are trying to add one:
		// increment by 2s since we always add/remove 2 tiles at a time (front and back).
		for (int i = 0; i < tileInfos.size(); i += 2) {
			if (tileInfos[i].index == -1) continue;
			if (nodes[tileInfos[i].centerNodeIndex]->getPosition() == pos)
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
		static_cast<CenterNode*>(nodes[newFrontNodeIndex])->setTileInfoIndex(newFrontTileIndex);
		static_cast<CenterNode*>(nodes[newBackNodeIndex])->setTileInfoIndex(newBackTileIndex);

		connectTilePair(&tileInfos[newFrontTileIndex]);		

		// TODO: add/remove corner nodes based on new geometry:

		return &tileInfos[newFrontTileIndex];
	}

	void deleteTilePair(int tileInfoIndex) { deleteTilePair(&tileInfos[tileInfoIndex]); }

	void deleteTilePair(TileInfo* t)
	{
		using namespace tnav;

		if (t == nullptr) return;

		TileInfo* siblingTile = getTileInfo(t->siblingIndex);
		int centerNodeIndex = (t->centerNodeIndex);
		int sibCenterNodeIndex = (siblingTile->centerNodeIndex);

		// reconnect edges:
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			TileInfo
				* neighborTile1 = getTileInfo(t, d),
				* neighborTile2 = getTileInfo(t->siblingIndex, d);
			if (neighborTile1 == siblingTile) {
				// edges that dont connect to anything can be simply removed, 
				// as no reconnection is necessary:
				removeNode(getNode(centerNodeIndex)->getNeighborIndex(d));
				continue;
			}

			CenterNode* centerNode1 = static_cast<CenterNode*>(getNode(centerNodeIndex));
			SideNode* sideNode1 = static_cast<SideNode*>(getNode(centerNode1->getNeighborIndex(d)));
			CenterNode* neighborCenterNode1 = static_cast<CenterNode*>(getNode(neighborTile1->centerNodeIndex));
			CenterNode* centerNode2 = static_cast<CenterNode*>(getNode(sibCenterNodeIndex));
			SideNode* sideNode2 = static_cast<SideNode*>(getNode(centerNode2->getNeighborIndex(d)));
			CenterNode* neighborCenterNode2 = static_cast<CenterNode*>(getNode(neighborTile2->centerNodeIndex));
			SideNode* newNode = static_cast<SideNode*>(getNode(addNode(NODE_TYPE_SIDE)));
			
			LocalDirection 
				n1ToNew = mapToSecondNeighbor(centerNode1, d, inverse(d)),
				n2ToNew = mapToSecondNeighbor(centerNode2, d, inverse(d)),
				newToN1 = inverse(n1ToNew),
				newToN2 = n1ToNew;
			
			MapType 
				newToN2Map = getNeighborMap(newToN2, n2ToNew),
				n2ToNewMap = inverse(newToN2Map);
			
			// While we could edit the existing nodes, its conceptually simpler to just make a fresh one:
			newNode->setPosition(sideNode1->getPosition());
			newNode->oriType = neighborCenterNode1->oriType; // makes sure the transition from center node type -> new side node type is possible
			newNode->setSideNodeType(
				(n1ToNew == LOCAL_DIRECTION_0 || n1ToNew == LOCAL_DIRECTION_2)
				? SIDE_NODE_TYPE_HORIZONTAL
				: SIDE_NODE_TYPE_VERTICAL);

			neighborCenterNode1->setNeighborIndex(n1ToNew, newNode->getIndex());
			neighborCenterNode1->setNeighborMap(n1ToNew, MAP_TYPE_IDENTITY);
			newNode->setNeighborIndex(newToN2, neighborCenterNode2->getIndex());
			newNode->setNeighborMap(newToN2, newToN2Map);
			neighborCenterNode2->setNeighborIndex(n2ToNew, newNode->getIndex());
			neighborCenterNode2->setNeighborMap(n2ToNew, n2ToNewMap);
			newNode->setNeighborIndex(newToN1, neighborCenterNode1->getIndex());
			newNode->setNeighborMap(newToN1, MAP_TYPE_IDENTITY);

			removeNode(sideNode1->getIndex());
			removeNode(sideNode2->getIndex());
		}
		
		removeTileInfo(static_cast<CenterNode*>(getNode(centerNodeIndex))->getTileInfoIndex());
		removeTileInfo(static_cast<CenterNode*>(getNode(sibCenterNodeIndex))->getTileInfoIndex());
		removeNode(centerNodeIndex);
		removeNode(sibCenterNodeIndex);

		// TODO: add/remove corner nodes based on new geometry:
	}
};