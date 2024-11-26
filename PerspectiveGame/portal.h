#pragma once
#include<iostream>
#include<vector>

#include"dependancyHeaders.h"

#include"makeShapes.h"
#include"cameraManager.h"
#include"shaderManager.h"

struct Portal {
	Camera* p_camera;
	ShaderManager* p_shaderManager;
	glm::vec2 postA, postB;
	glm::vec4 boundingBox;
	bool onSide;
	float angleToConnectedPortal;
	glm::vec2 distToSiblingPortal;
	float siblingScaleDif;

public:
	Portal() {
		p_camera = nullptr;
		p_shaderManager = nullptr;
		onSide = true;

		siblingScaleDif = 0.0f;
	}
	Portal(Camera* c, ShaderManager* sm, glm::vec2 p1, glm::vec2 p2) {
		p_camera = c;
		p_shaderManager = sm;

		postA = p1, postB = p2;
		onSide = vechelp::isLeft(glm::vec2(p_camera->viewPlanePos.x, -p_camera->viewPlanePos.y),
			postA, postB);
		updateBoundingBox();
	}

	glm::vec2 averagePostPos() {
		return (postA + postB) / 2.0f;
	}

	void updateBoundingBox() {
		boundingBox[0] = std::min(postA.x, postB.x);
		boundingBox[1] = std::max(postA.x, postB.x);
		boundingBox[2] = std::min(postA.y, postB.y);
		boundingBox[3] = std::max(postA.y, postB.y);
	}

	float boundingBoxMinX() { return boundingBox[0]; }
	float boundingBoxMaxX() { return boundingBox[1]; }
	float boundingBoxMinY() { return boundingBox[2]; }
	float boundingBoxMaxY() { return boundingBox[3]; }

	bool inBoundingBox(glm::vec2 pos) {
		return boundingBoxMinX() < pos.x && pos.x < boundingBoxMaxX()
			&& boundingBoxMinY() < pos.y&& pos.y < boundingBoxMaxY();
	}

	float distToCamPos() {
		return glm::distance(glm::vec2(p_camera->viewPlanePos.x, -p_camera->viewPlanePos.y), (postA + postB) / 2.0f);
	}

	void drawStencilIncrVal(int val, glm::vec2 post1, glm::vec2 post2, glm::vec2 lastPost1, glm::vec2 lastPost2, int recursionLayer) {
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		drawStencil(val, post1, post2, lastPost1, lastPost2, recursionLayer);
	}

	void drawStencilSetVal(int val, glm::vec2 post1, glm::vec2 post2, glm::vec2 lastPost1, glm::vec2 lastPost2, int recursionLayer) {
		glStencilOp(GL_KEEP, GL_KEEP, GL_SET);
		drawStencil(val, post1, post2, lastPost1, lastPost2, recursionLayer);
	}

	glm::vec2 getPostVec(glm::vec2 post) {
		float maxVecSize = (float)sqrt(2);
		glm::vec2 camPos(p_camera->lastFrameViewPlanePos.x, -p_camera->lastFrameViewPlanePos.y);
		glm::vec2 portalPostVecA = glm::normalize(camPos - post);
		portalPostVecA = vechelp::rotate(portalPostVecA, float(-p_camera->yaw));
		portalPostVecA *= maxVecSize;
		return portalPostVecA;
	}

	std::vector<glm::vec2> createStencil(glm::mat4 transf, bool onLeft) {
		glm::vec2 post1 = glm::vec2(transf * glm::vec4(postA, 0, 1));
		glm::vec2 post2 = glm::vec2(transf * glm::vec4(postB, 0, 1));
		//glm::vec2 portalPostVecA = getPostVec(post1);
		glm::vec2 camPos(p_camera->viewPlanePos.x, -p_camera->viewPlanePos.y);
		glm::vec2 portalPostVecA = glm::normalize(post1 - camPos);
		glm::vec2 portalPostVecB = glm::normalize(post2 - camPos);
		//glm::vec2 portalPostVecB = getPostVec(post2);
		//glm::vec2 portalPostVecA = glm::vec2(-1, 1);
		//glm::vec2 portalPostVecB = glm::vec2(1, 1);

		glm::vec2 portalNorm = glm::normalize(portalPostVecA + portalPostVecB);
		glm::vec2 posA = glm::vec2(p_camera->transfMatrix * glm::vec4(post1, 0, 1));
		glm::vec2 posB = glm::vec2(p_camera->transfMatrix * glm::vec4(post2, 0, 1));
		glm::vec2 posC = posA + (portalPostVecA);
		glm::vec2 posD = posB + (portalPostVecB);
		// because glfw windows have a side length of 1, it is safe to make the expansion 
		// as big as it could possibly be, or sqrt(2):
		glm::vec2 posE = posC + portalNorm * sqrt(2.0f);
		glm::vec2 posF = posD + portalNorm * sqrt(2.0f);

		std::vector<glm::vec2> currentFrustum;
		if (onLeft == true) {
			currentFrustum.push_back(posB);
			currentFrustum.push_back(posA);
			currentFrustum.push_back(posA + portalPostVecA);
			currentFrustum.push_back(posB + portalPostVecB);
		}
		else {
			currentFrustum.push_back(posA);
			currentFrustum.push_back(posB);
		}
		return currentFrustum;
	}

