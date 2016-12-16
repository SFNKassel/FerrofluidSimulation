#pragma once


#define _USE_MATH_DEFINES
#include <math.h>

#include "vec\vec2.h"
#include "vec\vec3.h"
#include "vec\vec4.h"
#include "mat4.h"

namespace engine {
namespace maths {
	
	inline constexpr float toRadians(float degrees) {
		return degrees * ((float)M_PI / 180.0f);
	}

}}