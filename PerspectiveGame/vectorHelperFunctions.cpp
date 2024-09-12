#include"vectorHelperFunctions.h"

float vechelp::getDist(glm::vec3 A, glm::vec3 B)
{
	return sqrt(((B.x - A.x) * (B.x - A.x)) + ((B.y - A.y) * (B.y - A.y)) + ((B.z - A.z) * (B.z - A.z)));
}

float vechelp::getDist(glm::vec2 A, glm::vec2 B)
{
	return sqrt(((B.x - A.x) * (B.x - A.x)) + ((B.y - A.y) * (B.y - A.y)));
}

glm::vec3 vechelp::rotate(glm::vec3 point, float radians)
{
	glm::vec3 temp(0, 0, 0);
	temp.x = (point.x * cos(radians)) + (point.y * -sin(radians));
	temp.y = (point.x * sin(radians)) + (point.y * cos(radians));
	return temp;
}

glm::vec2 vechelp::rotate(glm::vec2 point, float radians)
{
	glm::vec2 temp;
	temp.x = (point.x * cos(radians)) + (point.y * -sin(radians));
	temp.y = (point.x * sin(radians)) + (point.y * cos(radians));
	return temp;
}

glm::vec3 vechelp::rotate(glm::vec3 point, glm::vec3 angle)
{
	glm::vec3 temp;
	temp.x = (point.x * cos(angle.z)) - (point.y * sin(angle.z));
	temp.y = (point.x * sin(angle.z)) + (point.y * cos(angle.z));
	temp.z = point.z;
	point = temp;
	temp.x = (point.x * cos(angle.y)) + (point.z * sin(angle.y));
	temp.y = point.y;
	temp.z = (-point.x * sin(angle.y)) + (point.z * cos(angle.y));
	point = temp;
	temp.x = point.x;
	temp.y = (point.y * cos(angle.x)) - (point.z * sin(angle.x));
	temp.z = (point.y * sin(angle.x)) + (point.z * cos(angle.x));
	return temp;
}

void vechelp::rotate(glm::vec3& point, glm::vec3 center, float radians)
{
	glm::vec2 temp;
	point -= center;
	temp.x = (point.x * cos(radians)) + (point.y * -sin(radians));
	temp.y = (point.x * sin(radians)) + (point.y * cos(radians));
	point = glm::vec3(temp.x, temp.y, 0);
	point += center;
}

void vechelp::rotate(glm::vec2& point, glm::vec2 center, float radians)
{
	glm::vec2 temp;
	point -= center;
	temp.x = (point.x * cos(radians)) + (point.y * -sin(radians));
	temp.y = (point.x * sin(radians)) + (point.y * cos(radians));
	point = glm::vec2(temp.x, temp.y);
	point += center;
}