	void drawStencil(int val, glm::vec2 post1, glm::vec2 post2, glm::vec2 lastPost1, glm::vec2 lastPost2, int recursionLayer) {
		glm::vec2 portalPostVecA = getPostVec(post2);
		glm::vec2 portalPostVecB = getPostVec(post1);
		glm::vec2 lastPortalPostVecA = getPostVec(lastPost2);
		glm::vec2 lastPortalPostVecB = getPostVec(lastPost1);
		// create triangles to send to the stencil buffer:

		glEnable(GL_STENCIL_TEST);
		glDisable(GL_DEPTH_TEST);
		glStencilFunc(GL_ALWAYS, val, 0xFF); // all fragments pass the stencil test
		glStencilMask(0xFF); // enable writing to the stencil buffer
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// mask out the triangles so that they are only drawn on subsequent 
		// triangles (except for the first set):

		glm::mat4 scaleToScreenSpace(1);
		scaleToScreenSpace[0][0] = 1;

		glm::vec2 camPos(p_camera->lastFrameViewPlanePos.x, -p_camera->lastFrameViewPlanePos.y);
		glm::vec2 portalNorm = glm::normalize(portalPostVecA + portalPostVecB);
		glm::vec2 posA = glm::vec2(p_camera->transfMatrix * glm::vec4(post1, 0, 1));
		glm::vec2 posB = glm::vec2(p_camera->transfMatrix * glm::vec4(post2, 0, 1));
		glm::vec2 posC = posA + portalPostVecA;
		glm::vec2 posD = posB + portalPostVecB;
		glm::vec2 posE = posC + portalNorm * sqrt(2.0f);
		glm::vec2 posF = posD + portalNorm * sqrt(2.0f);
		std::vector<glm::vec2> currentFrustum = {
			posB, posA, posD,posF, posE, posC
		};


		std::vector<glm::vec2>cropToWindow = {
			glm::vec2(-1, -1), glm::vec2(1, -1), glm::vec2(1, 1), glm::vec2(-1, 1)
		};
		for (glm::vec2& v : cropToWindow) {
			v *= 0.9f;
		}
		vechelp::sutherlandHodgemanPolyCrop(currentFrustum, cropToWindow, false);
		if (currentFrustum.size() == 0) {
			return;
		}

		std::vector<GLfloat> stencilVerts;
		for (glm::vec2& vec : currentFrustum) {
			stencilVerts.push_back(vec.x);
			stencilVerts.push_back(vec.y);
		}
		std::vector<GLsizei> indices;
		for (int i = 0; i < currentFrustum.size() - 2; i++) {
			indices.push_back(0);
			indices.push_back(i + 1);
			indices.push_back(i + 2);
		}

		// Create reference containers for the Vertex Array Object and the Vertex Buffer Object
		GLuint VAO, VBO, EBO;

		// Generate the VAO and VBO with only 1 object each
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		// Make the VAO the current Vertex Array Object by binding it
		glBindVertexArray(VAO);

		// Bind the VBO specifying it's a GL_ARRAY_BUFFER
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// Introduce the vertices into the VBO
		glBufferData(GL_ARRAY_BUFFER, stencilVerts.size() * sizeof(GLfloat), stencilVerts.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);

		// Configure the Vertex Attribute so that OpenGL knows how to read the VBO
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		// Enable the Vertex Attribute so that OpenGL knows to use it
		glEnableVertexAttribArray(0);

		// Bind both the VBO and VAO to 0 so that we don't accidentally modify the VAO and VBO we created
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		// Tell OpenGL which Shader Program we want to use
		p_shaderManager->stencilShader.use();

		//sceneAngleAdj = glm::rotate(glm::mat4(1), float(M_PI / 4), glm::vec3(0, 0, 1));

		glm::vec3 testColor(1, 1, 1);
		GLuint inColorID = glGetUniformLocation(p_shaderManager->stencilShader.ID, "inColor");
		glUniform3f(inColorID, testColor.r, testColor.g, testColor.b);

		//scenePosAdj = glm::vec3((postA + postB), 0) / 2.0f;

		// Bind the VAO so OpenGL knows to use it
		glBindVertexArray(VAO);
		// Draw the triangle using the GL_TRIANGLES primitive
		//glDrawArrays(GL_TRIANGLES, 0, 24);
		glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}
};

