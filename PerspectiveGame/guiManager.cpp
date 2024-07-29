#include "guiManager.h"

void GuiManager::imGuiSetup() {
#ifdef USE_GUI_WINDOW

	show_demo_window = false;
	show_another_window = false;
	clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	f = 0.0f;
	leftEdit = 0.0f;
	rightEdit = 0.0f;
	topEdit = 0.0f;
	bottomEdit = 0.0f;

	// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char *glsl_version = "#version 100";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
	// GL 3.2 + GLSL 150
	const char *glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char *glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

		 // Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	glfwMakeContextCurrent(p_imGuiWindow);
	ImGui::CreateContext();
	ImGui::SetCurrentContext(ImGui::GetCurrentContext());

	io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(p_imGuiWindow, true);

	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	// - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != nullptr);

#endif
}

GuiManager::GuiManager(GLFWwindow *w, GLFWwindow *imgw, ShaderManager *sm, InputManager *im, Camera *c, TileManager *tm,
					   Framebuffer *fb, ButtonManager *bm, CurrentSelection*cs)
	: p_window(w), p_imGuiWindow(imgw), p_shaderManager(sm), p_inputManager(im), p_camera(c), p_tileManager(tm), p_framebuffer(fb),
	p_buttonManager(bm), p_currentSelection(cs) {

	imGuiSetup();

	GUI_EDGE_COLOR = glm::vec3(1, 0, 1);
}

GuiManager::~GuiManager() {
#ifdef USE_GUI_WINDOW
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
#endif
}

