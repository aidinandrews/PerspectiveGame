#pragma once
#include<iostream>
#ifndef GLAD_INCLUDED
#include <glad/glad.h>
#endif
#include<GLFW/glfw3.h>
#include<vector>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp> // Linear algebra.
#include <glm/gtc/matrix_transform.hpp>

#include"shaderManager.h"

extern std::vector<GLfloat> verts;
extern std::vector<GLuint> indices;

void defineVertexAttribPointer(GLuint locationID, GLuint vertexSubAttributeSize,
	GLint attributeVariableTypeID, bool normalize, GLint strideSize, void* offset);

void setVertAttribVec3PosVec3NormVec3ColorVec2TextCoord();
void setVertAttribVec2PosVec2TexCoordVec3Color();
void setVertAttribVec2Pos();
void setVertAttribVec3PosVec3NormVec3ColorVec2TextCoord1Index();

struct VertexManager {
	GLuint vertBuffObj, vertArrayObj, elementBuffObj;
	ShaderManager* p_shaderManager;

	VertexManager() {}
	~VertexManager() {
		glDeleteVertexArrays(1, &vertArrayObj);
		glDeleteBuffers(1, &vertBuffObj);
	}

	void init(ShaderManager* shaderManager) {
		p_shaderManager = shaderManager;

		glGenVertexArrays(1, &vertArrayObj);
		glBindVertexArray(vertArrayObj);
		
		glGenBuffers(1, &vertBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, vertBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verts.size(), verts.data(), GL_STATIC_DRAW);

		setVertAttribVec3PosVec3NormVec3ColorVec2TextCoord();

		glGenBuffers(1, &elementBuffObj);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffObj);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);
	}
};