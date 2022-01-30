#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>
#include <memory>

#include <cmath>

#include "vec.h"

struct RGB24 {
    uint8_t r, g, b;

    RGB24() = default;
    RGB24(const Vector3 &v):
        r(std::lrintf(v.x * 255)),
        g(std::lrintf(v.y * 255)),
        b(std::lrintf(v.z * 255))
    {}
    RGB24(uint8_t r, uint8_t g, uint8_t b): r(r), g(g), b(b) {}
};
struct GREY8 {
    uint8_t g;
};
struct Z32 {
    float z;

    Z32() = default;
    Z32(float z): z(z) {}

    operator float() const { return z; }
};

template <typename T>
struct px_traits {
    static constexpr auto PPM_FORMAT = nullptr;
    static constexpr bool raw_writable = false;
};
template <>
struct px_traits<RGB24> {
    static constexpr char PPM_FORMAT[] = "P6";
    static constexpr bool raw_writable = true;
};
template <>
struct px_traits<Z32> {
    static constexpr char PPM_FORMAT[] = "P5";
    static constexpr bool raw_writable = false;
    static void write_ppm(std::ostream& os, const Z32 &px) {
        char grey = std::clamp(px.z, 0.0f, 1.0f) * 255;
        os.write(&grey, 1);
    }
};

template<typename Px>
struct framebuf {
    const size_t width;
    const size_t height;
    const size_t stride;
    const std::unique_ptr<Px[]> buffer;
    using PxTraits = px_traits<Px>;

    static constexpr size_t ALIGN = 32;

    framebuf(int w, int h):
        width(w), height(h), stride(get_stride(w)),
        buffer(std::make_unique<Px[]>(stride * height)) {}

    static size_t get_stride(size_t w) {
        const size_t bpp = sizeof(Px);
        const size_t bw = bpp * w;
        return ((bw + ALIGN - 1) & -ALIGN) / bpp;
    }

    Px& at(int x, int y) {
        return buffer[stride * y + x];
    }
    const Px& at(int x, int y) const {
        return buffer[stride * y + x];
    }
    Px* line(int y) {
        return &buffer[stride * y];
    }
    const Px* line(int y) const {
        return &buffer[stride * y];
    }

    void fill(Px px) {
        std::fill_n(buffer.get(), height * stride, px);
    }

    void save_ppm(const char *path) const {
        std::ofstream os(path);
        os << PxTraits::PPM_FORMAT << "\n"
            << width << " " << height << "\n"
            << "255\n";
        for (int y = 0; y < height; y++) {
            if constexpr (PxTraits::raw_writable) {
                os.write((char*)line(y), sizeof(Px) * width);
            }
            else {
                for (int x = 0; x < width; x++) {
                    PxTraits::write_ppm(os, at(x, y));
                }
            }
        }
    }
};

