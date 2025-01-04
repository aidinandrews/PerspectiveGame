#pragma once
#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

#include "vectorHelperFunctions.h"

inline glm::vec2 imageSpace(float r, float g, float b, int cubeWidth)
{
	float cubeStep = 1.0f / float(cubeWidth);
	r /= float(cubeWidth);
	g /= float(cubeWidth);

	int blueScaled = int(b * 100);
	float blueW = float(blueScaled % 25) * cubeStep;
	float blueH = float(blueScaled / 25) * cubeStep;

	float x = (r + blueW);
	float y = (g + blueH);
	return glm::vec2(x, y);
}

// Aidin's Basic Shape Creator namespace:
namespace absc {
	struct ShapeInfo {
		std::vector<glm::vec3>verts;
		std::vector<glm::vec3>normals;
		std::vector<GLuint>indices;

		void addTriIndices(int a, int b, int c)
		{
			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(c);
		}
		void addTriVerts(glm::vec3 a, glm::vec3 b, glm::vec3 c)
		{
			verts.push_back(a);
			verts.push_back(b);
			verts.push_back(c);
		}
		void addQuad(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 norm)
		{
			int offset = (int)verts.size();
			indices.push_back(offset + 0);
			indices.push_back(offset + 1);
			indices.push_back(offset + 2);
			indices.push_back(offset + 3);
			indices.push_back(offset + 2);
			indices.push_back(offset + 1);
			verts.push_back(a);
			verts.push_back(b);
			verts.push_back(c);
			verts.push_back(d);
			normals.push_back(norm);
			normals.push_back(norm);
			normals.push_back(norm);
			normals.push_back(norm);
		}

		void addselfTo(std::vector<GLfloat>* glVerts, std::vector<GLuint>* glIndices,
					   glm::vec3 color, glm::vec2 texCoords)
		{

			GLuint indexOffset = (GLuint)glVerts->size() / 11;
			for (int i = 0; i < verts.size(); i++) {
				glVerts->push_back(verts[i].x);
				glVerts->push_back(verts[i].y);
				glVerts->push_back(verts[i].z);
				glVerts->push_back(normals[i].x);
				glVerts->push_back(normals[i].y);
				glVerts->push_back(normals[i].z);
				glVerts->push_back(color.x);
				glVerts->push_back(color.y);
				glVerts->push_back(color.z);
				glVerts->push_back(0.0f);
				glVerts->push_back(0.0f);
			}
			for (GLuint index : indices) {
				glIndices->push_back(index + indexOffset);
			}
		}

		void addSelfTo(std::vector<glm::vec3> inVerts, std::vector<glm::vec3> inNormals,
					   std::vector<GLuint> inIndices, glm::vec3 color,
					   std::vector<GLfloat>* glVerts, std::vector<GLuint>* glIndices)
		{

			clear();
			for (int i = 0; i < inVerts.size(); i++) {
				glVerts->push_back(inVerts[i].x);
				glVerts->push_back(inVerts[i].y);
				glVerts->push_back(inVerts[i].z);
				glVerts->push_back(inNormals[i].x);
				glVerts->push_back(inNormals[i].y);
				glVerts->push_back(inNormals[i].z);
				glVerts->push_back(color.x);
				glVerts->push_back(color.y);
				glVerts->push_back(color.z);
				glVerts->push_back(0.0f);
				glVerts->push_back(0.0f);
			}
			for (GLuint index : indices) {
				glIndices->push_back(index);
			}
		}

		void clear()
		{
			verts.clear();
			indices.clear();
			normals.clear();
		}
	};

	inline void pointCloud(std::vector<glm::vec3> verts, ShapeInfo& si)
	{
		int offset = (int)si.verts.size();
		for (glm::vec3 v : verts) {
			si.verts.push_back(v);
			si.normals.push_back(glm::vec3(0, 0, 1));
		}
		for (int i = 0; i < verts.size(); i++) {
			si.indices.push_back(offset + i);
		}
	}

	inline void floor(float width, float length, int density, ShapeInfo& si)
	{
		int offset = (int)si.verts.size();
		float widthStep = width / density;
		float lengthStep = length / density;
		for (int i = 0; i < density; i++) {
			for (int j = 0; j < density; j++) {
				si.verts.push_back(glm::vec3(j * widthStep, i * widthStep, 0));
				si.normals.push_back(glm::vec3(0, 0, 1));
			}
		}
		for (int i = 0; i < density - 1; i++) {
			for (int j = 0; j < density - 1; j++) {
				si.addTriIndices(
					offset + j + (density * i),
					offset + j + (density * i) + 1,
					offset + j + (density * (i + 1)));
				si.addTriIndices(
					offset + j + (density * (i + 1)) + 1,
					offset + j + (density * (i + 1)),
					offset + j + (density * i) + 1);
			}
		}

	}

