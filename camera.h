#pragma once

#include "vec.h"
#include "ray.h"

struct CameraOrientation {
    Vec3 lookfrom;
    Vec3 lookat;
    Vec3 up;
};

struct Camera {
    Vec3 origin;
    Vec3 horizontal;
    Vec3 vertical;
    Vec3 corner;

    Camera() = default;

    Camera(Vec3 origin, Vec2 viewport, float focal_length = 1.0f):
        origin(origin),
        horizontal(viewport.x, 0, 0),
        vertical(0, viewport.y, 0),
        corner(origin - horizontal * 0.5f - vertical * 0.5f -
               Vec3(0, 0, focal_length))
    {}

    Camera(CameraOrientation orient, float vfov, float aspect_ratio, float aperture, float focal_length)
    {
        auto theta = radians(vfov);
        auto h = std::tan(theta / 2);
        auto viewport_height = 2 * h;
        auto viewport_width = aspect_ratio * viewport_height;

        auto w = (orient.lookfrom - orient.lookat).norm();
        auto u = cross(orient.up, w).norm();
        auto v = cross(w, u);

        origin = orient.lookfrom;
        horizontal = viewport_width * u;
        vertical = viewport_height * v;
        corner = origin - horizontal * 0.5f - vertical * 0.5f - w;
    }

    Ray shoot_ray(float u, float v) const {
        return Ray(origin, corner + u * horizontal + v * vertical - origin);
    }
};
