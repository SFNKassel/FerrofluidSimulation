#include "mat4.h"

namespace engine {
namespace maths {

	Mat4::Mat4() {
		for (int i = 0; i < 4 * 4; i++)
			elements[i] = 0;
	}

	Mat4::Mat4(float diagonal) {
		for (int i = 0; i < 4 * 4; i++)
			elements[i] = 0;

		elements[getPosition(0, 0)] = diagonal;
		elements[getPosition(1, 1)] = diagonal;
		elements[getPosition(2, 2)] = diagonal;
		elements[getPosition(3, 3)] = diagonal;
	}

	Mat4 Mat4::identity() {
		return Mat4(1.0f);
	}

	Mat4 Mat4::orthographic(float left, float right, float bottom, float top, float near, float far) {
		Mat4 result = identity();

		result.elements[getPosition(0, 0)] = 2.0f / (right - left);
		result.elements[getPosition(1, 1)] = 2.0f / (top - bottom);
		result.elements[getPosition(2, 2)] = 2.0f / (near - far);

		result.elements[getPosition(3, 0)] = (left + right) / (left - right);
		result.elements[getPosition(3, 1)] = (bottom + top) / (bottom - top);
		result.elements[getPosition(3, 2)] = (far + near) / (far - near);

		return result;
	}

	Mat4 Mat4::perspective(float fov, float aspectRatio, float near, float far) {
		Mat4 result = identity();

		float q = 1.0f / tan(toRadians(0.5f * fov));

		result.elements[getPosition(0, 0)] = q / aspectRatio;
		result.elements[getPosition(1, 1)] = q;
		result.elements[getPosition(2, 2)] = (near + far) / (near - far);
		result.elements[getPosition(2, 3)] = -1.0f;
		result.elements[getPosition(3, 2)] = (2.0f * near * far) / (near - far);

		return result;
	}

	Mat4 Mat4::translation(const Vec3& translation) {
		Mat4 result = identity();

		result.elements[getPosition(3, 0)] = translation.x;
		result.elements[getPosition(3, 1)] = translation.y;
		result.elements[getPosition(3, 2)] = translation.z;

		return result;
	}

	Mat4 Mat4::rotation(float angle, const Vec3& axis) {
		Mat4 result = identity();

		float r = toRadians(angle);
		float c = cos(r);
		float s = sin(r);
		float omc = 1.0f - c;

		result.elements[getPosition(0, 0)] = axis.x * axis.x * omc + c;
		result.elements[getPosition(0, 1)] = axis.y * axis.x * omc + axis.z * s;
		result.elements[getPosition(0, 2)] = axis.x * axis.z * omc - axis.y * s;

		result.elements[getPosition(1, 0)] = axis.x * axis.y * omc - axis.z * s;
		result.elements[getPosition(1, 1)] = axis.y * axis.y * omc + c;
		result.elements[getPosition(1, 2)] = axis.y * axis.z * omc + axis.x * s;

		result.elements[getPosition(2, 0)] = axis.x * axis.z * omc + axis.y * s;
		result.elements[getPosition(2, 1)] = axis.y * axis.z * omc - axis.x * s;
		result.elements[getPosition(2, 2)] = axis.z * axis.z * omc + c;

		return result;
	}

	Mat4 Mat4::scale(const Vec3& scale) {
		Mat4 result = identity();

		result.elements[getPosition(0, 0)] = scale.x;
		result.elements[getPosition(1, 1)] = scale.y;
		result.elements[getPosition(2, 2)] = scale.z;

		return result;
	}