float vechelp::magnitude(glm::vec3 v)
{
	return (float)sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float vechelp::angleBetween(glm::vec3 A, glm::vec3 B)
{
	return acos(glm::dot(A, B) / (magnitude(A) * magnitude(B)));
}

glm::vec3 vechelp::randColor()
{
	return glm::vec3(1.0f * rand() / RAND_MAX, 1.0f * rand() / RAND_MAX, 1.0f * rand() / RAND_MAX);
}

void vechelp::clip(float num, float min, float max)
{
	if (num < min) {
		num = min;
		return;
	}
	else if (num > max) {
		num = max;
		return;
	}
}

void vechelp::print(glm::vec3 a)
{
	std::cout << "(" << a.x << ", " << a.y << ", " << a.z << ")";
}
void vechelp::println(glm::vec3 a)
{
	std::cout << "(" << a.x << ", " << a.y << ", " << a.z << ")\n";
}
void vechelp::print(glm::ivec3 a) { print((glm::vec3)a); }
void vechelp::println(glm::ivec3 a) { println((glm::vec3)a); }

void vechelp::println(glm::vec2 a)
{
	std::cout << "(" << a.x << ", " << a.y << ")\n";
}

void vechelp::print(glm::vec2 a)
{
	std::cout << "(" << a.x << ", " << a.y << ") ";
}

void vechelp::printMat4Linear(glm::mat4 m)
{
	std::cout
		<< m[0][0] << ", " << m[1][0] << ", " << m[2][0] << ", "
		<< m[0][1] << ", " << m[1][1] << ", " << m[2][1] << ", "
		<< m[0][2] << ", " << m[1][2] << ", " << m[2][2] << "\n";
}

float vechelp::distToLineSeg(glm::vec2 p, glm::vec2 a, glm::vec2 b, glm::vec2* closestPoint)
{
	glm::vec2 ab = b - a;
	glm::vec2 ap = p - a;

	float proj = glm::dot(ap, ab);
	float abLengSquared = ab.x * ab.x + ab.y * ab.y;
	float d = proj / abLengSquared;

	if (d <= 0) {
		if (closestPoint != nullptr) {
			*closestPoint = a;
		}
		return glm::distance(p, a);
	}
	else if (d >= 1) {
		if (closestPoint != nullptr) {
			*closestPoint = b;
		}
		return glm::distance(p, b);
	}
	else {
		glm::vec2 cp = a + ab * d;
		if (closestPoint != nullptr) {
			*closestPoint = cp;
		}
		return glm::distance(p, cp);
	}
}

bool vechelp::isLeft(glm::vec2 point, glm::vec2 lineSegPoint1, glm::vec2 lineSegPoint2)
{
	float val = (point.x - lineSegPoint1.x) * (lineSegPoint1.y - lineSegPoint2.y)
		- (point.y - lineSegPoint1.y) * (lineSegPoint1.x - lineSegPoint2.x);
	return val > 0;
}

// Checking if a point is inside a polygon
bool vechelp::point_in_polygon(glm::vec2 point, std::vector<glm::vec2> polygon)
{

	int num_vertices = (int)polygon.size();
	double x = point.x, y = point.y;
	bool inside = false;

	// Store the first point in the polygon and initialize
	// the second point
	glm::vec2 p1 = polygon[0], p2;

	// Loop through each edge in the polygon
	for (int i = 1; i <= num_vertices; i++) {
		// Get the next point in the polygon
		p2 = polygon[i % num_vertices];

		if (y > std::min(p1.y, p2.y)) {
			if (y <= std::max(p1.y, p2.y)) {
				if (x <= std::max(p1.x, p2.x)) {

					double x_intersection = (((y - p1.y) * (p2.x - p1.x)) / (p2.y - p1.y)) + p1.x;
					if (p1.x == p2.x || x <= x_intersection) {
						// Flip the inside flag
						inside = !inside;
					}
				}
			}
		}
		// Store the current point as the first point for
		// the next iteration
		p1 = p2;
	}
	return inside;
}

float vechelp::getVecAngle(glm::vec2 vec)
{
	float angle = (float)atan2(vec.x, -vec.y) - ((float)M_PI / 2.0f);
	if (angle < 0) {
		angle += (float)M_PI * 2.0f;
	}
	return angle;
}

// Calculates the general line formula (ax + by + c = 0) from two points (p1 and p2) 
// and stores them in the a, b, and c inputs.
void vechelp::generalLineFormula(glm::vec2 p1, glm::vec2 p2, float& a, float& b, float& c)
{
	a = p1.y - p2.y;
	b = p1.x - p2.x;
	c = p1.x * p2.y - p1.y * p2.x;
}

// Checks what side point p is on of the line segment lsp1->lsp2.
// Returns true if on left and false if on right.
bool vechelp::whatSide(glm::vec2 lsp1, glm::vec2 lsp2, glm::vec2 p)
{
	float dir = (p.x - lsp1.x) * (lsp2.y - lsp1.y) - (p.y - lsp1.y) * (lsp2.x - lsp1.x);
	return dir < 0.0f;
}


// Returns the intersection point of 2D line segments AB and CD.
glm::vec2 vechelp::intersection(glm::vec2 A, glm::vec2 B, glm::vec2 C, glm::vec2 D)
{
	float d = ((D.x - C.x) * (B.y - A.y) - (D.y - C.y) * (B.x - A.x));
	float alpha = ((D.x - C.x) * (C.y - A.y) - (D.y - C.y) * (C.x - A.x)) / d;

	return 	glm::vec2(
		A.x + alpha * (B.x - A.x),
		A.y + alpha * (B.y - A.y));
}

// Check if point q lies on the line segment pr
bool vechelp::onSegment(glm::vec2 p, glm::vec2 q, glm::vec2 r)
{
	return (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
			q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y));
}

