#pragma once

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

#include "globalVariables.h"
#include "shaderManager.h"
#include "vertexManager.h"
#include "vectorHelperFunctions.h"
#include "Button.h"
#include "frameBuffer.h"
#include "inputManager.h"

struct ButtonManager {
public:
	ShaderManager *p_shaderManager;
	GLFWwindow *p_window;
	InputManager *p_inputManager;
	Framebuffer *p_framebuffer;
	

	std::vector<Button> buttons;
	const static int pov2d3rdPersonViewButtonIndex = 0;
	int pov2d3rdPersonViewTexId;
	const static int pov3d3rdPersonViewButtonIndex = 1;

	// Used for when hovering over a button:
	bool hasTargetButton = false;
	Button *p_targetButton;
	glm::ivec2 targetButtonClickedPos;
	glm::ivec2 targetButtonClickedSize;
	glm::ivec2 lastClickedCursorGridPos;
	ButtonArea targetButtonArea;

public:
	ButtonManager(Framebuffer *fb, ShaderManager *sm, GLFWwindow* w, InputManager *im) 
		: p_framebuffer(fb), p_shaderManager(sm), p_window(w), p_inputManager(im) {
		
		pov2d3rdPersonViewTexId = p_framebuffer->pov2D3rdPersonTextureID;

		Button pov2d3rdPersonViewButton;
		pov2d3rdPersonViewButton.size = glm::ivec2(10, 10);
		pov2d3rdPersonViewButton.color = glm::vec3(1, 1, 0);
		pov2d3rdPersonViewButton.hasTexture = true;
		buttons.push_back(pov2d3rdPersonViewButton);

		Button pov3d3rdPersonViewButtonIndex(glm::ivec2(1, 1), glm::ivec2(1, 1), glm::vec3(1, 0.5, 1));
		pov3d3rdPersonViewButtonIndex.pos = glm::ivec2(0, 6);
		pov3d3rdPersonViewButtonIndex.size = glm::ivec2(3, 3);
		pov3d3rdPersonViewButtonIndex.color = glm::vec3(1, 0, 0);
		pov3d3rdPersonViewButtonIndex.hasTexture = true;
		buttons.push_back(pov3d3rdPersonViewButtonIndex);
	}

	void adjustTargetButtonSize() {
		glm::ivec2 cursorGridPos = glm::ivec2(CursorPixelPos) / PixelsPerGuiGridUnit;
		glm::ivec2 cursorGridPosAdj = (glm::ivec2(CursorPixelPos) + glm::ivec2(PixelsPerGuiGridUnit, PixelsPerGuiGridUnit) / 2)
			/ PixelsPerGuiGridUnit;
		glm::ivec2 difFromLastClickedPos = targetButtonClickedPos + targetButtonClickedSize;
		float changeOffset = 0.5f;
		switch (targetButtonArea) {
		case BUTTON_AREA_INSIDE:
			p_targetButton->pos = targetButtonClickedPos + cursorGridPos - lastClickedCursorGridPos;
			break;
		case BUTTON_AREA_LEFT_EDGE:
			p_targetButton->pos = glm::ivec2(cursorGridPosAdj.x, targetButtonClickedPos.y);
			p_targetButton->size = targetButtonClickedSize;
			p_targetButton->size.x += targetButtonClickedPos.x - cursorGridPosAdj.x;
			if (p_targetButton->size.x <= 0) {
				p_targetButton->pos = targetButtonClickedPos;
				p_targetButton->pos.x = targetButtonClickedPos.x + targetButtonClickedSize.x - 1;
				p_targetButton->size = glm::ivec2(1, targetButtonClickedSize.y);
			}
			break;
		case BUTTON_AREA_RIGHT_EDGE:
			p_targetButton->size = targetButtonClickedSize;
			p_targetButton->size.x += cursorGridPosAdj.x - difFromLastClickedPos.x;
			if (p_targetButton->size.x <= 0) {
				p_targetButton->size = glm::ivec2(1, targetButtonClickedSize.y);
			}
			break;
		case BUTTON_AREA_TOP_EDGE:
			p_targetButton->size = targetButtonClickedSize;
			p_targetButton->size.y += cursorGridPosAdj.y - difFromLastClickedPos.y;
			if (p_targetButton->size.y <= 0) {
				p_targetButton->size = glm::ivec2(targetButtonClickedSize.x, 1);
			}
			break;
		case BUTTON_AREA_BOTTOM_EDGE:
			p_targetButton->pos = glm::ivec2(targetButtonClickedPos.x, cursorGridPosAdj.y);
			p_targetButton->size = targetButtonClickedSize;
			p_targetButton->size.y += targetButtonClickedPos.y - cursorGridPosAdj.y;
			if (p_targetButton->size.y <= 0) {
				p_targetButton->pos = targetButtonClickedPos;
				p_targetButton->pos.y = targetButtonClickedPos.y + targetButtonClickedSize.y - 1;
				p_targetButton->size = glm::ivec2(targetButtonClickedSize.x, 1);
			}
			break;
		case BUTTON_AREA_TOP_LEFT_CORNER:
			p_targetButton->pos = glm::ivec2(cursorGridPosAdj.x, targetButtonClickedPos.y);
			p_targetButton->size = targetButtonClickedSize;
			p_targetButton->size.x += targetButtonClickedPos.x - cursorGridPosAdj.x;
			p_targetButton->size.y += cursorGridPosAdj.y - difFromLastClickedPos.y;
			if (p_targetButton->size.x <= 0) {
				p_targetButton->pos = targetButtonClickedPos;
				p_targetButton->pos.x = targetButtonClickedPos.x + targetButtonClickedSize.x - 1;
				p_targetButton->size.x = 1;
			}
			if (p_targetButton->size.y <= 0) {
				p_targetButton->size.y = 1;
			}
			break;
		case BUTTON_AREA_TOP_RIGHT_CORNER:
			p_targetButton->size = targetButtonClickedSize + cursorGridPosAdj - difFromLastClickedPos;
			if (p_targetButton->size.x <= 0) {
				p_targetButton->size.x = 1;
			}
			if (p_targetButton->size.y <= 0) {
				p_targetButton->size.y = 1;
			}
			break;
		case BUTTON_AREA_BOTTOM_RIGHT_CORNER:
			p_targetButton->pos = glm::ivec2(targetButtonClickedPos.x, cursorGridPosAdj.y);
			p_targetButton->size = targetButtonClickedSize;
			p_targetButton->size.x += cursorGridPosAdj.x - difFromLastClickedPos.x;
			p_targetButton->size.y += targetButtonClickedPos.y - cursorGridPosAdj.y;
			if (p_targetButton->size.y <= 0) {
				p_targetButton->pos = targetButtonClickedPos;
				p_targetButton->pos.y = targetButtonClickedPos.y + targetButtonClickedSize.y - 1;
				p_targetButton->size.y = 1;
			}
			if (p_targetButton->size.x <= 0) {
				p_targetButton->size.x = 1;
			}
			break;
		case BUTTON_AREA_BOTTOM_LEFT_CORNER:
			p_targetButton->pos = cursorGridPosAdj;
			p_targetButton->size = targetButtonClickedSize + targetButtonClickedPos - cursorGridPosAdj;
			if (p_targetButton->size.y <= 0) {
				p_targetButton->pos.y = targetButtonClickedPos.y + targetButtonClickedSize.y - 1;
				p_targetButton->size.y = 1;
			}
			if (p_targetButton->size.x <= 0) {
				p_targetButton->pos.x = targetButtonClickedPos.x + targetButtonClickedSize.x - 1;
				p_targetButton->size.x = 1;
			}
			break;
		}
	}

