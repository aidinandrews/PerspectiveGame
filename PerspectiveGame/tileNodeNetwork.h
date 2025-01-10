#pragma once

#include <iostream>
#include <vector>
#include <set>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "tileNavigation.h"
#include "tileNode.h"
#include "tile.h"
#include "cameraManager.h"

struct TileNodeNetwork {
private:
	std::vector<TileNode*> nodes;
	std::vector<int> freeNodeIndices;

	std::vector<Tile> tiles;
	std::vector<int> freeTileInfoIndices;

	Camera* p_camera;
	ForceManager* p_forceManager;

public: // Rendering:
	GLuint texID;
	GLuint positionNodeInfosBufferID;
	GLuint tilesBufferID;
	std::vector<glm::vec2> windowFrustum;

	std::vector<GPU_Tile> gpuTiles;
	std::vector<GPU_TileNodeInfo> gpuPositionNodeInfos;

	CenterNode CurrentNode;
	int currentMapIndex = 0;
	int currentNodeIndex = 4;

public:
	TileNodeNetwork(Camera* c, ForceManager* fm)
		: p_camera(c)
		, p_forceManager(fm)
	{
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 1; y++) {
				createTilePair(glm::vec3(float(x), float(y), 0), TILE_TYPE_XY);
			}
		}

		// Rendering:
		glGenBuffers(1, &positionNodeInfosBufferID);
		glGenBuffers(1, &tilesBufferID);
	}

	void update()
	{
		gpuPositionNodeInfos.clear();
		for (TileNode* p : nodes) {
			if (p == nullptr) 
				gpuPositionNodeInfos.push_back(GPU_TileNodeInfo());
			else
				gpuPositionNodeInfos.push_back(GPU_TileNodeInfo(*p));
		}

		gpuTiles.clear();
		for (Tile& i : tiles) {
			gpuTiles.push_back(GPU_Tile(i));
		}
	}

	int size() { return (int)nodes.size(); }

	void printSize()
	{
		int numCenterNodes = 0, numSideNodes = 0, numCornerNodes = 0;
		for (TileNode* n : nodes) {
			if (n == nullptr) continue;
			switch (n->type) {
			case NODE_TYPE_CENTER: numCenterNodes++; break;
			case NODE_TYPE_SIDE: numSideNodes++; break;
			case NODE_TYPE_CORNER: numCornerNodes++; break;
			}
		}
		std::cout
			<< "\nnum center nodes: " << numCenterNodes
			<< "\nnum side nodes: " << numSideNodes
			<< "\nnum corner nodes: " << numCornerNodes
			<< "\ntotal num nodes: " << numCenterNodes + numSideNodes + numCornerNodes
			<< std::endl;
	}

	void printCornerNodePositions()
	{
		for (TileNode* n : nodes) {
			if (n != nullptr && n->type == NODE_TYPE_CORNER)
				vechelp::println(n->position);
		}
	}

	TileNode* getNode(int index)
	{
		return nodes[index];
	}

	CenterNode* getNode(Tile* info)
	{
		return static_cast<CenterNode*>(nodes[info->centerNodeIndex]);
	}

	Tile* getTile(int index)
	{
		return &tiles[index];
	}

	int numTileInfos() { return (int)tiles.size(); }

	Tile* getTile(int tileInfoIndex, LocalDirection d)
	{
		return getTile(&tiles[tileInfoIndex], d);
	}

	// Gives the tile info of the neighbor tile in the direction of d.
	Tile* getTile(Tile* info, LocalDirection d)
	{
		CenterNode* node = static_cast<CenterNode*>(nodes[info->centerNodeIndex]);
		LocalDirection d2 = tnav::map(node->getNeighborMap(d), d);
		SideNode* sideNode = static_cast<SideNode*>(nodes[node->getNeighborIndex(d)]);
		CenterNode* neighborCenterNode = static_cast<CenterNode*>(nodes[sideNode->getNeighborIndex(d2)]);
		return &tiles[neighborCenterNode->getTileIndex()];
	}

	TileNode* getNeighbor(TileNode* node, LocalDirection toNeighbor)
	{
		return nodes[node->getNeighborIndex(toNeighbor)];
	}

	CenterNode* getSecondNeighbor(CenterNode* node, LocalDirection toNeighbor)
	{
		SideNode* sideNode = static_cast<SideNode*>(nodes[node->getNeighborIndex(toNeighbor)]);
		LocalDirection d = tnav::map(node->getNeighborMap(toNeighbor), toNeighbor);
		return static_cast<CenterNode*>(nodes[sideNode->getNeighborIndex(d)]);
	}

	MapType getSecondNeighborMap(CenterNode* node, LocalDirection toNeighbor)
	{
		SideNode* sideNode = static_cast<SideNode*>(nodes[node->getNeighborIndex(toNeighbor)]);
		LocalDirection d = tnav::map(node->getNeighborMap(toNeighbor), toNeighbor);
		return tnav::combine(node->getNeighborMap(toNeighbor), sideNode->getNeighborMap(d));
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
	int addNode(TileNodeType type, glm::vec3 pos, OrientationType orientation)
	{
		TileNode* node = nullptr;
		switch (type) {
		case NODE_TYPE_CENTER:
			node = new CenterNode();
			break;
		case NODE_TYPE_SIDE:
			node = (new SideNode(SIDE_NODE_TYPE_ERROR));
			break;
		case NODE_TYPE_CORNER:
			node = new CornerNode();
			break;
		case NODE_TYPE_DEGENERATE:
			node = new DegenerateCornerNode();
			break;
		default:
			return -1;
		}
		node->position = pos;
		node->orientation = orientation;
		node->forceListIndex = p_forceManager->addForce(LOCAL_DIRECTION_STATIC, node->index);

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
	int addCenterNode(glm::vec3 pos, TileType orientation)
	{
		return addNode(NODE_TYPE_CENTER, pos, orientation);
	}

	int addSideNode(glm::vec3 pos, OrientationType orientation)
	{
		return addNode(NODE_TYPE_SIDE, pos, orientation);
	}

	int addCornerNode(glm::vec3 pos, OrientationType orientation)
	{
		return addNode(NODE_TYPE_CORNER, pos, orientation);
	}

	int addDegenNode(glm::vec3 pos, int forceComponentIndexA1, int forceComponentIndexA2, int forceComponentIndexB1, int forceComponentIndexB2)
	{
		if (forceComponentIndexA1 == -1 || forceComponentIndexA2 == -1 || forceComponentIndexB1 == -1 || forceComponentIndexB2 == -1) {

		}

		int i = addNode(NODE_TYPE_DEGENERATE, pos, ORIENTATION_TYPE_ERROR);
		DegenerateCornerNode* degenNode = static_cast<DegenerateCornerNode*>(getNode(i));
		degenNode->componentPairIndices[0] = forceComponentIndexA1;
		degenNode->componentPairIndices[1] = forceComponentIndexA2;
		degenNode->componentPairIndices[2] = forceComponentIndexB1;
		degenNode->componentPairIndices[3] = forceComponentIndexB2;
		return i;
	}

	int addDegenNode(CornerNode& cornerNode)
	{
		DegenerateCornerNode* degenNode = new DegenerateCornerNode();
		degenNode->index = cornerNode.index;
		degenNode->position = cornerNode.position;
		degenNode->forceListIndex = cornerNode.forceListIndex;
		degenNode->numDegenComponents = 8;

		for (LocalDirection d : tnav::DIAGONAL_DIRECTION_SET) {
			LocalDirection toCorner = d;
			toCorner = tnav::map(cornerNode.getNeighborMap(toCorner), tnav::inverse(toCorner));
			int forceListIndex = degenNode->forceListIndex;
			
			switch (toCorner) {
			case LOCAL_DIRECTION_0_1:
				degenNode->componentPairIndices[0] = forceListIndex + 0;
				degenNode->componentPairIndices[1] = forceListIndex + 1; break;
			case LOCAL_DIRECTION_1_2:
				degenNode->componentPairIndices[0] = forceListIndex + 1;
				degenNode->componentPairIndices[1] = forceListIndex + 2; break;
			case LOCAL_DIRECTION_2_3:
				degenNode->componentPairIndices[0] = forceListIndex + 2;
				degenNode->componentPairIndices[1] = forceListIndex + 3; break;
			case LOCAL_DIRECTION_3_0:
				degenNode->componentPairIndices[0] = forceListIndex + 3;
				degenNode->componentPairIndices[1] = forceListIndex + 0; break;
			default:
				std::cout << "ERROR" << std::endl; return -1;
			}

			CenterNode* neighbor = static_cast<CenterNode*>(getNode(cornerNode.getNeighborIndex(d)));
			neighbor->setNeighborMap(toCorner, MAP_TYPE_ERROR);
		}

		delete nodes[cornerNode.index];
		nodes[degenNode->index] = degenNode;
		return degenNode->index;
	}

	void removeNode(int index)
	{
		if (index == nodes.size() - 1) {
			delete nodes[index];
			nodes.pop_back();
		}
		else {
			freeNodeIndices.push_back(index);
			delete nodes[index];
			nodes[index] = nullptr;
		}
	}

	void colorTile(int index)
	{
		switch (tiles[index].type) {
		case TILE_TYPE_XYF: tiles[index].color = glm::vec3(1.0f, 0.0f, 0.0f); break;
		case TILE_TYPE_XYB: tiles[index].color = glm::vec3(0.5f, 0.0f, 0.0f); break;
		case TILE_TYPE_XZF: tiles[index].color = glm::vec3(0.0f, 1.0f, 0.0f); break;
		case TILE_TYPE_XZB: tiles[index].color = glm::vec3(0.0f, 0.5f, 0.0f); break;
		case TILE_TYPE_YZF: tiles[index].color = glm::vec3(0.0f, 0.0f, 1.0f); break;
		case TILE_TYPE_YZB: tiles[index].color = glm::vec3(0.0f, 0.0f, 0.5f); break;
		}
	}

	void removeTile(int index)
	{
		tiles[index].wipe();
		freeTileInfoIndices.push_back(index);
	}

	// Adds an UNINITIALIZED tile info to the tileInfos list.  Returns a pointer to the added tileInfo.
	// frontIndex and backIndex are meant to be used as returns.
	void addTilePair(int frontCenterNodeIndex, int backCenterNodeIndex, SuperTileType type, int& frontInfoIndex, int& backInfoIndex)
	{
		if (freeTileInfoIndices.size() > 1) {
			frontInfoIndex = freeTileInfoIndices.back();
			tiles[frontInfoIndex].index = freeTileInfoIndices.back();
			freeTileInfoIndices.pop_back();

			backInfoIndex = freeTileInfoIndices.back();
			tiles[backInfoIndex].index = freeTileInfoIndices.back();
			freeTileInfoIndices.pop_back();
		}
		else if (freeTileInfoIndices.size() == 1) {
			frontInfoIndex = freeTileInfoIndices.back();
			tiles[frontInfoIndex].index = freeTileInfoIndices.back();
			freeTileInfoIndices.pop_back();

			tiles.push_back(Tile());
			backInfoIndex = (int)tiles.size() - 1;
			tiles[backInfoIndex].index = (int)tiles.size() - 1;
		}
		else {
			tiles.push_back(Tile());
			frontInfoIndex = (int)tiles.size() - 1;
			tiles[frontInfoIndex].index = (int)tiles.size() - 1;

			tiles.push_back(Tile());
			backInfoIndex = (int)tiles.size() - 1;
			tiles[backInfoIndex].index = (int)tiles.size() - 1;
		}

		tiles[frontInfoIndex].centerNodeIndex = frontCenterNodeIndex;
		tiles[frontInfoIndex].type = tnav::getFrontTileType(type);
		tiles[frontInfoIndex].siblingIndex = backInfoIndex;

		tiles[backInfoIndex].centerNodeIndex = backCenterNodeIndex;
		tiles[backInfoIndex].type = tnav::getBackTileType(type);
		tiles[backInfoIndex].siblingIndex = frontInfoIndex;

		colorTile(frontInfoIndex);
		colorTile(backInfoIndex);
	}

	// given a position in space, returns all the tiles connected to that point in the network.
	// currently assumes the given position is of a side node!  will error if given a position 
	// relating to a center of corner node.
	std::vector<Tile> getConnectedTiles(SideNode* node)
	{
		std::vector<Tile> connectedTiles;
		int sideNodeIndex = node->index;
		glm::vec3 pos = nodes[sideNodeIndex]->getPosition();

		for (TileNode* n : nodes) {
			if (n == nullptr || n->getPosition() != pos || n->getIndex() == sideNodeIndex)
				continue;

			SideNode* s = static_cast<SideNode*>(n);
			for (int i = 0; i < 2; i++) { // side nodes only have 2 neighbors
				if (s->getNeighborIndexDirect(i) == -1)
					continue;

				connectedTiles.push_back(
					tiles[
						static_cast<CenterNode*>(nodes[s->getNeighborIndexDirect(i)])->getTileIndex()
					]
				);
			}
		}

		return connectedTiles;
	}

	// returns how prioritized the connection between these two tiles should be.
	// 0 == high prio, 1 == medium, 2 = low, 3 = should not be connected in the first place!
	int getConnectionPrio(Tile* a, Tile* b)
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

	// returns a pointer to the center node of the newly created tile or nullptr if the tile could not be made.
	Tile* createTilePair(glm::vec3 pos, SuperTileType type)
	{
		// check if there is already a tile where we are trying to add one:
		// increment by 2s since we always add/remove 2 tiles at a time (front and back).
		for (int i = 0; i < tiles.size(); i += 2) {
			if (tiles[i].index == -1) continue;
			if (nodes[tiles[i].centerNodeIndex]->getPosition() == pos)
				return nullptr;
		}

		// create the new tile pair and center nodes:
		// indices are used because apparently pointers are unsafe if the vector resizes itself.
		TileType frontType = tnav::getTileType(type, true);
		TileType backType = tnav::getTileType(type, false);
		int newFrontNodeIndex = addCenterNode(pos, frontType);
		int newBackNodeIndex = addCenterNode(pos, backType);
		int newFrontTileIndex, newBackTileIndex;
		addTilePair(newFrontNodeIndex, newBackNodeIndex, type, newFrontTileIndex, newBackTileIndex);
		static_cast<CenterNode*>(nodes[newFrontNodeIndex])->setTileInfoIndex(newFrontTileIndex);
		static_cast<CenterNode*>(nodes[newBackNodeIndex])->setTileInfoIndex(newBackTileIndex);

		connectTilePair(&tiles[newFrontTileIndex]);
		reconnectCornerNodes(tiles[newFrontTileIndex]);
		reconnectCornerNodes(tiles[newBackTileIndex]);
		reconnectTile(tiles[newFrontTileIndex]);
		reconnectTile(tiles[newBackTileIndex]);

		return &tiles[newFrontTileIndex];
	}

	// Given a tile pair, will connect OR reconnect all the side nodes of that tile to the world.
	void connectTilePair(Tile* frontTile)
	{
		Tile* backTile = getTile(frontTile->siblingIndex);
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
			newSideNode->orientation = frontCenterNode->orientation; // Arbitrary but convenient later.
			newSideNode->setSideNodeType(
				(d == LOCAL_DIRECTION_0 || d == LOCAL_DIRECTION_2)
				? SIDE_NODE_TYPE_HORIZONTAL
				: SIDE_NODE_TYPE_VERTICAL);

			std::vector<Tile> linkedTiles = getConnectedTiles(newSideNode);
			if (linkedTiles.size() == 0) {
				connectTilePairSidesToThemselves(*frontTile, *newSideNode, d);
				continue;
			}

			// reconnect tiles based on heirarchy:
			for (Tile linkedTile : linkedTiles) {
				LocalDirection linkedTileDir;
				CenterNode* linkedTileCenterNode = static_cast<CenterNode*>(getNode(linkedTile.centerNodeIndex));

				for (LocalDirection dir : tnav::ORTHOGONAL_DIRECTION_SET) {
					if (nodes[linkedTileCenterNode->getNeighborIndex(dir)]->getPosition() == newSideNode->getPosition()) {
						linkedTileDir = dir;
						break;
					}
				}
				Tile* currentNeighbor = getTile(&linkedTile, linkedTileDir);
				CenterNode* currentNeighborCenterNode = getNode(currentNeighbor);

				int currentConnectionPrio = getConnectionPrio(&linkedTile, currentNeighbor);
				int newConnectionPrio1 = getConnectionPrio(&linkedTile, getTile(frontTile->index));
				int newConnectionPrio2 = getConnectionPrio(&linkedTile, getTile(backTile->index));
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
					linkedSideNode->orientation = backCenterNode->orientation;
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
			for (Tile& t : linkedTiles) {
				reconnectTile(tiles[t.index]);
			}
		}

		// add degenerate corners here:
		tryAddDegenNodes(*frontTile);
	}

	void connectTilePairSidesToThemselves(Tile& t, SideNode& newSideNode, LocalDirection toSibling)
	{
		CenterNode* centerNode = static_cast<CenterNode*>(getNode(t.centerNodeIndex));
		CenterNode* siblingNode = static_cast<CenterNode*>(getNode(getTile(t.siblingIndex)->centerNodeIndex));

		centerNode->setNeighborIndex(toSibling, newSideNode.getIndex());
		centerNode->setNeighborMap(toSibling, MAP_TYPE_IDENTITY);

		siblingNode->setNeighborIndex(toSibling, newSideNode.getIndex());
		siblingNode->setNeighborMap(toSibling, tnav::getNeighborMap(toSibling, toSibling));

		newSideNode.setNeighborIndex(tnav::inverse(toSibling), centerNode->getIndex());
		newSideNode.setNeighborMap(tnav::inverse(toSibling), MAP_TYPE_IDENTITY);
		newSideNode.setNeighborIndex(toSibling, siblingNode->getIndex());
		newSideNode.setNeighborMap(toSibling, tnav::getNeighborMap(toSibling, toSibling));
	}

	// adds any degen nodes to corners of a tile pair that are connected to nothing but the tile pair.
	void tryAddDegenNodes(Tile& t)
	{
		CenterNode* centerNode = static_cast<CenterNode*>(getNode(t.centerNodeIndex));
		CenterNode* siblingNode = static_cast<CenterNode*>(getNode(getTile(t.siblingIndex)->centerNodeIndex));

		for (LocalDirection component1 : tnav::ORTHOGONAL_DIRECTION_SET) {
			LocalDirection component2 = LocalDirection((component1 + 1) % 4);
			if (getSecondNeighbor(centerNode, component1) != siblingNode ||
				getSecondNeighbor(centerNode, component2) != siblingNode) {
				continue;
			}

			LocalDirection toCorner = tnav::combine(component1, component2);
			glm::vec3 degenPos = centerNode->position + tnav::getCenterToNeighborVec(t.type, toCorner) / 2.0f;
			int i = addDegenNode(degenPos,
								 centerNode->forceListIndex + component1, 
								 centerNode->forceListIndex + component2, 
								 siblingNode->forceListIndex + component1, 
								 siblingNode->forceListIndex + component2);

			centerNode->setNeighborIndex(toCorner, i);
			siblingNode->setNeighborIndex(toCorner, i);
		}
	}

	void reconnectTile(Tile& tile)
	{
		CenterNode* centerNode = static_cast<CenterNode*>(nodes[tile.centerNodeIndex]);
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			MapType m = getSecondNeighborMap(centerNode, d);
			CenterNode* neighborCenterNode = getSecondNeighbor(centerNode, d);
			tile.setNeighborMap(d, m);
			tile.setNeighborIndex(d, neighborCenterNode->getTileIndex());
		}
	}

	int addCornerNode(DegenerateCornerNode& degenNode)
	{
		if (degenNode.numDegenComponents != 8) return -1;

		// gather all the center nodes we need to connect:
		CenterNode* centerNodes[4] = {
			static_cast<CenterNode*>(nodes[p_forceManager->getNodeIndex(degenNode.componentPairIndices[0])]),
			static_cast<CenterNode*>(nodes[p_forceManager->getNodeIndex(degenNode.componentPairIndices[2])]),
			static_cast<CenterNode*>(nodes[p_forceManager->getNodeIndex(degenNode.componentPairIndices[4])]),
			static_cast<CenterNode*>(nodes[p_forceManager->getNodeIndex(degenNode.componentPairIndices[6])]),
		};

		CornerNode* newCornerNode = new CornerNode(); 
		newCornerNode->index = degenNode.index;
		newCornerNode->position = degenNode.position;
		newCornerNode->orientation = centerNodes[0]->orientation; // arbitrary
		newCornerNode->forceListIndex = degenNode.forceListIndex;

		// find the direction from the initial center node to the new corner node:
		const LocalDirection* comps = nullptr;
		for (LocalDirection d : tnav::DIAGONAL_DIRECTION_SET) {
			TileType type = getTile(centerNodes[0]->getTileIndex())->type;
			glm::vec3 pos = centerNodes[0]->position + tnav::getCenterToNeighborVec(type, d) / 2.0f;
			if (pos == newCornerNode->position) {
				comps = tnav::getAlignmentComponents(d);
			}
		}
		LocalDirection neighborToCornerComponents[2] = { comps[0], comps[1] };

		//int componenti = 0;
		//for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
		//	CenterNode* neighbor = getSecondNeighbor(centerNodes[0], d);
		//	
		//	for (int i = 1; i < 4; i++) {
		//		if (neighbor == centerNodes[i]) {
		//			if (neighborToCornerComponents[0] != LOCAL_DIRECTION_ERROR &&
		//				neighbor == getSecondNeighbor(centerNodes[0], neighborToCornerComponents[0]))
		//				continue; // both sides connecting to a sibling does not a connection make, in this case.
		//			if (componenti > 1) continue;
		//
		//			neighborToCornerComponents[componenti++] = d;
		//			break;
		//		}
		//	}
		//}

		// connect up the initial center node and the new corner node:
		LocalDirection neighborToCorner = tnav::combine(neighborToCornerComponents[0], neighborToCornerComponents[1]);
		centerNodes[0]->setNeighborIndex(neighborToCorner, newCornerNode->index);
		centerNodes[0]->setNeighborMap(neighborToCorner, MAP_TYPE_IDENTITY);
		newCornerNode->setNeighborIndex(tnav::inverse(neighborToCorner), centerNodes[0]->index);
		newCornerNode->setNeighborMap(tnav::inverse(neighborToCorner), MAP_TYPE_IDENTITY);

		// move to the other center nodes in centerNodes[] and connect them to the new corner node:
		CenterNode* neighbor = centerNodes[0];
		MapType M = MAP_TYPE_IDENTITY;
		for (int i = 0; i < 3; i++) {
			MapType m = getSecondNeighborMap(neighbor, neighborToCornerComponents[0]);
			M = tnav::combine(M, m);
			neighbor = getSecondNeighbor(neighbor, neighborToCornerComponents[0]);
			LocalDirection temp = neighborToCornerComponents[0];
			neighborToCornerComponents[0] = tnav::map(m, neighborToCornerComponents[1]);
			neighborToCornerComponents[1] = tnav::map(m, tnav::inverse(temp));
			neighborToCorner = tnav::combine(neighborToCornerComponents[0], neighborToCornerComponents[1]);

			neighbor->setNeighborIndex(neighborToCorner, newCornerNode->index);
			neighbor->setNeighborMap(neighborToCorner, tnav::inverse(M));

			LocalDirection cornerToCenterNode = tnav::map(tnav::inverse(M), tnav::inverse(neighborToCorner));
			newCornerNode->setNeighborIndex(cornerToCenterNode, neighbor->index);
			newCornerNode->setNeighborMap(cornerToCenterNode, M);
		}
		
		delete nodes[degenNode.index];
		nodes[newCornerNode->index] = newCornerNode;

		return newCornerNode->index;
	}

	void reconnectCornerNodes(Tile& tile)
	{
		using namespace tnav;

		CenterNode* centerNode = static_cast<CenterNode*>(getNode(tile.centerNodeIndex));
		CenterNode* siblingNode = static_cast<CenterNode*>(getNode(getTile(tile.siblingIndex)->centerNodeIndex));

		for (LocalDirection d : DIAGONAL_DIRECTION_SET) {
			// centerNodes with no diagonal connection must connect to at least on tile in a 
			// component of the diagonal direction, making that connection either to a degenerate 
			// node or a corner node, depending on how many connections the degenerate node has.
			bool needsConnecting = centerNode->getNeighborIndex(d) == -1;
			if (!needsConnecting)
				continue;

			const LocalDirection* components = getAlignmentComponents(d);
			LocalDirection toN = map(getSecondNeighborMap(centerNode, components[0]), combine(inverse(components[0]), components[1]));
			CenterNode* neighbor = getSecondNeighbor(centerNode, components[0]);
			if (neighbor == siblingNode) {
				toN = map(getSecondNeighborMap(centerNode, components[1]), combine(inverse(components[1]), components[0]));
				neighbor = getSecondNeighbor(centerNode, components[1]);
			}
			TileNode* n = getNode(neighbor->getNeighborIndex(toN));
			
			CornerNode* c;
			DegenerateCornerNode* degen;
			switch (n->type) {
			case NODE_TYPE_CORNER:
				c = static_cast<CornerNode*>(n);
				degen = static_cast<DegenerateCornerNode*>(getNode(addDegenNode(*c))); 
				degen->addDegenPair(centerNode->forceListIndex + components[0], centerNode->forceListIndex + components[1]);
				break;

			case NODE_TYPE_DEGENERATE:
				degen = static_cast<DegenerateCornerNode*>(n);
				degen->addDegenPair(centerNode->forceListIndex + components[0], centerNode->forceListIndex + components[1]);
				if (degen->numDegenComponents == 8) { // degen is connected to exactly 4 tiles, making it no longer degenerate
					addCornerNode(*static_cast<DegenerateCornerNode*>(n));
				}
				break;
			}
		}



		//CenterNode* frontNode = static_cast<CenterNode*>(getNode(frontTileInfo->centerNodeIndex));

		//// remove all corner nodes associated with this tile pair
		//std::set<int> deleteList;
		//for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
		//	CenterNode* neighborCenterNode = getSecondNeighbor(frontNode, d);
		//	LocalDirection D = tnav::map(getSecondNeighborMap(frontNode, d), tnav::inverse(d));
		//	LocalDirection diagonal1 = tnav::combine(D, LocalDirection((D + 1) % 4));
		//	LocalDirection diagonal2 = tnav::combine(D, LocalDirection((D + 3) % 4));

		//	deleteList.insert(neighborCenterNode->getNeighborIndex(diagonal1));
		//	deleteList.insert(neighborCenterNode->getNeighborIndex(diagonal2));
		//}
		//deleteList.erase(-1); // gets rid of all invalid indices.

		//// removes all connections to the corner nodes associated with this corner pair:
		//for (int i : deleteList) {
		//	CornerNode* c = static_cast<CornerNode*>(getNode(i));
		//	// remove centerNode -> this corner node connections:
		//	for (LocalDirection d : tnav::DIAGONAL_DIRECTION_SET) { // ALL corner nodes MUST have 4 connections definitially
		//		LocalDirection D = tnav::map(c->getNeighborMap(d), tnav::inverse(d));
		//		int index = c->getNeighborIndex(d);
		//		if (index == -1) continue;
		//		CenterNode* centerNode = static_cast<CenterNode*>(getNode(index));
		//		centerNode->setNeighborIndex(D, -1);
		//		centerNode->setNeighborMap(D, MAP_TYPE_ERROR);
		//	}
		//	removeNode(c->index);
		//}

		//// cycle through all 4 possible corner nodes and re-add them if they are valid
		//for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
		//	// could be that the deletion earlier resized the underlying node vector:
		//	frontNode = static_cast<CenterNode*>(getNode(frontTileInfo->centerNodeIndex));
		//	LocalDirection
		//		centralNorth = d,
		//		centralEast = LocalDirection((d + 1) % 4),
		//		centralSouth = LocalDirection((d + 2) % 4),
		//		centralWest = LocalDirection((d + 3) % 4),
		//		centralNorthEast = tnav::combine(centralNorth, centralEast),
		//		centralNorthWest = tnav::combine(centralNorth, centralWest),
		//		centralSouthEast = tnav::combine(centralSouth, centralEast),
		//		centralSouthWest = tnav::combine(centralSouth, centralWest);
		//	CenterNode
		//		* northNode = static_cast<CenterNode*>(getSecondNeighbor(frontNode, centralNorth)),
		//		* eastNode = static_cast<CenterNode*>(getSecondNeighbor(frontNode, centralEast));
		//	MapType
		//		goNorth = getSecondNeighborMap(frontNode, centralNorth),
		//		goEast = getSecondNeighborMap(frontNode, centralEast);
		//	LocalDirection
		//		toNorthEast = tnav::map(goNorth, centralEast),
		//		toEastNorth = tnav::map(goEast, centralNorth);
		//	CenterNode
		//		* northEastNode = static_cast<CenterNode*>(getSecondNeighbor(northNode, toNorthEast)),
		//		* eastNorthNode = static_cast<CenterNode*>(getSecondNeighbor(eastNode, toEastNorth));

		//	if (northEastNode != eastNorthNode || northEastNode == frontNode) 
		//		continue; // this corner is degenerate!
		//	
		//	MapType
		//		northToNorthEast = getSecondNeighborMap(northNode, toNorthEast),
		//		goNorthEast = tnav::combine(goNorth, northToNorthEast);

		//	glm::vec3 newCornerNodePosition = frontNode->position
		//		+ tnav::globalDirToVec3(tnav::localToGlobalDir(frontNode->orientation, centralNorth)) / 2.0f
		//		+ tnav::globalDirToVec3(tnav::localToGlobalDir(frontNode->orientation, centralEast)) / 2.0f;

		//	int northNodeIndex = northNode->index,
		//		eastNodeIndex = eastNode->index,
		//		northEastNodeIndex = northEastNode->index,
		//		newCornerNodeIndex = addCornerNode(newCornerNodePosition, frontNode->orientation);

		//	// addNode() may have resized the underlying vector that keeps nodes, making the pointers
		//	// error-prone.  just re-find here to avoid potential issues:
		//	northNode = static_cast<CenterNode*>(getNode(northNodeIndex));
		//	eastNode = static_cast<CenterNode*>(getNode(eastNodeIndex));
		//	northEastNode = static_cast<CenterNode*>(getNode(northEastNodeIndex));

		//	// stitch everything together finally:
		//	CornerNode* newCornerNode = static_cast<CornerNode*>(getNode(newCornerNodeIndex));
		//	newCornerNode->setNeighborIndex(centralSouthWest, frontNode->index);
		//	newCornerNode->setNeighborMap(centralSouthWest, MAP_TYPE_IDENTITY);
		//	newCornerNode->setNeighborIndex(centralNorthWest, northNode->index);
		//	newCornerNode->setNeighborMap(centralNorthWest, goNorth);
		//	newCornerNode->setNeighborIndex(centralSouthEast, eastNode->index);
		//	newCornerNode->setNeighborMap(centralSouthEast, goEast);
		//	newCornerNode->setNeighborIndex(centralNorthEast, northEastNode->index);
		//	newCornerNode->setNeighborMap(centralNorthEast, goNorthEast);

		//	frontNode->setNeighborIndex(centralNorthEast, newCornerNode->index);
		//	frontNode->setNeighborMap(centralNorthEast, MAP_TYPE_IDENTITY);

		//	LocalDirection toCornerNode = tnav::map(goNorth, centralSouthEast);
		//	northNode->setNeighborIndex(toCornerNode, newCornerNode->index);
		//	northNode->setNeighborMap(toCornerNode, tnav::inverse(newCornerNode->getNeighborMap(centralNorthWest)));

		//	toCornerNode = tnav::map(goEast, centralNorthWest);
		//	eastNode->setNeighborIndex(toCornerNode, newCornerNode->index);
		//	eastNode->setNeighborMap(toCornerNode, tnav::inverse(newCornerNode->getNeighborMap(centralSouthEast)));

		//	toCornerNode = tnav::map(goNorthEast, centralSouthWest);
		//	northEastNode->setNeighborIndex(toCornerNode, newCornerNode->index);
		//	northEastNode->setNeighborMap(toCornerNode, tnav::inverse(newCornerNode->getNeighborMap(centralNorthEast)));
		//}
	}

	void removeTilePair(int tileInfoIndex) { removeTilePair(&tiles[tileInfoIndex]); }

	void removeTilePair(Tile* t)
	{
		using namespace tnav;

		if (t == nullptr) return;

		Tile* siblingTile = getTile(t->siblingIndex);
		int centerNodeIndex = (t->centerNodeIndex);
		int sibCenterNodeIndex = (siblingTile->centerNodeIndex);

		std::set<int> affectedCenterNodeIndices; // for managing degenerate corners later

		// reconnect edges:
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			Tile
				* neighborTile1 = getTile(t, d),
				* neighborTile2 = getTile(t->siblingIndex, d);
			if (neighborTile1 == siblingTile) {
				// edges that dont connect to anything can be simply removed, 
				// as no reconnection is necessary:
				removeNode(getNode(centerNodeIndex)->getNeighborIndex(d));
				continue;
			}

			affectedCenterNodeIndices.insert(neighborTile1->centerNodeIndex);
			affectedCenterNodeIndices.insert(neighborTile2->centerNodeIndex);

			CenterNode* centerNode1 = static_cast<CenterNode*>(getNode(centerNodeIndex));
			SideNode* sideNode1 = static_cast<SideNode*>(getNode(centerNode1->getNeighborIndex(d)));
			CenterNode* neighborCenterNode1 = static_cast<CenterNode*>(getNode(neighborTile1->centerNodeIndex));
			CenterNode* centerNode2 = static_cast<CenterNode*>(getNode(sibCenterNodeIndex));
			SideNode* sideNode2 = static_cast<SideNode*>(getNode(centerNode2->getNeighborIndex(d)));
			CenterNode* neighborCenterNode2 = static_cast<CenterNode*>(getNode(neighborTile2->centerNodeIndex));
			
			LocalDirection 
				n1ToNew = mapToSecondNeighbor(centerNode1, d, inverse(d)),
				n2ToNew = mapToSecondNeighbor(centerNode2, d, inverse(d)),
				newToN1 = inverse(n1ToNew),
				newToN2 = n1ToNew;
			
			MapType 
				newToN2Map = getNeighborMap(newToN2, n2ToNew),
				n2ToNewMap = inverse(newToN2Map);
			
			// While we could edit the existing nodes, its conceptually simpler to just make a fresh one:
			SideNode* newNode = static_cast<SideNode*>(getNode(
				addSideNode(sideNode1->getPosition(), 
							neighborCenterNode1->orientation))); // makes sure the transition from center node type -> new side node type is possible
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

		for (LocalDirection d : tnav::DIAGONAL_DIRECTION_SET) {
			CenterNode* c = static_cast<CenterNode*>(getNode(centerNodeIndex));
			if (c->getNeighborIndex(d) != -1) {
				LocalDirection D = tnav::map(c->getNeighborMap(d), tnav::inverse(d));
				CornerNode* C = static_cast<CornerNode*>(getNode(c->getNeighborIndex(d)));
				C->setNeighborIndex(D, -1);
				C->setNeighborMap(D, MAP_TYPE_ERROR);
			}

			c = static_cast<CenterNode*>(getNode(sibCenterNodeIndex));
			if (c->getNeighborIndex(d) != -1) {
				LocalDirection D = tnav::map(c->getNeighborMap(d), tnav::inverse(d));
				CornerNode* C = static_cast<CornerNode*>(getNode(c->getNeighborIndex(d)));
				C->setNeighborIndex(D, -1);
				C->setNeighborMap(D, MAP_TYPE_ERROR);
			}
		}
		
		removeTile(static_cast<CenterNode*>(getNode(centerNodeIndex))->getTileIndex());
		removeTile(static_cast<CenterNode*>(getNode(sibCenterNodeIndex))->getTileIndex());
		removeNode(centerNodeIndex);
		removeNode(sibCenterNodeIndex);

		for (int i : affectedCenterNodeIndices) {
			reconnectCornerNodes(*getTile(static_cast<CenterNode*>(getNode(i))->getTileIndex()));
		}
	}
};