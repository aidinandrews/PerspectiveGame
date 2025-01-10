#pragma once
#include <iostream>
#include <vector>
#include <algorithm>

#include "tileNavigation.h"

struct Tile
{
private:
	int neighborIndices[4];
	MapType neighborMaps[4];

public:
	TileType type;
	int index;
	int centerNodeIndex;
	glm::vec3 color;
	glm::vec2 textureCoordinates[4];
	int siblingIndex;

	Tile(TileType type, int index, int siblingIndex, int centerNodeIndex, glm::vec3 color) 
		: type(type)
		, index(index)
		, siblingIndex(siblingIndex)
		, centerNodeIndex(centerNodeIndex)
		, color(color)
	{
		setTextureCoordsDefault();
	}

	Tile()
	{
		wipe();
		setTextureCoordsDefault();
	}

	void setTextureCoordsDefault()
	{
		textureCoordinates[0] = glm::vec2(1, 1);
		textureCoordinates[1] = glm::vec2(1, 0);
		textureCoordinates[2] = glm::vec2(0, 0);
		textureCoordinates[3] = glm::vec2(0, 1);
	}

	void wipe()
	{
		index = -1;
		siblingIndex = -1;
		centerNodeIndex = -1;
		type = TILE_TYPE_ERROR;
		color = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	}

	int getNeighborIndex(LocalDirection d) { return neighborIndices[d]; }
	void setNeighborIndex(LocalDirection d, int index) { neighborIndices[d] = index; }

	MapType getNeighborMap(LocalDirection d) { return neighborMaps[d]; }
	void setNeighborMap(LocalDirection d, MapType m) { neighborMaps[d] = m; }
};

struct alignas(32) GPU_Tile
{
	alignas(4) int neighbors[4];
	alignas(4) int maps[4];

	alignas(4) int entityPositions[4];
	alignas(4) int entityDirections[4];

	alignas(32) glm::vec2 texCoords[4];

	alignas(16) glm::vec4 color;
	alignas(4) int numEntities;
	alignas(4) int padding[3];

	GPU_Tile(Tile& tile)
	{
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			neighbors[d] = tile.getNeighborIndex(d);
			maps[d] = tile.getNeighborMap(d);
			texCoords[d] = tile.textureCoordinates[d];
		}
		color = glm::vec4(tile.color, 1.0f);
		numEntities = 0;
	}

	void addEntity(LocalPosition pos, LocalDirection heading)
	{
		entityPositions[numEntities] = pos;
		entityDirections[numEntities] = heading;
		numEntities++;
	}
};