	bool findTargetButton() {
		ButtonArea area = BUTTON_AREA_OUTSIDE;
		if (hasTargetButton) {
			return true;
		}
		for (Button &button : buttons) {
			area = button.isHoveredOver(CursorPixelPos);
			if (area != BUTTON_AREA_OUTSIDE && p_inputManager->mouseButtons[LEFT_CLICK_MOUSE_BUTTON].pressed) {
				hasTargetButton = true;
				p_targetButton = &button;
				targetButtonClickedPos = button.pos;
				targetButtonClickedSize = button.size;
				targetButtonArea = area;
				lastClickedCursorGridPos = (glm::ivec2)CursorPixelPos / PixelsPerGuiGridUnit;
				return true;
			}
		}
		return false;
	}

	void updateButtons() {
		// Reset the target button's info if not clicking:
		if (p_inputManager->leftMouseButtonReleased()) {
			hasTargetButton = false;
			p_targetButton = nullptr;
			targetButtonArea = BUTTON_AREA_OUTSIDE;
		}
		if (findTargetButton() == false) { return; }

		if (CanEditSubWindows) {
			adjustTargetButtonSize();
		}
	}

	void renderButton(Button &button, GLuint texId) {
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glViewport(0, 0, WindowSize.x, WindowSize.y);

		glBindVertexArray(p_framebuffer->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, p_framebuffer->VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_framebuffer->EBO);

		std::vector<GLfloat> verts;
		if (button.hasTexture) {
			setVertAttribVec3PosVec3NormVec3ColorVec2TextCoord();
			p_shaderManager->simpleShader.use();

			GLuint transfMatrixID = glGetUniformLocation(p_shaderManager->simpleShader.ID, "inTransfMatrix");
			glUniformMatrix4fv(transfMatrixID, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));

			GLuint alphaID = glGetUniformLocation(p_shaderManager->simpleShader.ID, "inAlpha");
			glUniform1f(alphaID, 0.0f);

			GLuint colorAlphaID = glGetUniformLocation(p_shaderManager->simpleShader.ID, "inColorAlpha");
			glUniform1f(colorAlphaID, 0.0f);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texId);

			for (int i = 0; i < 4; i++) {
				// position of vertex:
				glm::vec2 pos = guiGridUnitSpaceToWindowSpace(button.getVertPos(i));
				verts.push_back(pos.x);
				verts.push_back(-pos.y);
				verts.push_back(0);
				// normals:
				verts.push_back(0);
				verts.push_back(0);
				verts.push_back(0);
				// color of this vertex:
				verts.push_back(button.color.r);
				verts.push_back(button.color.g);
				verts.push_back(button.color.b);
				// texture coordinates at this vertex:
				verts.push_back(button.texCoords[i].x);
				verts.push_back(button.texCoords[i].y);
			}
		} else {
			setVertAttribVec2PosVec2TexCoordVec3Color();
			p_shaderManager->justVertsAndColors.use();

			for (int i = 0; i < 4; i++) {
				// position of vertex:
				glm::vec2 pos = guiGridUnitSpaceToWindowSpace(button.getVertPos(i));
				verts.push_back(pos.x);
				verts.push_back(pos.y);
				// texture coordinates at this vertex:
				verts.push_back(button.texCoords[i].x);
				verts.push_back(button.texCoords[i].y);
				// color of this vertex:
				verts.push_back(button.color.r);
				verts.push_back(button.color.g);
				verts.push_back(button.color.b);
			}
		}
		std::vector<GLuint> indices = {
			0, 1, 3, 1, 2, 3,
		};

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, texID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verts.size(), verts.data(), GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
		glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
	}
};
