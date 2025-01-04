#pragma once

#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tileNavigation.h"

// Connecitons    Nodes      Types
// ___________
// |\  /|\  /|   * * * *   L S L S L
// | \/ | \/ |   * * * *   S M S M S
// | /\ | /\ |   * * * *   L S L S L
// |/__\|/__\|   * * * *   S M S M S
// |\  /|\  /|   * * * *   L S L S L
// | \/ | \/ |
// | /\ | /\ |
// |/__\|/__\|
// 
//

enum TileNodeType {
	TILE_NODE_TYPE_SML_INNR_HZTL, // connects to: 1 tile  , 2 LrgNodes
	TILE_NODE_TYPE_SML_INNR_VTCL, // connects to: 1 tile  , 2 LrgNodes
	TILE_NODE_TYPE_SML_EDGE_HZTL, // connects to: 2 tiles , 2 LrgNodes
	TILE_NODE_TYPE_SML_EDGE_VTCL, // connects to: 2 tiles , 2 LrgNodes
	TILE_NODE_TYPE_MED,			  // connects to: 1 tile  , 4 LrgNodes
	TILE_NODE_TYPE_LRG_CNTR,	  // connects to: 1 tile  , 4 MedNodes , 4 SmlNodes
	TILE_NODE_TYPE_LRG_CRNR,	  // connects to: 2 tiles , 4 MedNodes , 4 SmlNodes
	TILE_NODE_TYPE_LRG_EDGE,	  // connects to: 4 tiles , 4 MedNodes , 4 SmlNodes
	TILE_NODE_TYPE_ERR,				
};

class TileNode {
private:
	int numNeighbors;
	std::vector<int> neighborIndices;
	std::vector<MapType> neighborMaps;

	int numTiles;
	std::vector<int> tileInfoIndices;
	std::vector<MapType> tileMaps;

public:
	int index;
	TileNodeType type;
	BasisType basis;
	glm::vec3 pos;