struct PortalPair {
	Portal A, B;
	//glm::vec2 atob, btoa;

	PortalPair() {
	}
	void update() {
		A.distToSiblingPortal = ((B.postA + B.postB) / 2.0f) - ((A.postA + A.postB) / 2.0f);
		B.distToSiblingPortal = -A.distToSiblingPortal;

		A.siblingScaleDif = glm::distance(A.postA, A.postB) / glm::distance(B.postA, B.postB) ;
		B.siblingScaleDif = 1 / A.siblingScaleDif;

		A.updateBoundingBox();
		B.updateBoundingBox();

		angleBetweenPortals();
	}

	void angleBetweenPortals() {
		glm::vec2 Anorm = glm::normalize(A.postA - A.postB);
		glm::vec2 Bnorm = glm::normalize(B.postA - B.postB);
		float aTobAngle = vechelp::getVecAngle(Anorm) - vechelp::getVecAngle(Bnorm);
		A.angleToConnectedPortal = aTobAngle;
		B.angleToConnectedPortal = -aTobAngle;
	}

	void rotatePair(glm::vec2 center, float angle) {
		A.postA -= center;
		A.postA = vechelp::rotate(A.postA, angle);
	}
};

struct PortalManager {
	ShaderManager* p_shaderManager;
	Camera* p_camera;

	glm::vec2 PortalAStake1;
	glm::vec2 PortalAStake2;
	glm::vec2 PortalBStake1;
	glm::vec2 PortalBStake2;
	std::vector<PortalPair> portalPairs;

	absc::ShapeInfo shapeInfo;
	std::vector<GLfloat> verts;
	std::vector<GLuint> indices;

	float scaleChange = 1.0f; // <- used by sceneManager to scale player if they go through a portal.

	PortalManager(ShaderManager* sm, Camera* c) {
		p_shaderManager = sm;
		p_camera = c;
		PortalPair pp;

		Portal portalA(p_camera, p_shaderManager,
			glm::vec2(+0.6f, -0.5f),
			glm::vec2(+0.6f, +0.5f));

		Portal portalB(p_camera, p_shaderManager,
			glm::vec2(-0.6f, -0.5f),
			glm::vec2(-0.6f, +0.5f));

		pp.A = portalA;
		pp.B = portalB;
		pp.update();
		portalPairs.push_back(pp);

		Portal portalC(p_camera, p_shaderManager,
			glm::vec2(+0.5f, +0.6f),
			glm::vec2(-0.5f, +0.6f));

		Portal portalD(p_camera, p_shaderManager,
			glm::vec2(+0.5f, -.6f),
			glm::vec2(-0.5f, -.6f));

		pp.A = portalC;
		pp.B = portalD;
		pp.update();

		portalPairs.push_back(pp);

		/*for (PortalPair& pp : portalPairs) {
			pp.A.postA = rotate(pp.A.postA, M_PI / 4);
			pp.A.postB = rotate(pp.A.postB, M_PI / 4);
			pp.B.postA = rotate(pp.B.postA, M_PI / 4);
			pp.B.postB = rotate(pp.B.postB, M_PI / 4);
		}*/
	}