void GuiManager::renderImGuiDebugWindows() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	io = ImGui::GetIO();


	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
	{
		static int counter = 1;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		if (1) {
			ImGui::SliderFloat("left", &leftEdit, -100.0f, 100.0f);
			ImGui::SliderFloat("right", &rightEdit, -3.0f, 3.0f);
			ImGui::SliderFloat("bottom", &bottomEdit, -3.0f, 3.0f);
			ImGui::SliderFloat("top", &topEdit, -3.0f, 3.0f);
		} else {
			ImGui::InputFloat("left", &leftEdit, -3.0f, 3.0f);
			ImGui::InputFloat("right", &rightEdit, -3.0f, 3.0f);
			ImGui::InputFloat("bottom", &bottomEdit, -3.0f, 3.0f);
			ImGui::InputFloat("top", &topEdit, -3.0f, 3.0f);
		}

		ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

		ImGui::Checkbox("edit tiles", &p_currentSelection->canEditTiles);
		ImGui::Checkbox("edit bases", &p_currentSelection->canEditBases);
		ImGui::Checkbox("edit entities", &p_currentSelection->canEditEntities);
		ImGui::Checkbox("edit sub-windows", &CanEditSubWindows);

		const char* basisLabels[] = { 
			"NONE",
			"BASIS_PRODUCER",
			"BASIS_CONSUMER",
			"BASIS_FORCE_SINK",
		};
		static int heldBasisIndex = 2;
		ImGui::ListBox("Held Basis", &heldBasisIndex, basisLabels, IM_ARRAYSIZE(basisLabels), 4);
		p_currentSelection->heldBasis.type = Tile::Basis::Type(heldBasisIndex);

		const char* entityLabels[] = { 
			"NONE",

			"MATERIAL_OMNI",
			"MATERIAL_A",
			"MATERIAL_B",

			"BUILDING_COMPRESSOR",
			"BUILDING_FORCE_BLOCK",
			"BUILDING_FORCE_MIRROR", 
		};
		static int heldEntityIndex = 5;
		ImGui::ListBox("Held Entity", &heldEntityIndex, entityLabels, IM_ARRAYSIZE(entityLabels), 7);
		p_currentSelection->heldEntity.type = Entity::Type(heldEntityIndex);
		ImGui::ColorEdit3("Preview Tile Color", (float *)&p_currentSelection->addTileColor); // Edit 3 floats representing a color

		// Buttons return true when clicked (most widgets return true when edited/activated)
		if (ImGui::Button("Preview Tile Orientation")) {
			counter = (counter + 1) % 3;
		}
		ImGui::SameLine();
		switch (counter) {
		case 0: ImGui::Text("UP"); 
			p_currentSelection->addTileRelativeOrientation = CurrentSelection::RELATIVE_TILE_ORIENTATION_UP;
			break;
		case 1: ImGui::Text("FLAT");
			p_currentSelection->addTileRelativeOrientation = CurrentSelection::RELATIVE_TILE_ORIENTATION_FLAT;
			break;
		case 2: ImGui::Text("DOWN");
			p_currentSelection->addTileRelativeOrientation = CurrentSelection::RELATIVE_TILE_ORIENTATION_DOWN;
			break;
		}

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", FrameTime, FPS);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window) {
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GuiManager::drawColoredRectFromPixelSpace(glm::ivec2 pos, glm::ivec2 size, glm::vec3 color) {
	glm::vec2 corners[4] = {
		pixelSpaceToWindowSpace(pos),
		pixelSpaceToWindowSpace(pos + glm::ivec2(size.x, 0)),
		pixelSpaceToWindowSpace(pos + size),
		pixelSpaceToWindowSpace(pos + glm::ivec2(0, size.y)),
	};
	drawColoredRect(corners, color);
}

// Assumes screen space:
void GuiManager::drawColoredRect(glm::vec2 corners[4], glm::vec3 color) {
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	glBindVertexArray(p_framebuffer->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, p_framebuffer->VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_framebuffer->EBO);

	setVertAttribVec2PosVec2TexCoordVec3Color();
	p_shaderManager->justVertsAndColors.use();

	std::vector<GLfloat> verts;

	for (int i = 0; i < 4; i++) {
		// position of vertex:
		verts.push_back(corners[i].x);
		verts.push_back(corners[i].y);
		// texture coordinates at this vertex:
		verts.push_back(0);
		verts.push_back(0);
		// color of this vertex:
		verts.push_back(color.r);
		verts.push_back(color.g);
		verts.push_back(color.b);
	}
	std::vector<GLuint> indices = {
		0, 1, 3, 1, 2, 3,
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verts.size(), verts.data(), GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
}

void GuiManager::drawColoredTriFromPixelSpace(glm::ivec2 A, glm::ivec2 B, glm::ivec2 C, glm::vec3 color) {
	glm::vec2 corners[3] = {
		pixelSpaceToWindowSpace(A),
		pixelSpaceToWindowSpace(B),
		pixelSpaceToWindowSpace(C),
	};
	drawColoredTri(corners, color);
}

void GuiManager::setupFramebufferForButtonRender(int buttonIndex, GLuint textureID) {
	p_framebuffer->bind_framebuffer();
	Button *button = &p_buttonManager->buttons[buttonIndex];
	glm::ivec2 buttonSizePixels = button->size * PixelsPerGuiGridUnit;
	p_framebuffer->rescale_framebuffer((GLsizei)buttonSizePixels.x, (GLsizei)buttonSizePixels.y, textureID);
	glViewport(0, 0, button->pixelWidth(), button->pixelHeight());
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Assumes screen space:
void GuiManager::drawColoredTri(glm::vec2 corners[3], glm::vec3 color) {
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	glBindVertexArray(p_framebuffer->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, p_framebuffer->VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_framebuffer->EBO);

	setVertAttribVec2PosVec2TexCoordVec3Color();
	p_shaderManager->justVertsAndColors.use();

	std::vector<GLfloat> verts;
	for (int i = 0; i < 3; i++) {
		// position of vertex:
		verts.push_back(corners[i].x);
		verts.push_back(corners[i].y);
		// texture coordinates at this vertex:
		verts.push_back(0);
		verts.push_back(0);
		// color of this vertex:
		verts.push_back(color.r);
		verts.push_back(color.g);
		verts.push_back(color.b);
	}
	std::vector<GLuint> indices = {
		0, 1, 2,
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verts.size(), verts.data(), GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
}

void GuiManager::renderTargetButtonMovementElements() {
	int border = GUI_WINDOW_BORDER_EDGE_SIZE;
	int cornerSize = GUI_WINDOW_BORDER_CORNER_SIZE;
	glm::ivec2 pos = p_buttonManager->p_targetButton->pos * PixelsPerGuiGridUnit;
	glm::ivec2 size = p_buttonManager->p_targetButton->size * PixelsPerGuiGridUnit;
	int sizeX = p_buttonManager->p_targetButton->size.x * PixelsPerGuiGridUnit;
	int sizeY = p_buttonManager->p_targetButton->size.y * PixelsPerGuiGridUnit;

	switch (p_buttonManager->targetButtonArea) {
	case BUTTON_AREA_INSIDE:
		drawColoredRectFromPixelSpace(pos, glm::ivec2(sizeX, border), GUI_EDGE_COLOR);
		drawColoredRectFromPixelSpace(pos + glm::ivec2(0, sizeY), glm::ivec2(sizeX, -border), GUI_EDGE_COLOR);
		drawColoredRectFromPixelSpace(pos, glm::ivec2(border, sizeY), GUI_EDGE_COLOR);
		drawColoredRectFromPixelSpace(pos + glm::ivec2(sizeX, 0), glm::ivec2(-border, sizeY), GUI_EDGE_COLOR);
		break;
	case BUTTON_AREA_LEFT_EDGE:
		drawColoredRectFromPixelSpace(pos, glm::ivec2(border, sizeY), GUI_EDGE_COLOR);
		break;
	case BUTTON_AREA_RIGHT_EDGE:
		drawColoredRectFromPixelSpace(pos + glm::ivec2(sizeX, 0), glm::ivec2(-border, sizeY), GUI_EDGE_COLOR);
		break;
	case BUTTON_AREA_TOP_EDGE:
		drawColoredRectFromPixelSpace(pos + glm::ivec2(0, sizeY), glm::ivec2(sizeX, -border), GUI_EDGE_COLOR);
		break;
	case BUTTON_AREA_BOTTOM_EDGE:
		drawColoredRectFromPixelSpace(pos, glm::ivec2(sizeX, border), GUI_EDGE_COLOR);
		break;
	case BUTTON_AREA_TOP_LEFT_CORNER:
		drawColoredTriFromPixelSpace(pos + glm::ivec2(0, sizeY),
									 pos + glm::ivec2(cornerSize, sizeY),
									 pos + glm::ivec2(0, sizeY - cornerSize),
									 GUI_EDGE_COLOR);
		break;
	case BUTTON_AREA_TOP_RIGHT_CORNER:
		drawColoredTriFromPixelSpace(pos + size,
									 pos + size + glm::ivec2(-cornerSize, 0),
									 pos + size + glm::ivec2(0, -cornerSize),
									 GUI_EDGE_COLOR);
		break;
	case BUTTON_AREA_BOTTOM_RIGHT_CORNER:
		drawColoredTriFromPixelSpace(pos + glm::ivec2(sizeX, 0),
									 pos + glm::ivec2(sizeX - cornerSize, 0),
									 pos + glm::ivec2(sizeX, cornerSize),
									 GUI_EDGE_COLOR);
		break;
	case BUTTON_AREA_BOTTOM_LEFT_CORNER:
		drawColoredTriFromPixelSpace(pos,
									 pos + glm::ivec2(cornerSize, 0),
									 pos + glm::ivec2(0, cornerSize),
									 GUI_EDGE_COLOR);
		break;
	}
}

void GuiManager::draw2d3rdPersonCpuCropping() {
	p_tileManager->windowFrustum.clear();
	glm::vec2 topLeft(glm::inverse(p_camera->transfMatrix) * glm::vec4(-1, +1, 0, 1));
	glm::vec2 topRight(glm::inverse(p_camera->transfMatrix) * glm::vec4(+1, +1, 0, 1));
	glm::vec2 bottomRight(glm::inverse(p_camera->transfMatrix) * glm::vec4(+1, -1, 0, 1));
	glm::vec2 bottomLeft(glm::inverse(p_camera->transfMatrix) * glm::vec4(-1, -1, 0, 1));
	p_tileManager->windowFrustum.push_back(topLeft);
	p_tileManager->windowFrustum.push_back(topRight);
	p_tileManager->windowFrustum.push_back(bottomRight);
	p_tileManager->windowFrustum.push_back(bottomLeft);
	p_tileManager->draw2D3rdPerson();
}

void GuiManager::draw2d3rdPersonGpuRaycasting() {
	// by this time we have updated the camera transformation matrix
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	glBindVertexArray(p_framebuffer->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, p_framebuffer->VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_framebuffer->EBO);
	Button *sceneView = &p_buttonManager->buttons[ButtonManager::pov2d3rdPersonViewButtonIndex];

	setVertAttribVec2Pos();
	p_shaderManager->POV2D3rdPerson.use();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, p_tileManager->tileInfosBufferID);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
				 p_tileManager->tileGpuInfos.size() * sizeof(TileGpuInfo),
				 p_tileManager->tileGpuInfos.data(),
				 GL_DYNAMIC_DRAW);

	GLuint tileInfosBlockID = glGetUniformBlockIndex(p_shaderManager->POV2D3rdPerson.ID, "tileInfosBuffer");
	GLuint tileInfosBindingPoint = 1;
	glUniformBlockBinding(p_shaderManager->POV2D3rdPerson.ID, tileInfosBlockID, tileInfosBindingPoint);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, tileInfosBindingPoint, p_tileManager->tileInfosBufferID);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind.

	GLuint deltaTimeID = glGetUniformLocation(p_shaderManager->POV2D3rdPerson.ID, "deltaTime");
	glUniform1f(deltaTimeID, TimeSinceProgramStart);

	GLuint updateProgressID = glGetUniformLocation(p_shaderManager->POV2D3rdPerson.ID, "updateProgress");
	glUniform1f(updateProgressID, float(TimeSinceProgramStart - p_tileManager->lastUpdateTime) / UpdateTime);

	GLuint initialTileIndexID = glGetUniformLocation(p_shaderManager->POV2D3rdPerson.ID, "initialTileIndex");
	glUniform1i(initialTileIndexID, p_tileManager->povTile.tile->index);

	GLuint initialTileSide0IndexID = glGetUniformLocation(p_shaderManager->POV2D3rdPerson.ID, "initialSideIndex");
	glUniform1i(initialTileSide0IndexID, p_tileManager->povTile.initialSideIndex);

	GLuint initialTileTexCoord0IndexID = glGetUniformLocation(p_shaderManager->POV2D3rdPerson.ID, "initialTexCoordIndex");
	glUniform1i(initialTileTexCoord0IndexID, p_tileManager->povTile.initialVertIndex);

	GLuint initialTileSideOffsetID = glGetUniformLocation(p_shaderManager->POV2D3rdPerson.ID, "initialSideOffset");
	glUniform1i(initialTileSideOffsetID, p_tileManager->povTile.sideInfosOffset);

	GLuint povPosID = glGetUniformLocation(p_shaderManager->POV2D3rdPerson.ID, "inPovPosWindowSpace");
	glUniform2f(povPosID, 0, 0);

	// Pack relative position data into a mat4:
	glm::mat4 relativePosData = {
		p_tileManager->relativePos[0].x, p_tileManager->relativePos[0].y, p_tileManager->relativePosTileIndices[0],	p_tileManager->relativePos[4].x,
		p_tileManager->relativePos[1].x, p_tileManager->relativePos[1].y, p_tileManager->relativePosTileIndices[1],	p_tileManager->relativePos[4].y,
		p_tileManager->relativePos[2].x, p_tileManager->relativePos[2].y, p_tileManager->relativePosTileIndices[2],	p_tileManager->relativePosTileIndices[4],
		p_tileManager->relativePos[3].x, p_tileManager->relativePos[3].y, p_tileManager->relativePosTileIndices[3],	0,
	};
	GLuint povRelativePosID = glGetUniformLocation(p_shaderManager->POV2D3rdPerson.ID, "inPovRelativePositions");
	glUniformMatrix4fv(povRelativePosID, 1, GL_FALSE, glm::value_ptr(relativePosData));

	glm::mat4 screenSpaceToWorldSpace = glm::inverse(p_camera->getProjectionMatrix((float)sceneView->pixelWidth(), 
																				   (float)sceneView->pixelHeight()));
	GLuint windowToWorldID = glGetUniformLocation(p_shaderManager->POV2D3rdPerson.ID, "inWindowToWorldSpace");
	glUniformMatrix4fv(windowToWorldID, 1, GL_FALSE, glm::value_ptr(screenSpaceToWorldSpace));

	// full screen quad:
	std::vector<GLfloat> verts = { -1, 1, 1, 1, 1, -1, -1, -1, };
	std::vector<GLsizei> indices = { 0, 1, 3, 1, 2, 3, };

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, p_tileManager->texID);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verts.size(), verts.data(), GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GuiManager::draw2d3rdPerson() {
	setupFramebufferForButtonRender(ButtonManager::pov2d3rdPersonViewButtonIndex, 
									p_framebuffer->pov2D3rdPersonTextureID);

	switch (renderType2d3rdPerson) {
	case cpuCropping: draw2d3rdPersonCpuCropping(); break;
	case gpu2dRayCasting: draw2d3rdPersonGpuRaycasting(); break;
	}

	if (p_currentSelection->canEditTiles) {
		setVertAttribVec3PosVec3NormVec3ColorVec2TextCoord1Index();
		p_shaderManager->simpleShader.use();
		p_tileManager->drawAddTilePreview();
	}

	p_framebuffer->unbind_framebuffer();

	// Render off-screen buffer to window:
	p_buttonManager->renderButton(p_buttonManager->buttons[p_buttonManager->pov2d3rdPersonViewButtonIndex], 
								  p_framebuffer->pov2D3rdPersonTextureID);
}

void GuiManager::draw3d3rdPerson() {
	setupFramebufferForButtonRender(ButtonManager::pov3d3rdPersonViewButtonIndex, 
									p_framebuffer->pov3D3rdPersonTextureID);
	
	p_tileManager->draw3Dview();
	//p_tileManager->drawPlayerPos();

	p_framebuffer->unbind_framebuffer();

	p_buttonManager->renderButton(p_buttonManager->buttons[p_buttonManager->pov3d3rdPersonViewButtonIndex],
								  p_framebuffer->pov3D3rdPersonTextureID);
}

void GuiManager::render() {
	/*for (Button &b : buttons) {
		renderButton(b);
	}*/

	draw2d3rdPerson();
	draw3d3rdPerson();

	if (p_buttonManager->p_targetButton != nullptr) {
		renderTargetButtonMovementElements();
	}
}