	TileNode()
		: index(-1)
		, type (TILE_NODE_TYPE_ERR)
		, basis (BASIS_TYPE_ERROR)
		, pos(glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX))
	{
		numNeighbors = -1;
		numTiles = -1;
		//neighborIndices = nullptr;
		//neighborMaps = nullptr;
		//tileInfoIndices = nullptr;
		//tileMaps = nullptr;
	}

	TileNode(int i, TileNodeType t, BasisType b, glm::vec3 p) 
		: index(i)
		, type(t) 
		, basis(b)
		, pos(p)
	{
		resizeArrays();
	}
	
	TileNode(const TileNode& other) : index(other.index), type(other.type), basis(other.basis), pos(other.pos), numNeighbors(other.numNeighbors), numTiles(other.numTiles)
	{
		neighborIndices.resize(other.numNeighbors);
		neighborMaps.resize(other.numNeighbors);
		tileInfoIndices.resize(other.numTiles);
		tileMaps.resize(other.numTiles);

		for (int i = 0; i < other.numNeighbors; i++) {
			neighborIndices[i] = other.neighborIndices[i];
			neighborMaps[i] = other.neighborMaps[i];
		}
		for (int i = 0; i < other.numTiles; i++) {
			tileInfoIndices[i] = other.tileInfoIndices[i];
			tileMaps[i] = other.tileMaps[i];
		}

		/*if (other.neighborIndices) {
			neighborIndices = new int[numNeighbors];
			std::copy(other.neighborIndices, other.neighborIndices + numNeighbors, neighborIndices);
		}
		else { 
			neighborIndices = nullptr; 
		}

		if (other.neighborMaps) {
			neighborMaps = new MapType[numNeighbors];
			std::copy(other.neighborMaps, other.neighborMaps + numNeighbors, neighborMaps);
		}
		else {
			neighborMaps = nullptr;
		}

		if (other.tileInfoIndices) {
			tileInfoIndices = new int[numTiles];
			std::copy(other.tileInfoIndices, other.tileInfoIndices + numTiles, tileInfoIndices);
		}
		else {
			tileInfoIndices = nullptr;
		}

		if (other.tileMaps) { 
			tileMaps = new MapType[numTiles]; 
			std::copy(other.tileMaps, other.tileMaps + numTiles, tileMaps); 
		}
		else {
			tileMaps = nullptr;
		}*/
	}

	void deleteArrays()
	{
		//delete[] neighborIndices;
		//delete[] neighborMaps;
		//delete[] tileInfoIndices;
		//delete[] tileMaps;
	}

	void resizeArrays()
	{
		switch (type) {
		case TILE_NODE_TYPE_SML_INNR_HZTL: numNeighbors = 2; numTiles = 1; break;
		case TILE_NODE_TYPE_SML_INNR_VTCL: numNeighbors = 2; numTiles = 1; break;
		case TILE_NODE_TYPE_SML_EDGE_HZTL: numNeighbors = 2; numTiles = 2; break;
		case TILE_NODE_TYPE_SML_EDGE_VTCL: numNeighbors = 2; numTiles = 2; break;
		case TILE_NODE_TYPE_MED:		   numNeighbors = 4; numTiles = 1; break;
		case TILE_NODE_TYPE_LRG_CNTR:	   numNeighbors = 8; numTiles = 1; break;
		case TILE_NODE_TYPE_LRG_EDGE:	   numNeighbors = 8; numTiles = 2; break;
		case TILE_NODE_TYPE_LRG_CRNR:	   numNeighbors = 8; numTiles = 4; break;
		default: /* TILE_NODE_TYPE_ERR */  numNeighbors = -1; numTiles = -1; break;
		}

		if (type == TILE_NODE_TYPE_ERR) {
			//neighborIndices = nullptr;
			//neighborMaps = nullptr;
			//tileInfoIndices = nullptr;
			//tileMaps = nullptr;
		}
		else {
			//neighborIndices = new int[numNeighbors]{};
			//neighborMaps = new MapType[numNeighbors]{};

			//tileInfoIndices = new int[numTiles]{};
			//tileMaps = new MapType[numTiles]{};

			neighborIndices.resize(numNeighbors);
			neighborMaps.resize(numNeighbors);
			tileInfoIndices.resize(numTiles);
			tileMaps.resize(numTiles);

			for (int i = 0; i < numNeighbors; i++) {
				neighborIndices[i] = -1;
				neighborMaps[i] = MAP_TYPE_ERROR;
			}
			for (int i = 0; i < numTiles; i++) {
				tileInfoIndices[i] = -1;
				tileMaps[i] = MAP_TYPE_ERROR;
			}
		}
	}

	virtual ~TileNode()
	{
		deleteArrays();
	}

	TileNode& operator=(const TileNode& other)
	{
		if (this == &other) return *this;

		deleteArrays();

		index = other.index; type = other.type;
		basis = other.basis; pos = other.pos;
		numNeighbors = other.numNeighbors;
		numTiles = other.numTiles;

		neighborIndices.resize(other.numNeighbors);
		neighborMaps.resize(other.numNeighbors);
		tileInfoIndices.resize(other.numTiles);
		tileMaps.resize(other.numTiles);

		for (int i = 0; i < other.numNeighbors; i++) {
			neighborIndices[i] = other.neighborIndices[i];
			neighborMaps[i] = other.neighborMaps[i];
		}
		for (int i = 0; i < other.numTiles; i++) {
			tileInfoIndices[i] = other.tileInfoIndices[i];
			tileMaps[i] = other.tileMaps[i];
		}

		/*if (other.neighborIndices) {
			neighborIndices = new int[numNeighbors];
			std::copy(other.neighborIndices, other.neighborIndices + numNeighbors, neighborIndices);
		}
		else {
			neighborIndices = nullptr;
		}
		if (other.neighborMaps) {
			neighborMaps = new MapType[numNeighbors];
			std::copy(other.neighborMaps, other.neighborMaps + numNeighbors, neighborMaps);
		}
		else {
			neighborMaps = nullptr;
		}
		if (other.tileInfoIndices) {
			tileInfoIndices = new int[numTiles];
			std::copy(other.tileInfoIndices, other.tileInfoIndices + numTiles, tileInfoIndices);
		}
		else {
			tileInfoIndices = nullptr;
		}
		if (other.tileMaps) {
			tileMaps = new MapType[numTiles]; 
			std::copy(other.tileMaps, other.tileMaps + numTiles, tileMaps);
		}
		else {
			tileMaps = nullptr;
		}*/
		
		return *this;
	}

	int dirToNeighborIndexMap(LocalDirection dir)
	{
		switch (type) {
		case TILE_NODE_TYPE_SML_EDGE_VTCL:
		case TILE_NODE_TYPE_SML_INNR_VTCL:
			switch (dir) {
			case LOCAL_DIRECTION_1: return 0;
			case LOCAL_DIRECTION_3: return 1;
			default: return -1;
			}

		case TILE_NODE_TYPE_SML_EDGE_HZTL:
		case TILE_NODE_TYPE_SML_INNR_HZTL:
			switch (dir) {
			case LOCAL_DIRECTION_0: return 0;
			case LOCAL_DIRECTION_2: return 1;
			default: return -1;
			}	
		case TILE_NODE_TYPE_MED:
			// medium nodes only have diagonal neighbors.  
			// orthogonal queries will error:
			switch (dir) {
			case LOCAL_DIRECTION_0_1: return 0;
			case LOCAL_DIRECTION_1_2: return 1;
			case LOCAL_DIRECTION_2_3: return 2;
			case LOCAL_DIRECTION_3_0: return 3;
			default: return -1;
			}
		case TILE_NODE_TYPE_LRG_CNTR:
		case TILE_NODE_TYPE_LRG_EDGE:
		case TILE_NODE_TYPE_LRG_CRNR: return (int)dir; // large nodes have neighbors in all directions
		default: return -1; 
		}
	}

	int getIndex() { return index; }
	void setIndex(int i) { index = i; }

	glm::vec3 getPosition() { return pos; }
	void setPosition(glm::vec3 pos) { pos = pos; }

	int getNeighborIndex(LocalDirection dir) { return neighborIndices[dirToNeighborIndexMap(dir)]; }
	void setNeighborIndex(LocalDirection dir, int i) { neighborIndices[dirToNeighborIndexMap(dir)] = i; }

	MapType getNeighborMap(LocalDirection dir) { return neighborMaps[dirToNeighborIndexMap(dir)]; }
	void setNeighborMap(LocalDirection dir, MapType map) { neighborMaps[dirToNeighborIndexMap(dir)] = map; }

	LocalAlignment mapToNeighbor(LocalAlignment align, LocalDirection dir) { return tnav::map(neighborMaps[dirToNeighborIndexMap(dir)], align); }

	//int* getTileInfoIndices() { return tileInfoIndices; }
	int getTileInfoIndex(int i) { return tileInfoIndices[i]; }
	void setTileInfoIndex(int i, int tileI) { tileInfoIndices[i] = tileI; }

	void wipe()
	{
		index = -1;
		type = TILE_NODE_TYPE_ERR;
		basis = BASIS_TYPE_ERROR;
		pos = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		numNeighbors = -1;
		numTiles = -1;

		neighborIndices.clear();
		neighborMaps.clear();
		tileInfoIndices.clear();
		tileMaps.clear();

		//delete neighborIndices;
		//delete neighborMaps;
		//delete tileInfoIndices;
		//delete tileMaps;
		//neighborIndices = nullptr;
		//neighborMaps = nullptr;
		//tileInfoIndices = nullptr;
		//tileMaps = nullptr;
	}
};