	void update() {
		float timeValue = (float)glfwGetTime();
		float zeroToOne = (cos(timeValue) + 1) / 2;
		float portalRotation = (float)M_PI / 2.0f + (float)M_PI * zeroToOne;
		float val = 0.5f * zeroToOne;
		glm::vec2 xAdj(val, 0);
		glm::vec2 yAdj(0, val);
		//float squish = 0.8f + 0.2f * zeroToOne;

		glm::vec2 squish = glm::vec2(0.8f, 1.0f) * (0.5f + 0.5f *zeroToOne);
		////squish = 1.0f;
		xAdj = glm::vec2(0, 0);
		yAdj = glm::vec2(0, 0);

		/*portalPairs[0].A.postA = glm::vec2(.5f, -0.5f) + glm::vec2(val, -val);
		portalPairs[0].A.postB = glm::vec2(.5f, 0.5f) + glm::vec2(val, val);
		portalPairs[1].A.postA = glm::vec2(0.5f, 0.5f) + glm::vec2(val, val);
		portalPairs[1].B.postA = glm::vec2(0.5f, -0.5f) + glm::vec2(val, -val);*/

		portalPairs[0].A.postA = glm::vec2(+0.5f, 0.0f) * squish + yAdj;
		portalPairs[0].A.postB = glm::vec2(-0.5f, 0.0f) * squish + yAdj;
		portalPairs[0].B.postA = glm::vec2(+0.5f, -0.6f) - yAdj;
		portalPairs[0].B.postB = glm::vec2(-0.5f, -0.6f) - yAdj;
		/*portalPairs[0].A.postA = rotate(portalPairs[0].A.postA, cos(timeValue*.05) * M_PI);
		portalPairs[0].A.postB = rotate(portalPairs[0].A.postB, cos(timeValue*.05) * M_PI);
		*/portalPairs[0].A.postA += glm::vec2(0.0f, 0.6f);
		portalPairs[0].A.postB += glm::vec2(0.0f, 0.6f);

		portalPairs[1].A.postA = glm::vec2(+0.6f, -0.5f) + xAdj;
		portalPairs[1].A.postB = glm::vec2(+0.6f, +0.5f) + xAdj;
		portalPairs[1].B.postA = glm::vec2(-0.6f, -0.5f) - xAdj;
		portalPairs[1].B.postB = glm::vec2(-0.6f, +0.5f) - xAdj;

		for (PortalPair& portalPair : portalPairs) {
			portalPair.update();
		}
	}

	void updatePosIfPassThroughSpecificPortal(float angle, glm::vec3 postAtoOrigin, glm::vec3 postBtoOrigin,
		Portal& A, Portal& B) 
	{
		using namespace vechelp;

		bool currentOnSideA = isLeft(glm::vec2(p_camera->viewPlanePos.x, -p_camera->viewPlanePos.y),
			A.postA, A.postB);
		glm::vec2 portalANorm = rotate(glm::normalize(A.postA - A.postB), (float)M_PI / 2);
		std::vector<glm::vec2> portalAbbPoints = {
			A.postA + portalANorm,  A.postA - portalANorm,
			A.postB - portalANorm,  A.postB + portalANorm,
		};
		bool inPortalABoundingBox = point_in_polygon(glm::vec2(p_camera->viewPlanePos.x, -p_camera->viewPlanePos.y), portalAbbPoints);
		bool intersection = currentOnSideA != A.onSide;
		intersection = intersection && inPortalABoundingBox;
		if (intersection) {
			// Translate the player to the other portal location:
			p_camera->viewPlanePos += postAtoOrigin;
			p_camera->viewPlanePos = rotate(p_camera->viewPlanePos, angle);
			p_camera->viewPlanePos /= A.siblingScaleDif;
			p_camera->viewPlanePos -= postBtoOrigin;
			p_camera->yaw -= angle;

			// Because we translate the linear zoom when making the transf matrix, we have to inverse
			// some stuff to make the adjusted zoom look correct after going through a portal:
			float target = (float)pow(2, p_camera->zoom) * A.siblingScaleDif;
			target = (float)log(target) / (float)log(2) - p_camera->zoom;
			p_camera->zoomLinear += target;
			// Flag for scene to adjust the character size after going through a portal:
			scaleChange /= A.siblingScaleDif;
			// With all these cameral variables updated, we need to remake the transf matrix!
			p_camera->update();

			// We must update all the portal isLeft checks since our position has changed:
			for (PortalPair& pp : portalPairs) {
				pp.A.onSide = isLeft(glm::vec2(p_camera->viewPlanePos.x, -p_camera->viewPlanePos.y),
					pp.A.postA, pp.A.postB);
				pp.B.onSide = isLeft(glm::vec2(p_camera->viewPlanePos.x, -p_camera->viewPlanePos.y),
					pp.B.postA, pp.B.postB);
			}
			return;
		}
		A.onSide = currentOnSideA;
	}

	void checkIfPassThroughPortals() {
		for (PortalPair& portalPair : portalPairs) {
			float angle = portalPair.A.angleToConnectedPortal;
			glm::vec3 postAtoOrigin = glm::vec3((portalPair.A.postA + portalPair.A.postB) / 2.0f, 0);
			glm::vec3 postBtoOrigin = glm::vec3((portalPair.B.postA + portalPair.B.postB) / 2.0f, 0);
			postAtoOrigin.x *= -1;
			postBtoOrigin.x *= -1;

			// some of the variables need to be flipped depending on what portal we are checking!
			updatePosIfPassThroughSpecificPortal(angle, postAtoOrigin, postBtoOrigin, portalPair.A, portalPair.B);
			updatePosIfPassThroughSpecificPortal(-angle, postBtoOrigin, postAtoOrigin, portalPair.B, portalPair.A);
		}
	}
};