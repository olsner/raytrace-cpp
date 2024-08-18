#pragma once

#include <cmath>
#include <iostream>
#include <random>

#include "base.h"

// Structure: Vector2
//
// Description: A 2D Vector that Holds Positional Data
struct Vector2
{
    Vector2() = default;
    Vector2(float x_, float y_)
    {
        x = x_;
        y = y_;
    }
    // Bool Equals Operator Overload
    bool operator==(const Vector2& other) const
    {
        return (this->x == other.x && this->y == other.y);
    }
    // Bool Not Equals Operator Overload
    bool operator!=(const Vector2& other) const
    {
        return !(this->x == other.x && this->y == other.y);
    }
    // Addition Operator Overload
    Vector2 operator+(const Vector2& right) const
    {
        return Vector2(this->x + right.x, this->y + right.y);
    }
    // Subtraction Operator Overload
    Vector2 operator-(const Vector2& right) const
    {
        return Vector2(this->x - right.x, this->y - right.y);
    }
    // Float Multiplication Operator Overload
    Vector2 operator*(const float& other) const
    {
        return Vector2(this->x *other, this->y * other);
    }

    float x = 0.0f;
    float y = 0.0f;
};
inline Vector2 operator*(float left, const Vector2 &right)
{
    return { left * right.x, left * right.y };
}

// Structure: Vector3
//
// Description: A 3D Vector that Holds Positional Data
struct Vector3
{
    // Default Constructor
    Vector3() = default;
    // Variable Set Constructor
    Vector3(float x_, float y_, float z_)
    {
        x = x_;
        y = y_;
        z = z_;
    }
    explicit Vector3(__m128 vec_) {
        vec = vec_;
    }

    // Bool Equals Operator Overload
    bool operator==(const Vector3& other) const
    {
        return (x == other.x && y == other.y && z == other.z);
    }
    // Bool Not Equals Operator Overload
    bool operator!=(const Vector3& other) const
    {
        return !(*this == other);
    }
    Vector3 operator*(const float& other) const
    {
        return Vector3(vec * other);
    }
    Vector3 operator/(const float& other) const
    {
        return Vector3(vec / other);
    }
    Vector3 operator+(const float& other) const
    {
        return Vector3(vec + other);
    }
    Vector3 operator-(const float& other) const
    {
        return Vector3(vec - other);
    }
    Vector3 operator-() const
    {
        return Vector3(0 - vec);
    }
    Vector2 xy() const
    {
        return { x, y };
    }
    inline float sqlen() const;
    float len() const
    {
        return std::sqrt(sqlen());
    }
    [[nodiscard]] Vector3 norm() const
    {
        return *this / len();
    }
    float horizontal_sum() const
    {
        return x + y + z;
    }
    template <typename RNG>
    static Vector3 random(RNG& rng, float min, float max)
    {
        std::uniform_real_distribution<float> dist{ min, max };
        return { dist(rng), dist(rng), dist(rng) };
    }
    bool near_zero(float eps = 1e-8) const
    {
        return std::abs(x) < eps && std::abs(y) < eps && std::abs(z) < eps;
    }

    static Vector3 inf()
    {
        return { INFINITY, INFINITY, INFINITY };
    }

    union {
        struct {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
        };
        __m128 vec;
    };
};
inline Vector3& operator+=(Vector3& left, const Vector3& right)
{
    left.vec += right.vec;
    return left;
}
inline Vector3& operator-=(Vector3& left, const Vector3& right)
{
    left.vec += right.vec;
    return left;
}
inline Vector3 operator+(const Vector3& left, const Vector3& right)
{
    return Vector3(left.vec + right.vec);
}
inline Vector3 operator-(const Vector3& left, const Vector3& right)
{
    return Vector3(left.vec - right.vec);
}
inline Vector3 operator*(const Vector3& left, const Vector3& right)
{
    return Vector3(left.vec * right.vec);
}
inline Vector3 operator*(const float& scale, const Vector3& vec)
{
    return Vector3(vec.vec * scale);
}
inline Vector3 operator/(const float& scale, const Vector3& vec)
{
    return Vector3(scale / vec.vec);
}
inline Vector3 min(const Vector3& left, const Vector3& right)
{
    return Vector3(_mm_min_ps(left.vec, right.vec));
}
inline Vector3 max(const Vector3& left, const Vector3& right)
{
    return Vector3(_mm_max_ps(left.vec, right.vec));
}
inline float dot(const Vector3& a, const Vector3& b)
{
    return (a * b).horizontal_sum();
}
inline Vector3 cross(const Vector3& a, const Vector3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

float Vector3::sqlen() const
{
    return dot(*this, *this);
}

using Vec2 = Vector2;
using Vec3 = Vector3;
using Point3 = Vector3;

template <typename RNG>
inline Vec3 random_in_unit_sphere(RNG& rng)
{
    // Eww
    while (true) {
        auto p = Vec3::random(rng, -1, 1);
        if (p.sqlen() <= 1) return p;
    }
}

template <typename RNG>
inline Vec3 random_unit_vector(RNG& rng)
{
    return random_in_unit_sphere(rng).norm();
}

inline Vec3 reflect(const Vec3& v, const Vec3& n)
{
    return v - 2 * dot(v, n) * n;
}

inline std::ostream& operator<<(std::ostream& os, Vector3 vec)
{
    return os << "[" << vec.x << "," << vec.y << "," << vec.z << "]";
}

inline std::ostream& operator<<(std::ostream& os, Vector2 vec)
{
    return os << "[" << vec.x << "," << vec.y << "]";
}