struct TileInfo {
public:
	TileType type;
	int index;
	BasisType basis;
	int siblingIndex;
	glm::vec3 color;
	int centerNodeIndex;

private:
	int neighborIndices[4];
	MapType neighborMaps[4];
	glm::vec2 texCoords[4];

public:
	TileInfo(TileType type, int index, BasisType basis, int siblingIndex, int centerNodeIndex, glm::vec3 color) 
		: type(type)
		, index(index)
		, basis(basis)
		, siblingIndex(siblingIndex)
		, centerNodeIndex(centerNodeIndex)
		, color(color)
	{
		texCoords[0] = glm::vec2(1, 1);
		texCoords[1] = glm::vec2(1, 0);
		texCoords[2] = glm::vec2(0, 0);
		texCoords[3] = glm::vec2(0, 1);

		for (int i = 0; i < 4; i++) {
			neighborIndices[i] = -1;
			neighborMaps[i] = MAP_TYPE_ERROR;
		}
	}

	TileInfo()
	{
		wipe();
		texCoords[0] = glm::vec2(1, 1);
		texCoords[1] = glm::vec2(1, 0);
		texCoords[2] = glm::vec2(0, 0);
		texCoords[3] = glm::vec2(0, 1);
	}

	void wipe()
	{
		type = TILE_TYPE_ERROR;
		index = -1;
		basis = BASIS_TYPE_ERROR;
		siblingIndex = -1;
		centerNodeIndex = -1;
		color = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		for (int i = 0; i < 4; i++) {
			neighborIndices[i] = -1;
			neighborMaps[i] = MAP_TYPE_ERROR;
		}
	}