// Calculate orientation of triplet (p, q, r)
int vechelp::orientation(glm::ivec2 p, glm::ivec2 q, glm::ivec2 r)
{
	int val = ((q.y - p.y) * (r.x - q.x)) - ((q.x - p.x) * (r.y - q.y));
	if (val == 0) return 0;  // collinear 
	return (val > 0) ? 1 : 2; // clock or counterclock wise 
}

// The main function that returns true if line segment 'p1q1' and 'p2q2' intersect. 
bool vechelp::doIntersect(glm::vec2 p1, glm::vec2 q1, glm::vec2 p2, glm::vec2 q2)
{
	p1 *= 1000;
	q1 *= 1000;
	p2 *= 1000;
	q2 *= 1000;
	int o1 = orientation((glm::ivec2)p1, (glm::ivec2)q1, (glm::ivec2)p2);
	int o2 = orientation((glm::ivec2)p1, (glm::ivec2)q1, (glm::ivec2)q2);
	int o3 = orientation((glm::ivec2)p2, (glm::ivec2)q2, (glm::ivec2)p1);
	int o4 = orientation((glm::ivec2)p2, (glm::ivec2)q2, (glm::ivec2)q1);

	// General case 
	if (o1 != o2 && o3 != o4) { return true; }

	// Special Cases 
	// p1, q1 and p2 are collinear and p2 lies on segment p1q1 
	if (o1 == 0 && onSegment(p1, p2, q1)) { return true; }
	// p1, q1 and q2 are collinear and q2 lies on segment p1q1 
	if (o2 == 0 && onSegment(p1, q2, q1)) { return true; }
	// p2, q2 and p1 are collinear and p1 lies on segment p2q2 
	if (o3 == 0 && onSegment(p2, p1, q2)) { return true; }
	// p2, q2 and q1 are collinear and q1 lies on segment p2q2 
	if (o4 == 0 && onSegment(p2, q1, q2)) { return true; }

	return false;
}


