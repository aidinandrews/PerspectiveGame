#include"scene.h"

bool comparePortalReferences(PortalReference A, PortalReference B) {
	return (A.distToClosesetPole > B.distToClosesetPole);
}

int pointIsSuperInsidePoly(glm::vec2 p, std::vector<glm::vec2>poly) {
	for (int i = 0; i < poly.size(); i++) {
		glm::vec2 a = poly[i];
		glm::vec2 b = poly[(i + 1) % poly.size()];

		if (isLeftSpecific(p, a, b) == 0) {
			return 0;
		}
		else if (isLeftSpecific(p, a, b) == -1) {
			return -1;
		}
	}
	return 1;
}

Scene::Scene(Camera* c, PortalManager* pm, ShaderManager* sm) {
	p_camera = c;
	p_portalManager = pm;
	p_shaderManager = sm;
	sceneSize = glm::ivec2(1000, 1000);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glGenVertexArrays(1, &stencilVAO);
	glGenBuffers(1, &stencilVBO);
	glGenBuffers(1, &stencilEBO);

	glGenTextures(1, &sceneTexture);
	glBindTexture(GL_TEXTURE_2D, sceneTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, sceneSize.x, sceneSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &sceneRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, sceneRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, sceneSize.x, sceneSize.y);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &sceneFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
	// Attach the texture to FBO color attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTexture, 0);
	// Attach the renderbuffer to depth attachment point
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sceneRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	absc::ShapeInfo si;
	int modelIndex = 0;
	Model temp;
	glm::mat4 tempMat(1);

	// player:
	models.push_back(temp);
	absc::sphere(glm::vec3(.1, .1, .1), 20, 20, si);
	si.addselfTo(&models[modelIndex].verts, &models[modelIndex].indices, glm::vec3(1, 1, 0), glm::vec2(0, 0));
	modelIndex++;
	tempMat = glm::mat4(1);
	si.clear();

	// floor:
	models.push_back(temp);
	absc::prism(glm::vec3(4, 4, .01), 3, false, si);
	si.addselfTo(&models[modelIndex].verts, &models[modelIndex].indices, glm::vec3(0, 1, 0), glm::vec2(0, 0));
	models[modelIndex].modelMatrix = tempMat;
	modelIndex++;
	tempMat = glm::mat4(1);

	// big floor:
	si.clear();
	models.push_back(temp);
	absc::inversePoly2D(2, 2, 0, 4, si);
	si.addselfTo(&models[modelIndex].verts, &models[modelIndex].indices, glm::vec3(0, 0, 1), glm::vec2(0, 0));
	models[modelIndex].modelMatrix = tempMat;
	modelIndex++;
	tempMat = glm::mat4(1);

	si.clear();

	// main torus thingy:
	models.push_back(temp);
	absc::torus(0.125f, 0.125f, .25, .25, 20, 20, 1, 0, si);
	si.addselfTo(&models[modelIndex].verts, &models[modelIndex].indices, glm::vec3(1, 0, 0), glm::vec2(0, 0));
	models[modelIndex].modelMatrix = tempMat;
	tempMat = glm::mat4(1);
	modelIndex++;
	si.clear();
}

std::vector<glm::vec2> createStencil(glm::vec2 p1, glm::vec2 p2, Camera* camera, bool onLeft) {
	glm::vec2 camPos = glm::vec2(camera->viewPlanePos);
	camPos.y *= -1;
	glm::vec2 p1Vec = rotate(2.0f * glm::normalize(p1 - camPos), -camera->yaw - (float)M_PI);
	glm::vec2 p2Vec = rotate(2.0f * glm::normalize(p2 - camPos), -camera->yaw - (float)M_PI);
	// Make sure that these vectors (in screen space) are proportional to the current window ratio:
	float windowAdj = float(WindowSize.x) / float(WindowSize.y);
	p1Vec.x /= windowAdj;
	p2Vec.x /= windowAdj;

	// Used for expanding the frustum out fron the initial trapazoid to a shape like so:
	//  ________
	// |        | <- expansion
	// |________|
	//  \      / <- initial trapezoid
	//   \    /
	//    \__/ <- portal
	// 
	//     @ <- point of view
	//
	glm::vec2 portalNorm = 2.0f * glm::normalize((p1Vec + p2Vec) / 2.0f);

	p1 = glm::vec2(camera->transfMatrix * glm::vec4(p1, 0, 1));
	p2 = glm::vec2(camera->transfMatrix * glm::vec4(p2, 0, 1));

	if (!onLeft) { // This reverses the vertex winding:
		std::swap(p1, p2);
		std::swap(p1Vec, p2Vec);
	}
	std::vector<glm::vec2> currentFrustum;
	currentFrustum.push_back(p2);
	currentFrustum.push_back(p1);
	currentFrustum.push_back(p1 + p1Vec);
	currentFrustum.push_back(p1 + p1Vec + portalNorm);
	currentFrustum.push_back(p2 + p2Vec + portalNorm);
	currentFrustum.push_back(p2 + p2Vec);
	return currentFrustum;
}