	TileType getType() { return type; }
	void setType(TileType t) { type = t; }

	int getIndex() { return index; }
	void setIndex(int i) { index = i; }

	int getSiblingIndex() { return siblingIndex; }
	void setSiblingIndex(int i) { siblingIndex = i; }

	int getCenterNodeIndex() { return centerNodeIndex; }
	void setCenterNodeIndex(int i) { centerNodeIndex = i; }

	int getNeighborIndex(LocalDirection d) { return neighborIndices[d]; }
	void setNighborIndex(LocalDirection d, int i) { neighborIndices[d] = i; }

	MapType getNeighborMap(LocalDirection d) { return neighborMaps[d]; }
	void setNeighborMap(LocalDirection d, MapType m) { neighborMaps[d] = m; }

	glm::vec3 getColor() { return color; }
	void setColor(glm::vec3 c) { color = c; }

	glm::vec2 getTexCoord(int i) { return texCoords[i]; }
	void setTexCoord(int i, glm::vec2 p) { texCoords[i] = p; }
};

struct alignas(8) GPU_EntityInfo {
	int index, heading;
};

struct alignas(32) GPU_TileNodeInfo {
	alignas(32) int neighborIndices[8];
	alignas(32) int neighborMaps[8];

	alignas(4) int index;
	alignas(4) int tileInfoIndex;
	alignas(4) int padding[6];

	GPU_TileNodeInfo()
	{
		index = -1;
		tileInfoIndex = -1;
		for (int i = 0; i < 8; i++) {
			neighborIndices[i] = -1;
			neighborMaps[i] = -1;
		}
	}

