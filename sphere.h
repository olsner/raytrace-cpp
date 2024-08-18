#pragma once

#include "aabb.h"
#include "ray.h"
#include "vec.h"

struct Sphere {
    Point3 center;
    float radius;

    void intersect(const InvertedRay &r, HitRecord &out, int id) const {
        NormalizedRay nr{r.origin, r.direction};
        intersect(nr, out, id);
    }

    void intersect(const NormalizedRay &r, HitRecord &out, int id) const {
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

    void set_normal(HitRecord &out, const NormalizedRay &r) const {
        out.p = r.at(out.distance);
        out.set_normal(r, (out.p - center) / radius);
    }

    const Point3& get_center() const {
        return center;
    }

    AABB get_bounds() const {
        return AABB::centered(center, radius);
    }
};