	inline void guiButton(float width, float height, glm::vec2 pos, ShapeInfo& si)
	{
		int offset = (int)si.verts.size();
		si.addQuad(
			glm::vec3(pos, 0),
			glm::vec3(pos.x + width, pos.y, 0),
			glm::vec3(pos.x + width, pos.y + height, 0),
			glm::vec3(pos.x, pos.y + height, 0),
			glm::vec3(0, 0, 1));
		// indices:
		si.addTriIndices(
			offset,
			offset + 1,
			offset + 2);
		si.addTriIndices(
			offset,
			offset + 3,
			offset + 1);
	}

	inline void poly2D(float width, float height, float zOffset, int sideCount, ShapeInfo& si)
	{
		float radians = float(M_PI * 2.0f) / float(sideCount);
		int offset = (int)si.verts.size();
		for (int i = 0; i < sideCount; i++) {
			// verts:
			float angle = float(i) * radians;
			si.verts.push_back(glm::vec3(cos(angle) * width, sin(angle) * height, zOffset));
			// normals:
			si.normals.push_back(glm::vec3(0, 0, 1));
		}
		// indices:
		for (int i = 1; i < sideCount - 1; i++) {
			si.addTriIndices(
				offset,
				offset + i,
				offset + i + 1);
		}
	}

	inline void inversePoly2D(float width, float height, float zOffset, int sideCount, ShapeInfo& si)
	{
		float radians = float(M_PI * 2.0f) / float(sideCount);
		int offset = (int)si.verts.size();
		for (int i = 0; i < sideCount; i++) {
			// verts:
			float angle = float(i) * radians;
			si.verts.push_back(glm::vec3(cos(angle) * width, sin(angle) * height, zOffset));
			// normals:
			si.normals.push_back(glm::vec3(0, 0, -1));
		}
		// indices:
		for (int i = 1; i < sideCount - 1; i++) {
			si.addTriIndices(
				offset,
				i + 1 + offset,
				i + offset);
		}
	}

	inline void prism(glm::vec3 boundingBox, int sideCount, bool smoothShading, ShapeInfo& si)
	{
		int vertOffset = (int)si.verts.size();
		// Make the faces of the prism:
		poly2D(boundingBox.x, boundingBox.y, boundingBox.z / 2, sideCount, si);
		inversePoly2D(boundingBox.x, boundingBox.y, -boundingBox.z / 2, sideCount, si);
		// Stitch the two sides together:
		int indexOffset = (int)si.verts.size();
		if (smoothShading) {
			for (int i = 0; i < sideCount; i++) {
				si.verts.push_back(si.verts[vertOffset + i]);

				float angle = (float(M_PI * 2) / float(sideCount)) * float(i);
				glm::vec3 norm(cos(angle), sin(angle), 0);
				si.normals.push_back(norm);
			}
			for (int i = 0; i < sideCount; i++) {
				si.verts.push_back(si.verts[vertOffset + sideCount + i]);

				glm::vec3 norm(1, 0, 0);
				float angle = (float(M_PI * 2) / float(sideCount)) * float(i);
				norm = vechelp::rotate(norm, angle);
				si.normals.push_back(norm);
			}
			for (int i = 0; i < sideCount; i++) {
				si.addTriIndices(
					indexOffset + i,
					indexOffset + i + sideCount,
					indexOffset + (i + 1) % sideCount
				);
				si.addTriIndices(
					indexOffset + (i + 1) % sideCount,
					indexOffset + i + sideCount,
					indexOffset + (i + 1) % sideCount + sideCount
				);
			}
			return;
		}
		else {
			for (int i = 0; i < sideCount; i++) {
				float angle = ((float(M_PI * 2) / float(sideCount)) * float(i));
				glm::vec3 norm(cos(angle), sin(angle), 0);

				si.addQuad(
					si.verts[vertOffset + i],
					si.verts[vertOffset + i + sideCount],
					si.verts[vertOffset + (i + 1) % sideCount],
					si.verts[vertOffset + (i + 1) % sideCount + sideCount],
					norm);
			}
		}
	}

