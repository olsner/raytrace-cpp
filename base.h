#pragma once

#include <array>
#include <iostream>
#include <vector>
#include <cfloat>

#include "vec.h"

using Vertex = Vector3;

struct BoundingBox {
    Vector3 min { FLT_MAX, FLT_MAX, FLT_MAX };
    Vector3 max { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    void update(Vector3 p) {
        min = ::min(min, p);
        max = ::max(max, p);
    }
};

inline std::ostream& operator<<(std::ostream& os, const BoundingBox& bbox)
{
    return os << bbox.min << " .. " << bbox.max << std::endl;
}

template <typename T, typename S>
T lerp(const T& x, const T& y, const S& s)
{
    return (1 - s) * x + s * y;
}

template <typename T>
T radians(const T& deg)
{
    return deg * (M_PI / 180);
}

#define NOINLINE __attribute__((noinline))