bool Scene::createPortalReference(PortalReference* pr, PortalReference parentPr) {
	// Quick/visually necessary cull to see if the portal even needs to be drawn.  If neither posts 
	// are inside the parent stencil, then there is not need to do any more work as it cannot be seen.
	glm::vec2 postAScreenSpace = glm::vec2(p_camera->transfMatrix * parentPr.transf * glm::vec4(pr->portal->postA, 0, 1));
	glm::vec2 postBScreenSpace = glm::vec2(p_camera->transfMatrix * parentPr.transf * glm::vec4(pr->portal->postB, 0, 1));
	// There has to be a more efficient way to check this but this works:
	std::vector<glm::vec2> testStencilInsideParent = { postAScreenSpace, postBScreenSpace };
	sutherlandHodgemanPolyCrop(testStencilInsideParent, parentPr.stencil, false);
	if (testStencilInsideParent.size() < 2) {
		return false;
	}

	// Depending on what side of the portal we are facing, the create stencil algo
	// will output differently-wound polygons.  Sutherland Hodgeman algo expects all inputs to be 
	// wound the same way though, so we have to make sure they are with this argument onLeft:
	glm::vec2 camPos(p_camera->viewPlanePos.x, -p_camera->viewPlanePos.y);
	glm::vec2 camPosScreenSpace = glm::vec2(p_camera->transfMatrix * glm::vec4(camPos, 0, 1));
	bool onleft = isLeft(camPosScreenSpace, postAScreenSpace, postBScreenSpace);
	// Finally we crop the stencil to the parent so that no overdraw happens:
	glm::vec2 p1 = glm::vec2(parentPr.transf * glm::vec4(pr->portal->postA, 0, 1));
	glm::vec2 p2 = glm::vec2(parentPr.transf * glm::vec4(pr->portal->postB, 0, 1));
	std::vector<glm::vec2> stencil = createStencil(p1, p2, p_camera, onleft);
	sutherlandHodgemanPolyCrop(stencil, parentPr.stencil, true);
	if (stencil.size() <= 2) {
		return false; // Not sure why bus some stencils slip past the initial check.
	}
	pr->stencil = stencil;

	// Create the transformation matrix for the future draw call:
	glm::mat4 goToOriginTransf = glm::translate(glm::mat4(1), -glm::vec3(pr->sibling->averagePostPos(), 0));
	glm::mat4 rotateTransf = glm::rotate(glm::mat4(1), pr->portal->angleToConnectedPortal, glm::vec3(0, 0, 1));
	// Going through portals of different sizes will scale the object going through:
	glm::mat4 zoomTransf = glm::scale(glm::mat4(1), glm::vec3(pr->portal->siblingScaleDif, pr->portal->siblingScaleDif, pr->portal->siblingScaleDif));
	glm::mat4 goToSiblingTransf = glm::translate(glm::mat4(1), glm::vec3(pr->portal->averagePostPos(), 0));
	pr->transf = parentPr.transf * goToSiblingTransf * zoomTransf * rotateTransf * goToOriginTransf;
	pr->transfNoScale = parentPr.transfNoScale * goToSiblingTransf * rotateTransf * goToOriginTransf;

	// Distance is used to draw the furthest portals first so that overlapping portals are
	// drawn visually correct.  Because some portals warp space, we need to use a non-warped transf matrix,
	// made earlier by omitting the scale step of the regular transf matrix.
	glm::vec2 postAScreenSpaceNoScale = glm::vec2(p_camera->transfMatrix * parentPr.transfNoScale * glm::vec4(pr->portal->postA, 0, 1));
	glm::vec2 postBScreenSpaceNoScale = glm::vec2(p_camera->transfMatrix * parentPr.transfNoScale * glm::vec4(pr->portal->postB, 0, 1));
	pr->distToClosesetPole = distToLineSeg(camPosScreenSpace, postAScreenSpaceNoScale, postBScreenSpaceNoScale, nullptr);
	float portalCloseness = -std::clamp(pr->distToClosesetPole / (PORTAL_TINT_RADIUS * (p_camera->getScaledZoom())), 0.0f, 1.0f) + 1;

	pr->tintAmount = parentPr.tintAmount;
	pr->tintAmount += PORTAL_TINT_ADDITION;
	pr->tintAmount -= PORTAL_TINT_ADDITION * portalCloseness;

	return true;
}

