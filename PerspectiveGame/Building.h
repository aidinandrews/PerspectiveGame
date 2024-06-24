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

struct ForceBlock {
	int tileIndex;
	//int orientation;
	int magnitude;

	ForceBlock(int tileIndex, int orientation, int magnitude) : 
		tileIndex(tileIndex),/* orientation(orientation),*/ magnitude(magnitude) 
	{}

	void update() {

	}
};

struct ForceSink {
	int tileIndex;

	ForceSink(int tileIndex) :
		tileIndex(tileIndex) 
	{}

	void update();
};