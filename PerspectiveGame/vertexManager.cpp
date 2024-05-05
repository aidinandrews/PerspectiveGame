#include"vertexManager.h"

std::vector<GLfloat> verts;
std::vector<GLuint> indices;

void defineVertexAttribPointer(GLuint locationID, GLuint vertexSubAttributeSize,
	GLint attributeVariableTypeID, bool normalize, GLint strideSize, void* offset) {
	glVertexAttribPointer(locationID, vertexSubAttributeSize, attributeVariableTypeID,
		normalize, strideSize, offset);
	glEnableVertexAttribArray(locationID);
}

void setVertAttribVec3PosVec3NormVec3ColorVec2TextCoord() {
	// Position:
	defineVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)0);
	// Normal:
	defineVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	// Color:
	defineVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	// Texture:
	defineVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(9 * sizeof(GLfloat)));
}

void setVertAttribVec2PosVec2TexCoordVec3Color() {
	// Position:
	defineVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void *)0);
	// Texture Coordinate:
	defineVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
	// Color:
	defineVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void *)(4 * sizeof(GLfloat)));
}

void setVertAttribVec2Pos() {
	defineVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
}

void setVertAttribVec3PosVec3NormVec3ColorVec2TextCoord1Index() {
	GLint strideSize = 12 * sizeof(GLfloat);
	// Position:
	defineVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, strideSize, (void *)0);
	// Normal:
	defineVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, strideSize, (void *)(3 * sizeof(GLfloat)));
	// Color:
	defineVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, strideSize, (void *)(6 * sizeof(GLfloat)));
	// Texture:
	defineVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, strideSize, (void *)(9 * sizeof(GLfloat)));
	// Tile index:
	defineVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, strideSize, (void *)(11 * sizeof(GLint)));
}