void Scene::fillPortalViewDrawInfos(Portal* unclePortal, PortalReference parentPr, int recursionDepth, float totalZoom) {
	// This is a recursive function that decriments the depth each layer of stencils.  eventually it
	// will reach an arbitrarily defined max depth and teminate.
	if (recursionDepth >= MAX_SEEN_PORTALS) {
		return;
	}

	// We gotta reorganize the portals by closeness to the initial camera so 
	// they can be drawin in order, painter style.
	std::vector<PortalReference>portalReferences;
	PortalReference pr;
	for (PortalPair& portalPair : p_portalManager->portalPairs) {
		pr.portal = &portalPair.A;
		pr.sibling = &portalPair.B;
		if (createPortalReference(&pr, parentPr)) {
			portalReferences.push_back(pr);
		}
		pr.portal = &portalPair.B;
		pr.sibling = &portalPair.A;
		if (createPortalReference(&pr, parentPr)) {
			portalReferences.push_back(pr);
		}
	}
	// For now we can make the portals draw in order by sorthing them based on how close they are to the player.
	// A better sorting/culling algo would be great but I dont know how to do it yet!
	std::sort(portalReferences.begin(), portalReferences.end(), comparePortalReferences);

	for (PortalReference& pr : portalReferences) {
		// Last time this function was called, the parent portal was translated to it's siblings position.
		// Now that we are inside the parent, we dont want to try and draw it's sibling, or "Uncle," as
		// it would lie on top of the parent portal, screwing up the visuals.
		if (pr.portal != unclePortal) {
			stencilDrawInfos.push_back(StencilDrawInfo(pr.stencil, frameStencilVal, randColor()));

			portalViewDrawInfos.push_back(PortalViewDrawInfo(pr.transf, frameStencilVal,
				totalZoom * pr.portal->siblingScaleDif, std::min(pr.tintAmount, 1.0f)));

			frameStencilVal++;

			// no sense continuing deeper into the recursive call if the tint is already opaque:
			if (pr.tintAmount < 1.0f) {
				pr.stencilVal = frameStencilVal;
				fillPortalViewDrawInfos(pr.sibling, pr, recursionDepth + 1, totalZoom * pr.portal->siblingScaleDif);
			}
		}
	}
}

void Scene::drawPortalStencils() {
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilMask(0xFF); // enable writing to the stencil buffer
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(stencilVAO);
	glBindBuffer(GL_ARRAY_BUFFER, stencilVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stencilEBO);

	p_shaderManager->stencilShader.use();
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	for (int i = 0; i < stencilDrawInfos.size(); i++) {
		drawStencil(stencilDrawInfos[i].stencil, stencilDrawInfos[i].stencilVal, stencilDrawInfos[i].stencilColor);
	}
}

void Scene::drawStencil(std::vector<glm::vec2>& stencilFrustum, int stencilVal, glm::vec3 color) {
	glStencilFunc(GL_ALWAYS, stencilVal, 0xFF);

	GLuint inColorID = glGetUniformLocation(p_shaderManager->stencilShader.ID, "inColor");
	glUniform3f(inColorID, color.r, color.g, color.b);

	std::vector<GLfloat> stencilVerts;
	for (glm::vec2& vec : stencilFrustum) {
		stencilVerts.push_back(vec.x);
		stencilVerts.push_back(vec.y);
	}
	glBufferData(GL_ARRAY_BUFFER, stencilVerts.size() * sizeof(GLfloat), stencilVerts.data(), GL_DYNAMIC_DRAW);

	std::vector<GLsizei> indices;
	for (int i = 0; i < stencilFrustum.size() - 2; i++) {
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i + 2);
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
}

