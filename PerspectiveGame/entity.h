#pragma once

#include <iostream>

#include "forceManager.h"
#include "collisionSolver.h"

struct Entity {
	enum Type {
		ENTITY_TYPE_DEFAULT,
		ENTITY_TYPE_ERROR,
	};

	Entity::Type type;
	glm::vec3 color;

	TileNode* node;
	int forceListIndex; // index to a number of bools that designate this entity's direction of velocity

	Entity(ForceManager* fm)
		: type(ENTITY_TYPE_ERROR)
		, color(1, 1, 1)
		, node(nullptr)
		, forceListIndex(-1)
	{}

	Entity(Entity::Type type, 
		   glm::vec3 color, 
		   TileNode* node,
		   int forceListIndex)
		: type(type)
		, color(color)
		, node(node)
		, forceListIndex(forceListIndex)
	{}
};

struct alignas(16) GPU_EntityInfo {
	alignas(16) glm::vec4 info; // R, G, B, type

	inline void setType(int type)
	{
		info[3] = (float)type;
	}
	inline void setColor(glm::vec3 color)
	{
		info = glm::vec4(color, info[3]);
	}

	GPU_EntityInfo(Entity* entity)
	{
		setType(entity->type);
		setColor(entity->color);
	}
};