	GPU_TileNodeInfo(TileNode& node)
	{
		/*index = node.getIndex();

		SmlNode* sideNode;
		MedNode* cornerNode;

		switch (node.type) {
		case NODE_TYPE_CENTER:
			tileInfoIndex = dynamic_cast<LrgNode&>(node).getTileInfoIndex();

			for (LocalDirection d : tnav::DIRECTION_SET) {
				neighborIndices[d] = node.getNeighborIndex(d);
				neighborMaps[d] = node.getNeighborMap(d);
			}
			break;
		case NODE_TYPE_SIDE:
			tileInfoIndex = -1;

			for (LocalDirection d : tnav::DIRECTION_SET) {
				neighborIndices[d] = -1;
				neighborMaps[d] = -1;
			}
			
			sideNode = static_cast<SmlNode*>(&node);
			switch (sideNode->getSideNodeType()) {
			case SIDE_NODE_TYPE_HORIZONTAL:
				neighborIndices[LOCAL_DIRECTION_0] = sideNode->getNeighborIndex(LOCAL_DIRECTION_0);
				neighborIndices[LOCAL_DIRECTION_2] = sideNode->getNeighborIndex(LOCAL_DIRECTION_2);

				neighborMaps[LOCAL_DIRECTION_0] = sideNode->getNeighborMap(LOCAL_DIRECTION_0);
				neighborMaps[LOCAL_DIRECTION_2] = sideNode->getNeighborMap(LOCAL_DIRECTION_2);
				break;
			case SIDE_NODE_TYPE_VERTICAL:
				neighborIndices[LOCAL_DIRECTION_1] = sideNode->getNeighborIndex(LOCAL_DIRECTION_1);
				neighborIndices[LOCAL_DIRECTION_3] = sideNode->getNeighborIndex(LOCAL_DIRECTION_3);

				neighborMaps[LOCAL_DIRECTION_1] = sideNode->getNeighborMap(LOCAL_DIRECTION_1);
				neighborMaps[LOCAL_DIRECTION_3] = sideNode->getNeighborMap(LOCAL_DIRECTION_3);
				break;
			}
			break;

		case NODE_TYPE_CORNER:
			tileInfoIndex = -1;

			for (LocalDirection d : tnav::DIRECTION_SET) {
				neighborIndices[d] = -1;
				neighborMaps[d] = -1;
			}

			cornerNode = static_cast<MedNode*>(&node);
			neighborIndices[LOCAL_DIRECTION_0_1] = cornerNode->getNeighborIndex(LOCAL_DIRECTION_0_1);
			neighborIndices[LOCAL_DIRECTION_1_2] = cornerNode->getNeighborIndex(LOCAL_DIRECTION_1_2);
			neighborIndices[LOCAL_DIRECTION_2_3] = cornerNode->getNeighborIndex(LOCAL_DIRECTION_2_3);
			neighborIndices[LOCAL_DIRECTION_3_0] = cornerNode->getNeighborIndex(LOCAL_DIRECTION_3_0);

			neighborMaps[LOCAL_DIRECTION_0_1] = cornerNode->getNeighborMap(LOCAL_DIRECTION_0_1);
			neighborMaps[LOCAL_DIRECTION_1_2] = cornerNode->getNeighborMap(LOCAL_DIRECTION_1_2);
			neighborMaps[LOCAL_DIRECTION_2_3] = cornerNode->getNeighborMap(LOCAL_DIRECTION_2_3);
			neighborMaps[LOCAL_DIRECTION_3_0] = cornerNode->getNeighborMap(LOCAL_DIRECTION_3_0);
			break;
		}*/
	}
};

struct alignas(32) GPU_TileInfo {
	alignas(32) glm::vec2 texCoords[4];
	
	alignas(16) int neighborIndices[4];
	alignas(16) int neighborMaps[4];
	
	alignas(16) int entityIndices[4];
	alignas(16) glm::vec4 color;

	alignas(4) int numEntities;
	alignas(4) int index;
	alignas(4) int padding[6];

	GPU_TileInfo(TileInfo& info)
	{
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			neighborIndices[d] = info.getNeighborIndex(d);
			neighborMaps[d] = info.getNeighborMap(d);
			texCoords[d] = info.getTexCoord(d);
			entityIndices[d] = -1;
		}
		numEntities = 0;
		color = glm::vec4(info.color, 1.0f);
		//centerNodeIndex = info.centerNodeIndex;
	}
};