	Mat4& Mat4::invert() {
		double inv[16];

		inv[0] = elements[5] * elements[10] * elements[15] -
			elements[5] * elements[11] * elements[14] -
			elements[9] * elements[6] * elements[15] +
			elements[9] * elements[7] * elements[14] +
			elements[13] * elements[6] * elements[11] -
			elements[13] * elements[7] * elements[10];

		inv[4] = -elements[4] * elements[10] * elements[15] +
			elements[4] * elements[11] * elements[14] +
			elements[8] * elements[6] * elements[15] -
			elements[8] * elements[7] * elements[14] -
			elements[12] * elements[6] * elements[11] +
			elements[12] * elements[7] * elements[10];

		inv[8] = elements[4] * elements[9] * elements[15] -
			elements[4] * elements[11] * elements[13] -
			elements[8] * elements[5] * elements[15] +
			elements[8] * elements[7] * elements[13] +
			elements[12] * elements[5] * elements[11] -
			elements[12] * elements[7] * elements[9];

		inv[12] = -elements[4] * elements[9] * elements[14] +
			elements[4] * elements[10] * elements[13] +
			elements[8] * elements[5] * elements[14] -
			elements[8] * elements[6] * elements[13] -
			elements[12] * elements[5] * elements[10] +
			elements[12] * elements[6] * elements[9];

		inv[1] = -elements[1] * elements[10] * elements[15] +
			elements[1] * elements[11] * elements[14] +
			elements[9] * elements[2] * elements[15] -
			elements[9] * elements[3] * elements[14] -
			elements[13] * elements[2] * elements[11] +
			elements[13] * elements[3] * elements[10];

		inv[5] = elements[0] * elements[10] * elements[15] -
			elements[0] * elements[11] * elements[14] -
			elements[8] * elements[2] * elements[15] +
			elements[8] * elements[3] * elements[14] +
			elements[12] * elements[2] * elements[11] -
			elements[12] * elements[3] * elements[10];

		inv[9] = -elements[0] * elements[9] * elements[15] +
			elements[0] * elements[11] * elements[13] +
			elements[8] * elements[1] * elements[15] -
			elements[8] * elements[3] * elements[13] -
			elements[12] * elements[1] * elements[11] +
			elements[12] * elements[3] * elements[9];

		inv[13] = elements[0] * elements[9] * elements[14] -
			elements[0] * elements[10] * elements[13] -
			elements[8] * elements[1] * elements[14] +
			elements[8] * elements[2] * elements[13] +
			elements[12] * elements[1] * elements[10] -
			elements[12] * elements[2] * elements[9];

		inv[2] = elements[1] * elements[6] * elements[15] -
			elements[1] * elements[7] * elements[14] -
			elements[5] * elements[2] * elements[15] +
			elements[5] * elements[3] * elements[14] +
			elements[13] * elements[2] * elements[7] -
			elements[13] * elements[3] * elements[6];

		inv[6] = -elements[0] * elements[6] * elements[15] +
			elements[0] * elements[7] * elements[14] +
			elements[4] * elements[2] * elements[15] -
			elements[4] * elements[3] * elements[14] -
			elements[12] * elements[2] * elements[7] +
			elements[12] * elements[3] * elements[6];

		inv[10] = elements[0] * elements[5] * elements[15] -
			elements[0] * elements[7] * elements[13] -
			elements[4] * elements[1] * elements[15] +
			elements[4] * elements[3] * elements[13] +
			elements[12] * elements[1] * elements[7] -
			elements[12] * elements[3] * elements[5];

		inv[14] = -elements[0] * elements[5] * elements[14] +
			elements[0] * elements[6] * elements[13] +
			elements[4] * elements[1] * elements[14] -
			elements[4] * elements[2] * elements[13] -
			elements[12] * elements[1] * elements[6] +
			elements[12] * elements[2] * elements[5];

		inv[3] = -elements[1] * elements[6] * elements[11] +
			elements[1] * elements[7] * elements[10] +
			elements[5] * elements[2] * elements[11] -
			elements[5] * elements[3] * elements[10] -
			elements[9] * elements[2] * elements[7] +
			elements[9] * elements[3] * elements[6];

		inv[7] = elements[0] * elements[6] * elements[11] -
			elements[0] * elements[7] * elements[10] -
			elements[4] * elements[2] * elements[11] +
			elements[4] * elements[3] * elements[10] +
			elements[8] * elements[2] * elements[7] -
			elements[8] * elements[3] * elements[6];

		inv[11] = -elements[0] * elements[5] * elements[11] +
			elements[0] * elements[7] * elements[9] +
			elements[4] * elements[1] * elements[11] -
			elements[4] * elements[3] * elements[9] -
			elements[8] * elements[1] * elements[7] +
			elements[8] * elements[3] * elements[5];

		inv[15] = elements[0] * elements[5] * elements[10] -
			elements[0] * elements[6] * elements[9] -
			elements[4] * elements[1] * elements[10] +
			elements[4] * elements[2] * elements[9] +
			elements[8] * elements[1] * elements[6] -
			elements[8] * elements[2] * elements[5];

		double det = 1.0 / (elements[0] * inv[0] + elements[1] * inv[4] + elements[2] * inv[8] + elements[3] * inv[12]);

		for (int i = 0; i < 16; i++)
			elements[i] = inv[i] * det;

		return *this;
	}

	Mat4& Mat4::operator*=(const Mat4& other) {
		return this->multiply(other);
	}

	Mat4 operator~(Mat4 right) {
		return right.invert();
	}

	Mat4 operator*(Mat4 left, const Mat4& right) {
		return left.multiply(right);
	}

	Vec3 operator*(const Mat4& left, const Vec3& right) {
		return left.multiply(right);
	}

	Vec4 operator*(const Mat4& left, const Vec4& right) {
		return left.multiply(right);
	}

	Mat4& Mat4::multiply(const Mat4& other) {
		float data[16];
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				float sum = 0.0f;
				for (int e = 0; e < 4; e++)
				{
					sum += elements[x + e * 4] * other.elements[e + y * 4];
				}
				data[x + y * 4] = sum;
			}
		}

		memcpy(elements, data, 16 * 4);

		return *this;
	}

	Vec3 Mat4::multiply(const Vec3& other) const {
		return Vec3(
			elements[getPosition(0, 0)] * other.x + elements[getPosition(1, 0)] * other.y + elements[getPosition(2, 0)] * other.z + elements[getPosition(3, 0)],
			elements[getPosition(0, 1)] * other.x + elements[getPosition(1, 1)] * other.y + elements[getPosition(2, 1)] * other.z + elements[getPosition(3, 1)],
			elements[getPosition(0, 2)] * other.x + elements[getPosition(1, 2)] * other.y + elements[getPosition(2, 2)] * other.z + elements[getPosition(3, 2)]
			);
	}

	Vec4 Mat4::multiply(const Vec4& other) const {
		return Vec4(
			elements[getPosition(0, 0)] * other.x + elements[getPosition(1, 0)] * other.y + elements[getPosition(2, 0)] * other.z + elements[getPosition(3, 0)] * other.w,
			elements[getPosition(0, 1)] * other.x + elements[getPosition(1, 1)] * other.y + elements[getPosition(2, 1)] * other.z + elements[getPosition(3, 1)] * other.w,
			elements[getPosition(0, 2)] * other.x + elements[getPosition(1, 2)] * other.y + elements[getPosition(2, 2)] * other.z + elements[getPosition(3, 2)] * other.w,
			elements[getPosition(0, 3)] * other.x + elements[getPosition(1, 3)] * other.y + elements[getPosition(2, 3)] * other.z + elements[getPosition(3, 3)] * other.w
			);
	}

}}
