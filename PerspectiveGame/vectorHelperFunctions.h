#pragma once
#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

#include"dependancyHeaders.h"

namespace vechelp {

	struct LineSeg2D {
		glm::vec2 A, B;
		LineSeg2D()
		{
			A = glm::vec2(0, 0);
			B = glm::vec2(0, 0);
		}
		LineSeg2D(glm::vec2 a, glm::vec2 b)
		{
			A = a;
			B = b;
		}
	};

	enum SideOfLine {
		INSIDE = 1,
		ON_LINE_SEG = 0,
		OUTSIDE = -1
	};

	float getDist(glm::vec3 A, glm::vec3 B);

	float getDist(glm::vec2 A, glm::vec2 B);

	glm::vec3 rotate(glm::vec3 point, float radians);

	glm::vec2 rotate(glm::vec2 point, float radians);

	glm::vec3 rotate(glm::vec3 point, glm::vec3 angle);

	void rotate(glm::vec3& point, glm::vec3 center, float radians);

	void rotate(glm::vec2& point, glm::vec2 center, float radians);

	float magnitude(glm::vec3 v);

	float angleBetween(glm::vec3 A, glm::vec3 B);

	glm::vec3 randColor();

	void clip(float num, float min, float max);

	void print(glm::vec2 a);
	void print(glm::vec3 a);
	void print(glm::ivec3 a);

	void println(glm::vec2 a);
	void println(glm::ivec3 a);
	void println(glm::vec3 a);

	void printMat4Linear(glm::mat4 m);

	float distToLineSeg(glm::vec2 p, glm::vec2 a, glm::vec2 b, glm::vec2* closestPoint);

	bool isLeft(glm::vec2 point, glm::vec2 lineSegPoint1, glm::vec2 lineSegPoint2);

	inline SideOfLine isLeftSpecific(glm::vec2 point, glm::vec2 lineSegPoint1, glm::vec2 lineSegPoint2)
	{
		float val = (point.x - lineSegPoint1.x) * (lineSegPoint1.y - lineSegPoint2.y)
			- (point.y - lineSegPoint1.y) * (lineSegPoint1.x - lineSegPoint2.x);
		if (val > 0) {
			return INSIDE;
		}
		else if (val < 0) {
			return OUTSIDE;
		}
		else {
			return ON_LINE_SEG;
		}
	}

	// Checking if a point is inside a polygon
	bool point_in_polygon(glm::vec2 point, std::vector<glm::vec2> polygon);

	float getVecAngle(glm::vec2 vec);

	// Calculates the general line formula (ax + by + c = 0) from two points (p1 and p2) 
	// and stores them in the a, b, and c inputs.
	void generalLineFormula(glm::vec2 p1, glm::vec2 p2, float& a, float& b, float& c);

	// Checks what side point p is on of the line segment lsp1->lsp2.
	// Returns true if on left and false if on right.
	bool whatSide(glm::vec2 lsp1, glm::vec2 lsp2, glm::vec2 p);


	// Returns the intersection point of 2D line segments AB and CD.
	glm::vec2 intersection(glm::vec2 A, glm::vec2 B, glm::vec2 C, glm::vec2 D);

	// Check if point q lies on the line segment pr
	bool onSegment(glm::vec2 p, glm::vec2 q, glm::vec2 r);

	// Calculate orientation of triplet (p, q, r)
	int orientation(glm::ivec2 p, glm::ivec2 q, glm::ivec2 r);

	// The main function that returns true if line segment 'p1q1' and 'p2q2' intersect. 
	bool doIntersect(glm::vec2 p1, glm::vec2 q1, glm::vec2 p2, glm::vec2 q2);


	// Crops subjectPoly to cropToPoly.  Both need to be wound clockwise!
	bool cropFrustumToFrustum(std::vector<glm::vec2>& subjectPoly,
									 std::vector<glm::vec2> cropToPoly);

	// Crops subjectPoly to cropToPoly.  Both need to be wound clockwise!
	bool cropTileToFrustum(std::vector<glm::vec2>& subjectPolyVerts,
								  std::vector<glm::vec2>& subjectPolyTexCoords,
								  glm::vec2 cropToPoly[3]);

	// Crops subjectPoly to cropToPoly.  Both need to be wound clockwise!
	bool sutherlandHodgemanPolyCrop(std::vector<glm::vec2>& subjectPoly, std::vector<glm::vec2> cropToPoly,
								   bool removeDuplicatePoints);

	std::vector<glm::vec2> sutherlandHodgemanLineCrop(std::vector<glm::vec2> subjectPoly, std::vector<glm::vec2>* cropToLine,
													 bool removeDuplicatePoints);

	float angleBetween(glm::vec2 A, glm::vec2 B);

	// Returns the maximum of all four vertex's coordinates.  
	// Meant for use finding the maxVert of four vertices that will make up a tile.
	glm::ivec3 getMaxVert(glm::ivec3 A, glm::ivec3 B, glm::ivec3 C, glm::ivec3 D);


}