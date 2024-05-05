#pragma once
#include<iostream>
#ifndef GLAD_INCLUDED
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderManager.h"
#include "vertexManager.h"
#include "vectorHelperFunctions.h"

enum ButtonArea {
	BUTTON_AREA_OUTSIDE,
	BUTTON_AREA_INSIDE,
	BUTTON_AREA_LEFT_EDGE, 
	BUTTON_AREA_RIGHT_EDGE, 
	BUTTON_AREA_TOP_EDGE,
	BUTTON_AREA_BOTTOM_EDGE,
	BUTTON_AREA_TOP_LEFT_CORNER,
	BUTTON_AREA_TOP_RIGHT_CORNER,
	BUTTON_AREA_BOTTOM_RIGHT_CORNER,
	BUTTON_AREA_BOTTOM_LEFT_CORNER,
};

struct Button {
	const static int minDistToEdgeForEditEdge = 5;

    glm::ivec2 pos, size;
	glm::vec2 texCoords[4];
    glm::vec3 color;
    bool on;
	bool hasTexture;

	void init() {
		texCoords[0] = glm::vec2(0, 0);
		texCoords[1] = glm::vec2(1, 0);
		texCoords[2] = glm::vec2(1, 1);
		texCoords[3] = glm::vec2(0, 1);
		on = false;
		hasTexture = false;
	}

    Button() {
        pos = glm::ivec2(0, 0);
        size = glm::ivec2(1, 1);
        color = glm::vec3(1, 1, 1);
		init();
    }

    Button(glm::ivec2 pos, glm::ivec2 size, glm::vec3 color) : pos(pos), size(size), color(color) {
		init();
    }

	int pixelWidth() { return size.x * PixelsPerGuiGridUnit; }
	int pixelHeight() { return size.y * PixelsPerGuiGridUnit; }

	glm::vec2 getVertPos(int vertIndex) {
		switch (vertIndex) {
		case 0:
			return pos; break;
		case 1:
			return pos + glm::ivec2(size.x, 0); break;
		case 2:
			return pos + size; break;
		default: // case 3:
			return pos + glm::ivec2(0, size.y); break;
		}
	}

	static glm::vec2 guiGridUnitSpaceToWindowSpace(glm::ivec2 gridPos) {
		return glm::vec2(
			(float(gridPos.x * PixelsPerGuiGridUnit) / float(WindowSize.x) * 2) - 1,
			(float(gridPos.y * PixelsPerGuiGridUnit) / float(WindowSize.y) * 2) - 1);
	}

	static glm::ivec2 guiGridUnitSpaceToPixelSpace(glm::ivec2 gridPos) {
		return gridPos * PixelsPerGuiGridUnit;
	}

	glm::vec2 screenSpaceCenterPos() {
		return guiGridUnitSpaceToWindowSpace(glm::vec2(size) / 2.0f + glm::vec2(pos));
	}

	ButtonArea isHoveredOver(glm::vec2 cursorPosPixelSpace) {
		glm::vec2 bottomLeft = guiGridUnitSpaceToPixelSpace(pos);
		glm::vec2 topRight = guiGridUnitSpaceToPixelSpace(pos + size);

		if (cursorPosPixelSpace.x < bottomLeft.x || cursorPosPixelSpace.x > topRight.x
			|| cursorPosPixelSpace.y < bottomLeft.y || cursorPosPixelSpace.y > topRight.y) {
			return BUTTON_AREA_OUTSIDE;
		}
		if (abs(cursorPosPixelSpace.x - bottomLeft.x) < minDistToEdgeForEditEdge) {
			if (abs(cursorPosPixelSpace.y - bottomLeft.y) < minDistToEdgeForEditEdge) {
				return BUTTON_AREA_BOTTOM_LEFT_CORNER;
			} else if (abs(cursorPosPixelSpace.y - topRight.y) < minDistToEdgeForEditEdge) {
				return BUTTON_AREA_TOP_LEFT_CORNER;
			} else {
				return BUTTON_AREA_LEFT_EDGE;
			}
		} else if (abs(cursorPosPixelSpace.x - topRight.x) < minDistToEdgeForEditEdge) {
			if (abs(cursorPosPixelSpace.y - bottomLeft.y) < minDistToEdgeForEditEdge) {
				return BUTTON_AREA_BOTTOM_RIGHT_CORNER;
			} else if (abs(cursorPosPixelSpace.y - topRight.y) < minDistToEdgeForEditEdge) {
				return BUTTON_AREA_TOP_RIGHT_CORNER;
			} else {
				return BUTTON_AREA_RIGHT_EDGE;
			}
		} else if (abs(cursorPosPixelSpace.y - bottomLeft.y) < minDistToEdgeForEditEdge) {
			return BUTTON_AREA_BOTTOM_EDGE;
		} else if (abs(cursorPosPixelSpace.y - topRight.y) < minDistToEdgeForEditEdge) {
			return BUTTON_AREA_TOP_EDGE;
		} else {
			return BUTTON_AREA_INSIDE;
		}
	}
};