// Crops subjectPoly to cropToPoly.  Both need to be wound clockwise!
bool vechelp::cropFrustumToFrustum(std::vector<glm::vec2>& subjectPoly,
								 std::vector<glm::vec2> cropToPoly)
{
	if (cropToPoly.size() < 3) {
		subjectPoly.clear();
		return false;
	}

	glm::vec2* cropToP1, * cropToP2, * inputP1, * inputP2;
	std::vector<glm::vec2> croppedPoly;
	std::vector<glm::vec2>* inputPoly = &subjectPoly;
	std::vector<glm::vec2>* outputPoly = &croppedPoly;
	bool inputP1Inside, inputP2Inside;

	for (int cropToIndex = 0; cropToIndex < cropToPoly.size(); cropToIndex++) {
		cropToP1 = &cropToPoly[cropToIndex];
		cropToP2 = &cropToPoly[(cropToIndex + 1) % cropToPoly.size()];

		for (int inputIndex = 0; inputIndex < inputPoly->size(); inputIndex++) {
			inputP1 = &(*inputPoly)[inputIndex];
			inputP2 = &(*inputPoly)[(inputIndex + 1) % inputPoly->size()];


			/*bool p1IsCropPoint = glm::distance(*inputP1, *cropToP1) < 0.01f
				|| glm::distance(*inputP1, *cropToP2) < 0.01f;
			bool p2IsCropPoint = glm::distance(*inputP2, *cropToP1) < 0.01f
				|| glm::distance(*inputP2, *cropToP2) < 0.01f;*/
			bool p1IsCropPoint = inputP1 == cropToP1 || inputP1 == cropToP2;
			bool p2IsCropPoint = inputP2 == cropToP1 || inputP2 == cropToP2;

			inputP1Inside = isLeft(*inputP1, *cropToP1, *cropToP2);
			inputP2Inside = isLeft(*inputP2, *cropToP1, *cropToP2);

			if (p1IsCropPoint) {
				outputPoly->push_back(*inputP1);
				continue;
			}
			if (inputP1Inside) {
				outputPoly->push_back(*inputP1);
			}
			if (inputP1Inside != inputP2Inside && !p2IsCropPoint) {
				glm::vec2 inter = intersection(*cropToP1, *cropToP2, *inputP1, *inputP2);
				if (glm::distance(*inputP1, inter) > 0.01f) {
					outputPoly->push_back(inter);
				}
			}
		}
		std::swap(inputPoly, outputPoly);
		outputPoly->clear();
	}
	// Depending on the number of edges in cropToPoly, the inputPly pointer may be ending at
	// either subjectPoly or croppedPoly, so we need to make sure the right one is being output!
	if (inputPoly != &subjectPoly) {
		// if this is the case, we can assume subjectPoly has already been cleared.
		for (glm::vec2 v : croppedPoly) {
			subjectPoly.push_back(v);
		}
	}
	return true;
}

// Crops subjectPoly to cropToPoly.  Both need to be wound clockwise!
bool vechelp::cropTileToFrustum(std::vector<glm::vec2>& subjectPolyVerts,
							  std::vector<glm::vec2>& subjectPolyTexCoords,
							  glm::vec2 cropToPoly[3])
{
	if (cropToPoly[0] == cropToPoly[1]) {
		return false;
	}

	glm::vec2* cropToP1, * cropToP2, * inputP1, * inputP2;
	std::vector<glm::vec2>* inputPolyVerts = &subjectPolyVerts;
	std::vector<glm::vec2>* inputPolyTexCoords = &subjectPolyTexCoords;
	std::vector<glm::vec2> croppedPolyVerts;
	std::vector<glm::vec2> croppedPolyTexCoords;
	std::vector<glm::vec2>* outputPolyVerts = &croppedPolyVerts;
	std::vector<glm::vec2>* outputPolyTexCoords = &croppedPolyTexCoords;
	bool inputP1Inside, inputP2Inside;

	for (int cropToIndex = 0; cropToIndex < 2; cropToIndex++) {
		cropToP1 = &cropToPoly[cropToIndex];
		cropToP2 = &cropToPoly[cropToIndex + 1];

		for (int inputIndex = 0; inputIndex < inputPolyVerts->size(); inputIndex++) {
			inputP1 = &(*inputPolyVerts)[inputIndex];
			inputP2 = &(*inputPolyVerts)[(inputIndex + 1) % inputPolyVerts->size()];

			/*if (inputP1 == cropToP1 || inputP1 == cropToP2) {
				outputPolyVerts->push_back(*inputP1);
				outputPolyTexCoords->push_back((*inputPolyTexCoords)[inputIndex]);
				continue;
			}*/

			inputP1Inside = isLeft(*inputP1, *cropToP1, *cropToP2);
			if (inputP1Inside) {
				outputPolyVerts->push_back(*inputP1);
				outputPolyTexCoords->push_back((*inputPolyTexCoords)[inputIndex]);
			}

			//bool p2IsCropPoint = inputP2 == cropToP1 || inputP2 == cropToP2;
			inputP2Inside = isLeft(*inputP2, *cropToP1, *cropToP2);
			if (inputP1Inside != inputP2Inside /*&& !p2IsCropPoint*/) {
				glm::vec2 vecInter = intersection(*cropToP1, *cropToP2, *inputP1, *inputP2);
				outputPolyVerts->push_back(vecInter);

				float distPerc = glm::distance(vecInter, *inputP1) / glm::distance(*inputP2, *inputP1);
				glm::vec2 texCoordInter = (*inputPolyTexCoords)[inputIndex] + distPerc * ((*inputPolyTexCoords)[(inputIndex + 1) % inputPolyTexCoords->size()] - (*inputPolyTexCoords)[inputIndex]);
				outputPolyTexCoords->push_back(texCoordInter);
			}
		}
		std::swap(inputPolyVerts, outputPolyVerts);
		std::swap(inputPolyTexCoords, outputPolyTexCoords);
		outputPolyVerts->clear();
		outputPolyTexCoords->clear();
	}
	// Depending on the number of edges in cropToPoly, the inputPly pointer may be ending at
	// either subjectPoly or croppedPoly, so we need to make sure the right one is being output!
	if (inputPolyVerts != &subjectPolyVerts) {
		// if this is the case, we can assume subjectPoly has already been cleared.
		for (int i = 0; i < croppedPolyVerts.size(); i++) {
			subjectPolyVerts.push_back(croppedPolyVerts[i]);
			subjectPolyTexCoords.push_back(croppedPolyTexCoords[i]);
		}
	}
	return true;
}

