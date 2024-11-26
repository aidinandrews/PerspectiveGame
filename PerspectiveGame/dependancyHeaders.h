#pragma once
#ifndef VAGUE_HEADERS_DEFINED

#define NOMINMAX
#define USING_WINDOWS
#include "Windows.h"

	#include<iostream>
	
	#ifndef GLAD_INCLUDED
		#include <glad/glad.h>
	#endif
	#define GLAD_INCLUDED
	
	#define GLFW_INCLUDE_NONE
	#include <GLFW/glfw3.h>
	
	#define GLM_FORCE_RADIANS
	#define GLM_FORCE_DEPTH_ZERO_TO_ONE
	#include <glm/glm.hpp>
	#include <glm/gtc/matrix_transform.hpp>
	#include <glm/gtc/type_ptr.hpp>
	
	#ifndef STB_INCLUDED
		#include "stb_image.h"
	#endif
	#define STB_INCLUDED


#endif
#define VAGUE_HEADERS_DEFINED