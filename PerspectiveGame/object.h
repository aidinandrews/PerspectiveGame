#pragma once
#include<iostream>
#include<vector>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Obj {
	glm::vec3 pos;
	float radius;
};

struct ObjManager {
	std::vector<Obj> objs;
};