#pragma once

#include <array>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <vector>

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