	inline void torus(float ringWidth, float ringHeight, float torusWidth,
					  float torusHeight, int ringCount, int ringSideCount, float percentOfTorus,
					  float noiseAmount, ShapeInfo& si)
	{
		using namespace vechelp;

		int offset = (int)si.verts.size();
		clip(percentOfTorus, 0.0f, 1.0f);
		srand(NULL);

		for (int i = 0; i < ringCount + 1; i++) {
			float torusAngle = float(i) * (float(2 * M_PI * percentOfTorus) / float(ringCount));
			for (int j = 0; j < ringSideCount; j++) {
				float ringAngle = float(j) * (float(2 * M_PI) / ringSideCount);
				// verts:
				float mildNoise = ((1.0f * rand()) / RAND_MAX) * noiseAmount + 1;
				si.verts.push_back(glm::vec3(
					mildNoise * cos(torusAngle) * (torusWidth + cos(ringAngle) * ringWidth),
					mildNoise * sin(torusAngle) * (torusHeight + cos(ringAngle) * ringWidth),
					mildNoise * sin(ringAngle) * ringHeight));

				// normals:
				glm::vec3 norm = glm::vec3(1, 0, 0);
				norm = rotate(norm, glm::vec3(0, -ringAngle, 0));
				norm = rotate(norm, glm::vec3(0, 0, torusAngle));
				si.normals.push_back(norm);
			}
		}
		// indices:
		for (int i = 0; i < ringCount; i++) {
			int generalOffset = offset + ringSideCount * i;
			int nextRingOffset = offset + ringSideCount * (i + 1);

			for (int j = 0; j < ringSideCount; j++) {
				int nextVertOffset = (j + 1) % ringSideCount;
				si.addTriIndices(
					j + generalOffset,
					j + nextRingOffset,
					nextVertOffset + generalOffset);
				si.addTriIndices(
					nextVertOffset + generalOffset,
					j + nextRingOffset,
					nextVertOffset + nextRingOffset);
			}
		}
		if (percentOfTorus < 1) { // cap the ends:
			int secondCapOffset = (int)si.verts.size() - ringSideCount;

			int cap1Offset = (int)si.verts.size();
			for (int i = 0; i < ringSideCount; i++) {
				si.verts.push_back(si.verts[offset + i]);
				si.normals.push_back(glm::vec3(0, -1, 0));
			}
			int cap2Offset = (int)si.verts.size();
			glm::vec3 secondCapNormal(0, 1, 0);
			rotate(secondCapNormal, float(M_PI * 2 * percentOfTorus));
			for (int i = 0; i < ringSideCount; i++) {
				si.verts.push_back(si.verts[secondCapOffset + i]);
				si.normals.push_back(secondCapNormal);
			}

			for (int i = 1; i < ringSideCount - 1; i++) {
				si.addTriIndices(
					cap1Offset,
					cap1Offset + i,
					cap1Offset + i + 1);
				si.addTriIndices(
					cap2Offset,
					cap2Offset + i + 1,
					cap2Offset + i);
			}
		}
	}

	inline void sphere(glm::vec3 boundingBox, int numRings, int numBands, ShapeInfo& si)
	{
		int offset = (int)si.verts.size();
		// top and bottom verts:
		si.verts.push_back(glm::vec3(0, boundingBox.y / 2, 0));
		si.normals.push_back(glm::vec3(0, 1, 0));
		si.verts.push_back(glm::vec3(0, -boundingBox.y / 2, 0));
		si.normals.push_back(glm::vec3(0, -1, 0));
		// bands connecting top and bottom verts:
		float ringAngle = float(M_PI * 2) / float(numRings);
		float bandAngle = float(M_PI) / float(numBands);

		int offsetNoPolar = (int)si.verts.size();
		for (int i = 0; i < numRings; i++) {
			// because we already have polar verts, we start w/ an offset (bandAngle):
			for (int j = 0; j < numBands - 1; j++) {
				si.verts.push_back(glm::vec3(
					(boundingBox.x / 2) * sin(bandAngle * (j + 1)) * cos(ringAngle * i),
					(boundingBox.y / 2) * cos(bandAngle * (j + 1)),
					(boundingBox.z / 2) * sin(bandAngle * (j + 1)) * sin(ringAngle * i)));

				si.normals.push_back(glm::vec3(
					sin(bandAngle * (j + 1)) * cos(ringAngle * i),
					cos(bandAngle * (j + 1)),
					sin(bandAngle * (j + 1)) * sin(ringAngle * i)));
			}
		}
		for (int i = 0; i < numRings; i++) {
			int bandOffset = (numBands - 1) * i;
			int nextBandOffset = (numBands - 1) * ((i + 1) % numRings);
			// polar triangles:
			si.addTriIndices(
				offsetNoPolar + bandOffset,
				offset,
				offsetNoPolar + nextBandOffset);
			si.addTriIndices(
				offset + 1,
				offset + numBands + bandOffset,
				offset + numBands + nextBandOffset);
			// connections between bands:
			for (int j = 0; j < numBands - 2; j++) {
				si.addTriIndices(
					offsetNoPolar + j + bandOffset + 1,
					offsetNoPolar + j + bandOffset,
					offsetNoPolar + j + nextBandOffset);
				si.addTriIndices(
					offsetNoPolar + j + nextBandOffset + 1,
					offsetNoPolar + j + bandOffset + 1,
					offsetNoPolar + j + nextBandOffset);
			}
		}
	}

