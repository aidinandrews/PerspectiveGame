#pragma once
#include <iostream>
#include <vector>
#ifndef GLAD_INCLUDED
#include <glad/glad.h>
#endif
#include <string>
#include <fstream>
#include <sstream>
#include<GLFW/glfw3.h>

struct Program {
	GLuint ID;

	// constructor reads and builds the shader
	Program() : ID(0) {}
	Program(const char* vertexPath, const char* fragmentPath) {
		init(vertexPath, fragmentPath);
	}
	void init(const char* vertexPath, const char* fragmentPath);

	void init2d3rdPersonPov();

	// use/activate the shader
	void use() { glUseProgram(ID); }

	// utility uniform functions
	void setUniformIndex(const std::string& name, int value) const {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
};

struct ShaderManager {
	Program defaultShader;
	Program stencilShader;
	Program simpleShader;
	Program justVertsAndColors;
	Program POV2D3rdPerson;
	Program POV3D3rdPerson;
	std::vector<GLuint> texIDs;

	void init() {
		defaultShader.init("shaders/default.vert", "shaders/default.frag");
		stencilShader.init("shaders/stencil.vert", "shaders/stencil.frag");
		simpleShader.init("shaders/simple.vert", "shaders/simple.frag");
		justVertsAndColors.init("shaders/passthrough.vert", "shaders/empty.frag");
		POV2D3rdPerson.init2d3rdPersonPov();
		POV3D3rdPerson.init("shaders/3D3rdPersonPOV.vert", "shaders/3D3rdPersonPOV.frag");
	}

	~ShaderManager() {
		glDeleteProgram(defaultShader.ID);
		glDeleteProgram(stencilShader.ID);
		glDeleteProgram(simpleShader.ID);
		glDeleteProgram(justVertsAndColors.ID);
		glDeleteProgram(POV2D3rdPerson.ID);
	}
};