void Scene::drawPortalViews() {
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClearStencil(0);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilMask(0x00); // disable writing to the stencil buffer
	glDisable(GL_BLEND);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// Configure the Vertex Attribute so that OpenGL knows how to read the VBO
	setVertAttribVec3PosVec3NormVec3ColorVec2TextCoord();
	p_shaderManager->defaultShader.use();

	// Bind a mat4 to the shader object for use in shader.vert:
	GLuint lightPosID = glGetUniformLocation(p_shaderManager->defaultShader.ID, "inLightPos");
	GLuint camPosID = glGetUniformLocation(p_shaderManager->defaultShader.ID, "inCamPos");

	// create a moving light source:
	//glm::vec3 lightPos(3 * cos(timeValue), 3 * sin(timeValue * 2), -4);
	glm::vec3 lightPos(1, 1, -4);
	//lightPos += glm::vec3(posOffset, 0);

	glUniform3f(lightPosID, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(camPosID, p_camera->viewVec.x, p_camera->viewVec.y, p_camera->viewVec.z);

	glm::mat4 playerModelMat(1);
	playerModelMat[3][0] = p_camera->viewPlanePos.x;
	playerModelMat[3][1] = -p_camera->viewPlanePos.y;
	playerModelMat[3][2] = -2;
	glm::mat4 scaleAdj = glm::scale(glm::mat4(1), glm::vec3(p_portalManager->scaleChange, p_portalManager->scaleChange, p_portalManager->scaleChange));
	//models[0].modelMatrix = playerModelMat * scaleAdj;
	models[0].modelMatrix = glm::inverse(p_camera->translationMatrix) * scaleAdj;

	for (int i = 0; i < portalViewDrawInfos.size(); i++) {
		drawScenePiece(portalViewDrawInfos[i]);
	}
}

void Scene::drawScenePiece(PortalViewDrawInfo pvdi) {
	float timeValue = (float)glfwGetTime();

	GLuint transfMatrixID = glGetUniformLocation(p_shaderManager->defaultShader.ID, "inTransfMatrix");
	GLuint modelMatrixID = glGetUniformLocation(p_shaderManager->defaultShader.ID, "inModelMatrix");
	GLuint blendAmountID = glGetUniformLocation(p_shaderManager->defaultShader.ID, "inBlendAmount");

	glStencilFunc(GL_EQUAL, pvdi.stencilVal, 0xFF);
	glUniformMatrix4fv(transfMatrixID, 1, GL_FALSE, glm::value_ptr(p_camera->transfMatrix * pvdi.transf));
	glUniform1f(blendAmountID, pvdi.tint);

	// Finally bind the vertex info and draw the object to the window:
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	float z = 1 / p_camera->getScaledZoom() / pvdi.zoomAdj * 10;
	glm::vec3 transl = p_camera->viewPlanePos;
	transl.y *= -1;
	glm::mat4 translate = glm::translate(glm::mat4(1), transl);
	glm::mat4 zoom = glm::scale(glm::mat4(1), glm::vec3(z, z, z));
	models[2].modelMatrix = translate * zoom;

	for (Model& m : models) {
		glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(m.modelMatrix));
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m.verts.size(), m.verts.data(), GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * m.indices.size(), m.indices.data(), GL_DYNAMIC_DRAW);
		glDrawElements(GL_TRIANGLES, (GLsizei)m.indices.size(), GL_UNSIGNED_INT, 0);
	}
}

void Scene::draw() {
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glViewport(0, 0, sceneSize.x, sceneSize.y);

	// Draw the initial scene before any portal shenanigans:
	glm::mat4 playerModelMat(1);
	playerModelMat[3][0] = p_camera->viewPlanePos.x;
	playerModelMat[3][1] = -p_camera->viewPlanePos.y;
	playerModelMat[3][2] = -2;
	glm::mat4 scaleAdj = glm::scale(glm::mat4(1), glm::vec3(p_portalManager->scaleChange, p_portalManager->scaleChange, p_portalManager->scaleChange));
	models[0].modelMatrix = glm::inverse(p_camera->translationMatrix) * scaleAdj;
	drawScenePiece(PortalViewDrawInfo(glm::mat4(1), 0, 1.0f, 0.0f));

	drawPortalStencils();
	drawPortalViews();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, sceneFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, sceneSize.x, sceneSize.y,
		0, 0, WindowSize.x, WindowSize.y,
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
}