	inline void arrow(glm::vec3 boundingBox, float percentArrow, float arrowBodyWidthPercent, ShapeInfo& si)
	{
		glm::vec3 bb = boundingBox;
		glm::vec2 points[7];
		points[0] = glm::vec2(bb.x / 2, bb.y);
		points[1] = glm::vec2(bb.x, bb.y - (bb.y * percentArrow));
		points[2] = glm::vec2((bb.x / 2) + ((bb.x * arrowBodyWidthPercent) / 2), bb.y - (bb.y * percentArrow));
		points[3] = glm::vec2((bb.x / 2) + ((bb.x * arrowBodyWidthPercent) / 2), 0);
		points[4] = glm::vec2((bb.x / 2) - ((bb.x * arrowBodyWidthPercent) / 2), 0);
		points[5] = glm::vec2((bb.x / 2) - ((bb.x * arrowBodyWidthPercent) / 2), bb.y - (bb.y * percentArrow));
		points[6] = glm::vec2(0, bb.y - (bb.y * percentArrow));

		//  /\
		// /  \
		//  ||
		//  ||

		int offset = (int)si.verts.size();
		// push back top and bottom verts/normals:
		for (glm::vec2& point : points) {
			si.verts.push_back(glm::vec3(point - glm::vec2(bb.x / 2, bb.y / 2), bb.z / 2));
			si.normals.push_back(glm::vec3(0, 0, 1));
		}
		for (glm::vec2& point : points) {
			si.verts.push_back(glm::vec3(point - glm::vec2(bb.x / 2, bb.y / 2), -bb.z / 2));
			si.normals.push_back(glm::vec3(0, 0, -1));
		}
		// connect them together:
		si.addTriIndices(offset + 1, offset, offset + 2);
		si.addTriIndices(offset + 2, offset, offset + 5);
		si.addTriIndices(offset + 5, offset, offset + 6);
		si.addTriIndices(offset + 3, offset + 2, offset + 4);
		si.addTriIndices(offset + 4, offset + 2, offset + 5);
		offset += 7;
		si.addTriIndices(offset, offset + 1, offset + 2);
		si.addTriIndices(offset, offset + 2, offset + 5);
		si.addTriIndices(offset, offset + 5, offset + 6);
		si.addTriIndices(offset + 2, offset + 3, offset + 4);
		si.addTriIndices(offset + 2, offset + 4, offset + 5);
		offset -= 7;
		// more verts/normals/indices for side triangles:
		glm::vec3 arrowNormAngle = glm::vec3(points[1] - points[0], 0);
		vechelp::rotate(arrowNormAngle, float(M_PI / 2));
		arrowNormAngle = glm::normalize(arrowNormAngle);
		si.addQuad(si.verts[offset], si.verts[offset + 1], si.verts[offset + 7], si.verts[offset + 8], arrowNormAngle);
		si.addQuad(si.verts[offset + 1], si.verts[offset + 2], si.verts[offset + 8], si.verts[offset + 9], glm::vec3(0, -1, 0));
		si.addQuad(si.verts[offset + 2], si.verts[offset + 3], si.verts[offset + 9], si.verts[offset + 10], glm::vec3(1, 0, 0));
		si.addQuad(si.verts[offset + 3], si.verts[offset + 4], si.verts[offset + 10], si.verts[offset + 11], glm::vec3(0, -1, 0));
		si.addQuad(si.verts[offset + 4], si.verts[offset + 5], si.verts[offset + 11], si.verts[offset + 12], glm::vec3(-1, 0, 0));
		si.addQuad(si.verts[offset + 5], si.verts[offset + 6], si.verts[offset + 12], si.verts[offset + 13], glm::vec3(0, -1, 0));
		si.addQuad(si.verts[offset + 6], si.verts[offset], si.verts[offset + 13], si.verts[offset + 7], glm::vec3(-arrowNormAngle.x, arrowNormAngle.y, 0));
	}
}