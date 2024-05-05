#pragma once
#include <iostream>

namespace calc3 {
	struct Point2D;
	struct Point3D;
	struct Line2D;
	struct Line3D;
	struct Plane;

	struct Point2D {
		float x, y;
		Point2D(float x, float y) : x(x), y(y) {}

		void print() {
			std::cout << "(" << x << ", " << y << ")";
		}
	};

	struct Point3D {
		float x, y, z;

		Point3D() : x(0), y(0), z(0) {}
		Point3D(float x, float y, float z) : x(x), y(y), z(z) {}

		float magnitude() {
			return sqrt(x * x + y * y + z * z);
		}

		void print() {
			std::cout << "(" << x << ", " << y << ", " << z << ")";
		}

		Point3D cylindricalCoord() {
			float r;
			if (x > 0 && y < 0) {
				r = 2 * float(M_PI) + atan(y / x);
			}
			else if (x < 0 && y > 0) {
				r = float(M_PI) - atan(y / x);
			}
			else if (x < 0 && y < 0) {
				r = float(M_PI) + atan(y / x);
			}
			else {
				r = atan(y / x);
			}
			return Point3D(sqrt(x*x+y*y), r, z);
		}

		Point3D sphericalCoord() {

		}
	};

	inline Point3D operator+(Point3D A, Point3D B) {
		return Point3D(A.x + B.x, A.y + B.y, A.z + B.z);
	}

	inline Point3D operator-(Point3D A, Point3D B) {
		return Point3D(A.x - B.x, A.y - B.y, A.z - B.z);
	}

	inline Point3D operator*(Point3D A, float scalar) {
		return Point3D(A.x * scalar, A.y * scalar, A.z * scalar);
	}

	inline float dot(Point3D A, Point3D B) {
		return A.x * B.x + A.y * B.y + A.z * B.z;
	}

	inline Point3D cross(Point3D A, Point3D B) {
		return Point3D(
			A.y * B.z - B.y * A.z,
			-(A.x * B.z - B.x * A.z),
			A.x * B.y - B.x * A.y
		);
	}

	struct Line2D {
		Point2D initial, vector;
	};

	struct Line3D {
		Point3D initial, vector;

		Line3D() : initial(Point3D(0,0,0)), vector(Point3D(0, 0, 0)) {}
		Line3D(Point3D initial, Point3D vector) : initial(initial), vector(vector) {}

		void print() {
			std::cout << "r = ";
			initial.print();
			std::cout << " + t";
			vector.print();
		}
		void printParametric() {
			std::cout << "x(t) = " << initial.x << " + " << vector.x << "t" <<std::endl;
			std::cout << "y(t) = " << initial.z << " + " << vector.y << "t" <<std::endl;
			std::cout << "z(t) = " << initial.z << " + " << vector.z << "t" <<std::endl;
		}
	};

	// assumes theh plane is in the form ax + by + cz = d.
	struct Plane {
		float a, b, c, d;

		// Creates an equation of the plane in the form of ax + by + cz = d.
		Plane(float a, float b, float c, float d) : a(a), b(b), c(c), d(d) {}

		// Creates an equation of the plane that passes through the points P, Q, and R.
		Plane(Point3D P, Point3D Q, Point3D R) {
			Point3D PQ = Q - P;
			Point3D PR = R - P;
			Point3D norm = cross(PQ, PR);
			a = norm.x;
			b = norm.y;
			c = norm.z;
			d = -(norm.x * -P.x + norm.y * -P.y + norm.z * -P.z);
		}

		Point3D normal() {
			return Point3D(a, b, c);
		}

		void print() {
			std::cout << a << "x + " << b << "y + " << c << "z = " << d;
		}
	};

