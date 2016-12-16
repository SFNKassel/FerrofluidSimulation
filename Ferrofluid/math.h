#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#define sq(x) (x*x)

typedef double math;

struct Vec3 {
	Vec3(math X, math Y, math Z) {
		x = X;
		y = Y;
		z = Z;
	}
	math x, y, z;
};

struct Magnet {

};