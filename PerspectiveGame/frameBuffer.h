#pragma once
#include <iostream>
#ifndef GLAD_INCLUDED
#include <glad/glad.h>
#endif
#include <glm/glm.hpp>
#include "vectorHelperFunctions.h"

class Framebuffer {
public:
	GLint width = 800;
	GLint height = 600;
	glm::ivec2 textureSizes[2];
	GLuint VAO;
	GLuint VBO;
	GLuint FBO;
	GLuint RBO;
	GLuint EBO;
	GLuint pov2D3rdPersonTextureID;
	GLuint pov3D3rdPersonTextureID;

public:
	void init() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glGenTextures(1, &pov2D3rdPersonTextureID);
		glBindTexture(GL_TEXTURE_2D, pov2D3rdPersonTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pov2D3rdPersonTextureID, 0);

		glGenTextures(1, &pov3D3rdPersonTextureID);
		glBindTexture(GL_TEXTURE_2D, pov3D3rdPersonTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, pov3D3rdPersonTextureID, 0);

		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	~Framebuffer() {
		glDeleteFramebuffers(1, &FBO);
		glDeleteTextures(1, &pov2D3rdPersonTextureID);
		glDeleteTextures(1, &pov3D3rdPersonTextureID);
		glDeleteRenderbuffers(1, &RBO);
		glDeleteBuffers(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

	void bind_framebuffer() {
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	}

	void unbind_framebuffer() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void rescale_framebuffer(GLsizei width, GLsizei height, GLuint texture_id) {
		glBindTexture(GL_TEXTURE_2D, texture_id);

		int texIndex = 0;
		if (texture_id == pov2D3rdPersonTextureID) {
			texIndex = 0;
		} else if (texture_id == pov3D3rdPersonTextureID) {
			texIndex = 1;
		}

		if (textureSizes[texIndex].x != width || textureSizes[texIndex].y != height) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			textureSizes[texIndex].x = width;
			textureSizes[texIndex].y = height;
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	}
};