#pragma once

#include <cmath>
#include <random>

inline float fast_sqrt(float x)
{
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x)));
}

struct Vector2
{
    Vector2() = default;
    Vector2(float x_, float y_)
    {
        x = x_;
        y = y_;
    }
    bool operator==(const Vector2& other) const
    {
        return (this->x == other.x && this->y == other.y);
    }
    bool operator!=(const Vector2& other) const
    {
        return !(this->x == other.x && this->y == other.y);
    }
    Vector2 operator+(const Vector2& right) const
    {
        return Vector2(this->x + right.x, this->y + right.y);
    }
    Vector2 operator-(const Vector2& right) const
    {
        return Vector2(this->x - right.x, this->y - right.y);
    }
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

struct Vector3
{
    Vector3() = default;
    Vector3(float x_, float y_, float z_)
    {
        x = x_;
        y = y_;
        z = z_;
    }
    bool operator==(const Vector3& other) const
    {
        return (x == other.x && y == other.y && z == other.z);
    }
    bool operator!=(const Vector3& other) const
    {
        return !(*this == other);
    }
    Vector3 operator*(const float& other) const
    {
        return Vector3(simd_ * _mm_set1_ps(other));
    }
    Vector3 operator/(const float& other) const
    {
        return Vector3(simd_ / _mm_set1_ps(other));
    }
    Vector3 operator+(const float& other) const
    {
        return Vector3(simd_ + _mm_set1_ps(other));
    }
    Vector3& operator+=(const Vector3& right)
    {
        simd_ += right.simd_;
        return *this;
    }
    Vector3 operator+(const Vector3& right) const
    {
        return Vector3(simd_ + right.simd_);
    }
    Vector3 operator-(const Vector3& right) const
    {
        return Vector3(simd_ - right.simd_);
    }
    Vector3 operator-() const
    {
        return *this * -1.0f;
    }
    Vector3 operator*(const Vector3& right) const
    {
        return Vector3(simd_ * right.simd_);
    }
    Vector2 xy() const
    {
        return Vector2 { x, y };
    }
    friend float dot(const Vector3& a, const Vector3& b)
    {
        auto c = a * b;
        return c.x + c.y + c.z;
    }
    float sqlen() const
    {
        return dot(*this, *this);
    }
    float len() const
    {
        return fast_sqrt(sqlen());
    }
    [[nodiscard]] Vector3 norm() const
    {
        return *this / len();
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

    union {
        struct {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
            float pad__;
        };
        __m128 simd_;
    };

private:
    explicit Vector3(__m128 vec) { simd_ = vec; }
};

inline Vector3 operator*(const float& scale, const Vector3& vec)
{
    return vec * scale;
}
inline Vector3 min(const Vector3& left, const Vector3& right)
{
    return Vector3(
        std::min(left.x, right.x),
        std::min(left.y, right.y),
        std::min(left.z, right.z));
}
inline Vector3 max(const Vector3& left, const Vector3& right)
{
    return Vector3(
        std::max(left.x, right.x),
        std::max(left.y, right.y),
        std::max(left.z, right.z));
}
inline Vector3 cross(const Vector3& a, const Vector3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

using Vec2 = Vector2;
using Vec3 = Vector3;

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
