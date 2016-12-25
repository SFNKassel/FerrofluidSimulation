#pragma once

#define _USE_MATH_DEFINES
#include <cinttypes>
#include <cmath>
#include <ostream>

typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

template <typename valueType>
valueType sq(valueType a) {
    return a * a;
}

template <typename valueType>
struct Vector3D {
    valueType x;
    valueType y;
    valueType z;

    constexpr Vector3D() : x(), y(), z() {}
    constexpr Vector3D(valueType a) : x(a), y(a), z(a) {}
    constexpr Vector3D(valueType a, valueType b, valueType c) : x(a), y(b), z(c) {}
    Vector3D(const Vector3D & other) {
        this->x = other.x;
        this->y = other.y;
        this->z = other.z;
    }

    template <typename VT>
    friend std::ostream & operator<<(std::ostream &       output,
                                     const Vector3D<VT> & vec) {
        return output << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]";
    }

    Vector3D<valueType> operator+(const Vector3D<valueType> & rhs) const {
        Vector3D<valueType> vec(*this);
        vec += rhs;
        return vec;
    }

    Vector3D<valueType> operator-(const Vector3D<valueType> & rhs) const {
        Vector3D<valueType> vec(*this);
        vec -= rhs;
        return vec;
    }

    Vector3D<valueType> operator-() const {
        Vector3D<valueType> vec(*this);
        vec -= Vector3D<valueType>();
        return vec;
    }

    template <typename scalaType>
    Vector3D<valueType> operator*(const scalaType & rhs) const {
        Vector3D<valueType> vec(*this);
        vec *= rhs;
        return vec;
    }

    valueType operator*(const Vector3D<valueType> & rhs) const {
        return this->x * rhs.x + this->y * rhs.y + this->z * rhs.z;
    }

    template <typename scalaType>
    Vector3D<valueType> operator/(const scalaType & rhs) const {
        Vector3D<valueType> vec(*this);
        vec /= rhs;
        return vec;
    }

    Vector3D<valueType> & operator+=(const Vector3D<valueType> & rhs) {
        this->x += rhs.x;
        this->y += rhs.y;
        this->z += rhs.z;

        return *this;
    }

    Vector3D<valueType> & operator-=(const Vector3D<valueType> & rhs) {
        this->x -= rhs.x;
        this->y -= rhs.y;
        this->z -= rhs.z;

        return *this;
    }

    template <typename scalaType>
    Vector3D<valueType> & operator*=(const scalaType & rhs) {
        this->x *= rhs;
        this->y *= rhs;
        this->z *= rhs;

        return *this;
    }

    template <typename scalaType>
    Vector3D<valueType> & operator/=(const scalaType & rhs) {
        this->x /= rhs;
        this->y /= rhs;
        this->z /= rhs;

        return *this;
    }

    bool operator==(const Vector3D<valueType> & rhs) const {
        return (this->x == rhs.x) && (this->y == rhs.y) && (this->z == rhs.z);
    }

    auto norm() const -> decltype(x * x + y * y + z * z) {
        return std::sqrt(x * x + y * y + z * z);
    }
};

typedef Vector3D<double> Vec3;

namespace std {
    template <typename valueType>
    struct hash<Vector3D<valueType>>
        : public std::unary_function<const Vector3D<valueType> &, std::size_t> {
        inline std::size_t operator()(const Vector3D<valueType> & vec) const {
            std::size_t seed = 0;
            hash_combine(seed, vec.x);
            hash_combine(seed, vec.y);
            hash_combine(seed, vec.z);

            return seed;
        }
    };
}
