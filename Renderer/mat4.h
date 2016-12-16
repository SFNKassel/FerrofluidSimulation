#pragma once

#include "maths.h"

namespace engine {
namespace maths {

	struct Mat4 {

		union {
			float elements[4 * 4];
			Vec4 columns[4];
		};

		Mat4();
		Mat4(float diagonal);

		static inline Mat4 identity();

		static Mat4 orthographic(float left, float right, float bottom, float top, float near, float far);
		static Mat4 perspective(float fov, float aspectRatio, float near, float far);

		static Mat4 translation(const Vec3& translation);
		static Mat4 rotation(float angle, const Vec3& axis);
		static Mat4 scale(const Vec3& scale);

		Mat4& invert();

		Mat4& operator*=(const Mat4& other);

		friend Mat4 operator~(Mat4 right);
		friend Mat4 operator*(Mat4 left, const Mat4& right);
		friend Vec3 operator*(const Mat4& left, const Vec3& right);
		friend Vec4 operator*(const Mat4& left, const Vec4& right);
	private:
		Mat4& multiply(const Mat4 &other);
		Vec3 multiply(const Vec3 &other) const;
		Vec4 multiply(const Vec4 &other) const;
		static constexpr inline int getPosition(int x, int y) { return y + x * 4; }
	};
}}