inline vechelp::SideOfLine vechelp::isLeftSpecific(glm::vec2 point, glm::vec2 lineSegPoint1, glm::vec2 lineSegPoint2)
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

// Crops subjectPoly to cropToPoly.  Both need to be wound clockwise!
bool vechelp::sutherlandHodgemanPolyCrop(std::vector<glm::vec2>& subjectPoly, std::vector<glm::vec2> cropToPoly,
									   bool removeDuplicatePoints)
{
	if (cropToPoly.size() < 2) {
		subjectPoly.clear();
		return false;
	}

	glm::vec2 cropToP1, cropToP2, inputP1, inputP2;
	std::vector<glm::vec2> croppedPoly;
	std::vector<glm::vec2>* inputPoly, * outputPoly;
	inputPoly = &subjectPoly;
	outputPoly = &croppedPoly;
	SideOfLine inputP1Side, inputP2Side;

	for (int cropToIndex = 0; cropToIndex < cropToPoly.size(); cropToIndex++) {
		cropToP1 = cropToPoly[cropToIndex];
		cropToP2 = cropToPoly[(cropToIndex + 1) % cropToPoly.size()];

		for (int inputIndex = 0; inputIndex < inputPoly->size(); inputIndex++) {
			inputP1 = (*inputPoly)[inputIndex];
			inputP2 = (*inputPoly)[(inputIndex + 1) % inputPoly->size()];
			inputP1Side = isLeftSpecific(inputP1, cropToP1, cropToP2);
			inputP2Side = isLeftSpecific(inputP2, cropToP1, cropToP2);

			if (inputP1Side == INSIDE || inputP1Side == ON_LINE_SEG) {
				outputPoly->push_back(inputP1);
			}
			if (inputP1Side != inputP2Side && inputP2Side != ON_LINE_SEG) {
				outputPoly->push_back(intersection(cropToP1, cropToP2, inputP1, inputP2));
			}
		}
		std::swap(inputPoly, outputPoly);
		outputPoly->clear();
	}
	// Depending on the number of edges in cropToPoly, the inputPly pointer may be ending at
	// either subjectPoly or croppedPoly, so we need to make sure the right one is being output!
	if (inputPoly != &subjectPoly) {
		// if this is the case, we can assume subjectPoly has already been cleared.
		for (glm::vec2 v : croppedPoly) {
			subjectPoly.push_back(v);
		}
	}
	if (removeDuplicatePoints) {
		// for the life of me idk why we cant just see if i == j.  Distance check works tho.
		for (int i = 0; i < subjectPoly.size(); i++) {
			for (int j = i + 1; j < subjectPoly.size(); j++) {
				if (glm::distance(subjectPoly[i], subjectPoly[j]) < 0.001f) {
					subjectPoly.erase(subjectPoly.begin() + i);
					i--;
					break;
				}
			}
		}
	}
	return true;
}

