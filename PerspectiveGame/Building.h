//#ifndef BUILDING_DEFINED
//#define BUILDING_DEFINED

#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#ifndef GLAD_INCLUDED
#include <glad/glad.h>
#endif
#include<GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#ifndef STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>
#endif

#include "entity.h"
#include "globalVariables.h"
//#include "tile.h"

struct Producer {
	float cooldown;
	int producedEntityID;
	int tileIndex;
	bool connected;

	Producer() {
		cooldown = 0.0f;
		producedEntityID = 0;
		tileIndex = 0;
		connected = false;
	}

	Producer(int tileIndex, int producedEntityID) : tileIndex(tileIndex), producedEntityID(producedEntityID) {
		cooldown = 0.0f;
		connected = false;
	}

	void update() {
		if (cooldown > 0.0f) {
			cooldown = std::max(0.0f, cooldown - 1.0f * DeltaTime);
		}
	}
};

struct Consumer {
	float cooldown;
	int tileIndex;

	Consumer() {
		cooldown = 0.0f;
		tileIndex = 0;
	}

	Consumer(int tileIndex) : tileIndex(tileIndex) {
		cooldown = 0.0f;
	}

	void update() {
		if (cooldown > 0.0f) {
			cooldown = std::max(0.0f, cooldown - 1.0f * DeltaTime);
		}
	}
};



















//struct Tile;
//
//// Each tile can have a building on it, which is the most generic version of an object that houses/effects
//// entities and/or other buildings around it.
//class Building {
//public:
//	enum Type {
//		NONE,
//		PRODUCER,
//		CONSUMER,
//	};
//
//	enum Side {
//		RIGHT, DOWN, LEFT, UP
//	};
//
//	Tile *parentTile;
//	Building::Side orientation; // Relative to Tile.sideInfos
//	Building::Type buildingType;
//
//	Building() {
//		parentTile = nullptr;
//		orientation = Building::Side::UP;
//		buildingType = Building::Type::NONE;
//	}
//
//	Building(Building::Side orientation, Building::Type buildingType, Tile *parentTile);
//
//	virtual ~Building() {}
//
//	virtual void update() {}
//
//	virtual Entity::Type getEntityType() { return Entity::Type::NONE; }
//	virtual float getEntityOffset() { return 0.0f; }
//	virtual int getEntityOffsetSide() { return 0; }
//};
//
//// Buildings that do not house/store/create entities but rather effect other building/entities around them:
//class EffectBuilding : public Building {
//public:
//
//	Entity::Type getEntityType() override { return Entity::Type::NONE; }
//	float getEntityOffset() override { return 0.0f; }
//	int getEntityOffsetSide() override { return 0; }
//};
//
//// Buildings that house / store / create / destroy or otherwise deal with entites inside them :
//class EntityBuilding : public Building {
//public:
//	Entity entity;
//
//	EntityBuilding(Building::Side orientation, Building::Type buildingType, Tile *parentTile) :
//		Building(orientation, buildingType, parentTile) {
//		entity.type = Entity::Type::NONE;
//		entity.offsetSide = 0;
//		entity.offset = 0.0f;
//	}
//
//	Entity::Type getEntityType() override { return entity.type; }
//	float getEntityOffset() override { return entity.offset; }
//	int getEntityOffsetSide() override { return entity.offsetSide; }
//};
//
//// Building that creates an entity and then gives it to a neighboring building if able:
//class Producer : public EntityBuilding {
//public:
//	Entity::Type producedEntityType;
//	EntityBuilding *connectedBuilding;
//	float cooldown;
//
//	Producer(Building::Side orientation, Tile *parentTile) :
//		EntityBuilding(orientation, Building::Type::PRODUCER, parentTile) {
//		cooldown = 0.0f;
//		updateConnections();
//	}
//
//	void updateConnections() {}
//
//	void update() override;
//};
//
//// Building that creates an entity and then gives it to a neighboring building if able:
//struct Consumer : EntityBuilding {
//	EntityBuilding *connectedBuilding[4];
//	float cooldown;
//
//	Consumer(Building::Side orientation, Tile *parentTile) :
//		EntityBuilding(orientation, Building::Type::CONSUMER, parentTile) {
//		cooldown = 0.0f;
//		updateConnections();
//	}
//
//	// connects/updades the connections of the consumer to all the entity buildings adjacent to it.
//	void updateConnections();
//
//	void update() {
//		if (cooldown > 0.0f) {
//			cooldown -= 1.0f * DeltaTime;
//			return;
//		}
//		if (entity.type != Entity::Type::NONE) {
//			entity.offset = std::max(0.0f, entity.offset - 1 * DeltaTime);
//			return;
//		}
//		// Delete the entity:
//		if (entity.offset == 0.0f) {
//			entity.type = Entity::Type::NONE;
//			cooldown = 1.0f;
//		}
//	}
//};
//
//#endif