	inline Line3D getLine(Plane P, Plane Q) {
		Line3D line;
		float scalar = Q.a / P.a;
		if ((scalar * P.b) == Q.b && (scalar * P.c) == Q.c) {
			std::cout << "The two planes are parallel!" << std::endl;
			line.initial = Point3D(0, 0, 0);
			line.vector = Point3D(0, 0, 0);
			return line;
		}
		float x = 0;
		float z = (Q.d * P.b - Q.b * P.d) / (-Q.b * P.c + P.b * Q.c);
		float y = (P.d - P.c * z) / P.b;
		line.initial = Point3D(x, y, z);

		line.vector = cross(Point3D(P.a, P.b, P.c), Point3D(Q.a, Q.b, Q.c));
		return line;
	}

	// Returns the radian angle between the two 3D vectors.
	inline float angleBetweenVectors(Point3D A, Point3D B) {
		return acos(dot(A, B) / (A.magnitude() * B.magnitude()));
	}

	inline float angleBetweenPlanes(Plane A, Plane B) {
		return angleBetweenVectors(A.normal(), B.normal());
	}

	inline Point3D intersection(Plane P, Line3D L) {
		float t = (P.d - P.a * L.initial.x - P.b * L.initial.y - P.c - L.initial.z)
			/ (P.a * L.vector.x + P.b * L.vector.y + P.c * L.vector.z);
		return Point3D(
			L.initial.x + t * L.vector.x,
			L.initial.y + t * L.vector.y,
			L.initial.z + t * L.vector.z);
	}

	inline float dist(Plane P, Point3D X) {
		return abs(P.a * X.x + P.b * X.y + P.c * X.z - P.d)
			/ sqrt(P.a * P.a + P.b * P.b + P.c * P.c);
	}

	inline Point3D closestPoint(Point3D P, Plane Q) {
		float t = (Q.d - Q.a * P.x - Q.b * P.y - Q.c * P.z)
			/ (Q.a * Q.a + Q.b * Q.b + Q.c * Q.c);
		return P + (Q.normal() * t);
	}

	inline void volumeOfParallelepiped(Point3D a, Point3D b, Point3D c) {
		std::cout << "The volume of a parallelepiped given three vectors is:\n\t|(a x b) * c|\n\n";

		std::cout << "So given:\n\ta = "; a.print();
		std::cout << "\n\tb = "; b.print();
		std::cout << "\n\tc = "; c.print();
		std::cout << "\n\n";

		std::cout << "We must find:\n\t|("; a.print();
		std::cout << " x "; b.print();
		std::cout << ") * "; c.print(); std::cout << "|\n\n";

		Point3D axb = cross(a, b);

		std::cout << "Which simplifies to:\n\t|"; axb.print(); std::cout << " * "; c.print();
		std::cout << "|\n\n";

		float axbdotc = dot(axb, c);

		std::cout << "And equals " << axbdotc << std::endl;
	}

	inline void areaOfTriangle(Point3D a, Point3D b, Point3D c) {
		std::cout << "The area of a triangle given 3 R3 points is:\n\t(1/2) * ||(b - a) x (c - a)||\n\n";

		std::cout << "So given:\n\ta = ";
		a.print();
		std::cout << "\n\tb = ";
		b.print();
		std::cout << "\n\tc = ";
		c.print();
		std::cout << "\n\n";

		std::cout << "We must find:\n\t(1/2) * ||(";
		b.print();
		std::cout << " - ";
		a.print();
		std::cout << ") x (";
		c.print();
		std::cout << " - ";
		a.print();
		std::cout << ")||\n\n";

		Point3D bmina = b - a;
		Point3D cmina = c - a;

		std::cout << "Which simplifies to:\n\t(1/2) * ||";
		bmina.print();
		std::cout << " x ";
		cmina.print();
		std::cout << "||\n\n";

		Point3D normal = cross(bmina, cmina);

		std::cout << "Which simplifies to:\n\t(1/2) * ||";
		normal.print();
		std::cout << "||\n\n";

		float result = normal.magnitude();

		std::cout << "Which simplifies to:\n\t(1/2) * " << result << "\n\n";

		std::cout << "And equals " << 0.5f * result << std::endl;
	}
}