std::vector<glm::vec2> vechelp::sutherlandHodgemanLineCrop(std::vector<glm::vec2> subjectPoly, std::vector<glm::vec2>* cropToLine,
														 bool removeDuplicatePoints)
{

	glm::vec2 cropToP1, cropToP2, inputP1, inputP2;
	std::vector<glm::vec2> croppedPoly;
	std::vector<glm::vec2>* inputPoly, * outputPoly;
	inputPoly = &subjectPoly;
	outputPoly = &croppedPoly;
	bool inputP1Inside, inputP2Inside;

	for (int cropToIndex = 0; cropToIndex < cropToLine->size() - 1; cropToIndex++) {
		cropToP1 = (*cropToLine)[cropToIndex];
		cropToP2 = (*cropToLine)[(cropToIndex + 1)];

		for (int inputIndex = 0; inputIndex < inputPoly->size(); inputIndex++) {
			inputP1 = (*inputPoly)[inputIndex];
			inputP2 = (*inputPoly)[(inputIndex + 1) % inputPoly->size()];
			inputP1Inside = isLeft(inputP1, cropToP1, cropToP2);
			inputP2Inside = isLeft(inputP2, cropToP1, cropToP2);
			bool p1IsCropPoint = inputP1 == cropToP1 || inputP1 == cropToP2;

			if (inputP1Inside || p1IsCropPoint) {
				outputPoly->push_back(inputP1);
			}
			if (inputP1Inside != inputP2Inside && !p1IsCropPoint) {
				glm::vec2 inter = intersection(cropToP1, cropToP2, inputP1, inputP2);
				if (inter != glm::vec2(FLT_MAX, FLT_MAX)) {
					outputPoly->push_back(inter);
				}
			}
		}
		std::swap(inputPoly, outputPoly);
		outputPoly->clear();
	}
	// Depending on the number of edges in cropToPoly, the inputPly pointer may be ending at
	// either subjectPoly or croppedPoly, so we need to make sure the right one is being output!
	if (inputPoly != &subjectPoly) {
		// if this is the case, we can assume subjectPoly has already been cleared.
		for (glm::vec2 v : croppedPoly) {
			subjectPoly.push_back(v);
		}
	}
	if (removeDuplicatePoints) {
		// for the life of me idk why we cant just see if i == j.  Distance check works tho.
		for (int i = 0; i < subjectPoly.size(); i++) {
			for (int j = i + 1; j < subjectPoly.size(); j++) {
				if (glm::distance(subjectPoly[i], subjectPoly[j]) < 0.001f) {
					subjectPoly.erase(subjectPoly.begin() + i);
					i--;
					break;
				}
			}
		}
	}
	return subjectPoly;
}

float vechelp::angleBetween(glm::vec2 A, glm::vec2 B)
{
	return acos(glm::dot(A, B)
				/ (glm::distance(glm::vec2(0, 0), A) * glm::distance(glm::vec2(0, 0), B)));
}

// Returns the maximum of all four vertex's coordinates.  
// Meant for use finding the maxVert of four vertices that will make up a tile.
glm::ivec3 vechelp::getMaxVert(glm::ivec3 A, glm::ivec3 B, glm::ivec3 C, glm::ivec3 D)
{
	return glm::ivec3(std::max(std::max(std::max(A.x, B.x), C.x), D.x),
					  std::max(std::max(std::max(A.y, B.y), C.y), D.y),
					  std::max(std::max(std::max(A.z, B.z), C.z), D.z));
}
