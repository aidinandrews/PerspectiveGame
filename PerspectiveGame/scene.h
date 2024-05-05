#pragma once
#include<iostream>

#include <Windows.h>

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

#include"globalVariables.h"
#include"windowManager.h"
#include"inputManager.h"
#include"shaderManager.h"
#include"vertexManager.h"
//#include"textureManager.h"
#include"cameraManager.h"
#include"makeShapes.h"
#include"portal.h"

struct PortalPosts {
	glm::vec2 postA, postB;
};

struct PortalReference {
	Portal* portal, * sibling;
	glm::mat4 transf, transfNoScale;
	float distToClosesetPole;
	std::vector<glm::vec2> stencil;
	float tintAmount, tintOffset = 0;
	int test;
	int maxSeenPortals;
	int stencilVal;

	PortalReference(int identity = 1) {
		transf = glm::mat4(1);
		transfNoScale = glm::mat4(1);
		stencil = { glm::vec2(-1, -1), glm::vec2(1, -1), glm::vec2(1, 1), glm::vec2(-1, 1) };
		tintAmount = 0.0f;
		tintOffset = 0.0f;
		stencilVal = 1;
	}
};

struct StencilDrawInfo {
	std::vector<glm::vec2>stencil;
	int stencilVal;
	glm::vec3 stencilColor; // debugging.
	StencilDrawInfo(std::vector<glm::vec2> stencil, int stencilVal, glm::vec3 stencilColor)
		:stencil(stencil), stencilVal(stencilVal), stencilColor(stencilColor) {}
};

struct PortalViewDrawInfo {
	glm::mat4 transf;
	int stencilVal;
	float zoomAdj, tint;
	PortalViewDrawInfo(glm::mat4 transf, int stencilVal, float zoomAdj, float tint)
		: transf(transf), stencilVal(stencilVal), zoomAdj(zoomAdj), tint(tint) {}
};

struct Scene {
	struct Model {
		std::vector<GLfloat> verts;
		std::vector<GLuint> indices;
		glm::mat4 modelMatrix;
	};

	const static int MAX_PORTAL_RECURSION = 20;
	const static int MAX_SEEN_PORTALS = 5;
	float PORTAL_TINT_ADDITION = 1.0f / float(MAX_SEEN_PORTALS)+0.0001f;
	float PORTAL_TINT_RADIUS = 0.8f; // (changed /frame to account for zoom)

	const std::vector<glm::vec2> WINDOW_STENCIL = {
			glm::vec2(-1, -1), glm::vec2(1, -1), glm::vec2(1, 1), glm::vec2(-1, 1)
	};

	std::vector<Model> models;
	Camera* p_camera;
	GLFWwindow* p_window;
	PortalManager* p_portalManager;
	ShaderManager* p_shaderManager;
	std::vector<PortalViewDrawInfo> portalViewDrawInfos;
	std::vector<StencilDrawInfo> stencilDrawInfos;
	
	// OpenGL stuff:
	glm::ivec2 sceneSize;
	GLuint VAO, VBO, EBO;
	GLuint stencilVAO, stencilVBO, stencilEBO;
	GLuint sceneTexture;
	GLuint sceneRBO;
	GLuint sceneFBO;

	std::vector<glm::vec2>borderStencilPoints;

	int frameStencilVal;

	Scene(Camera* c, PortalManager* pm, ShaderManager* sm);

	~Scene() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
		glDeleteVertexArrays(1, &stencilVAO);
		glDeleteBuffers(1, &stencilVBO);
		glDeleteBuffers(1, &stencilEBO);
		glDeleteFramebuffers(1, &sceneFBO);
		glDeleteRenderbuffers(1, &sceneRBO);
		glDeleteTextures(1, &sceneTexture);
	}

	void update() {
		PORTAL_TINT_RADIUS = 2.0f * p_camera->getScaledZoom();
		frameStencilVal = 1;
		portalViewDrawInfos.clear();
		stencilDrawInfos.clear();
		fillPortalViewDrawInfos(nullptr, PortalReference(1), 0, 1.0f);
	}

	// Because the portals have to be drawn recursively, each draw call will have a transformaiton matrix
	// in order to adjust the camera to 'see' out of the portal as well as a parentPortalFrustum which will
	// be used to clip the drawn scene to it is only rendered inside the last portals frustum.  For a first pass,
	// the portalTransf will have to be the identity matrix and the frustum will have to be the entire window!
	// There are instances where there can be an infiniete amount of scenes drawn, so a maximum depth must be
	// defined as recursionDepth.
	void fillPortalViewDrawInfos(Portal* skipThisOne, PortalReference parentPr, int recursionDepth, float totalZoom);
	bool createPortalReference(PortalReference* pr, PortalReference parentPr);
	void drawScenePiece(PortalViewDrawInfo pvdi);
	void drawPortalViews();
	// Draws an individual portal stencil:
	void drawStencil(std::vector<glm::vec2>& stencilFrustum, int stencilVal, glm::vec3 color);
	// Draws each portal stencil held in the stencilDrawInfos vector:
	void drawPortalStencils();

	void draw();
};