#pragma once
#include"dependancyHeaders.h"
#include"shaderManager.h"

struct aaTexture {
	GLuint ID;
	GLenum type;

	aaTexture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType);

	void texUnit(Program& shader, const char* uniform, GLuint unit);
	void bind() { glBindTexture(type, ID); }
	void unbind() { glBindTexture(type, 0); }
	void destroy() { glDeleteTextures(1, &ID); }
};