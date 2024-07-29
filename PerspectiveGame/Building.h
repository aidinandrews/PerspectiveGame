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

#include "globalVariables.h"

struct ForceSink {
	int tileIndex;

	ForceSink(int tileIndex) :
		tileIndex(tileIndex) 
	{}

	void update();
};