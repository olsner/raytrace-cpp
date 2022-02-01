#pragma once

#include "ray.h"
#include "vec.h"

struct Sphere {
    Vec3 center;
    float radius;

    // Allow us to assume direction is normalized regardless of ray input.
    // (ray should probably require that the direction is normalized anyway?)
    struct RayType {
        RayType(const Ray& ray):
            origin(ray.origin), direction(ray.direction.norm()) {}

        Vec3 origin;
        Vec3 direction;

        Vec3 at(float t) const {
            return origin + t * direction;
        }
    };
    void intersect(HitRecord &out, const RayType &r, int id) const {
        const Vec3 oc = r.origin - center;
        auto half_b = dot(oc, r.direction);
        auto c = oc.sqlen() - radius * radius;
        if (half_b * half_b < c) {
            return;
        }

        auto discriminant = half_b * half_b - c;
        auto distance = -half_b - std::sqrt(discriminant);
        // Many checks here... Can we rely on sqrt producing NaN for negative
        // values perhaps?
        if (distance >= 0 && (distance < out.distance || !out.is_hit())) {
            out.distance = distance;
            out.id = id;
        }
    }

    void set_normal(HitRecord &out, const RayType &r) const {
        out.p = r.at(out.distance);
        out.set_normal(r, (out.p - center) / radius);
    }
};
