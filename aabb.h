#pragma once

#include "base.h"
#include "ray.h"
#include "vec.h"

struct InvertedRay {
    Vec3 origin;
    Vec3 direction;
    Vec3 inv_direction;

    template <typename Ray>
    InvertedRay(const Ray& ray): origin(ray.origin), direction(ray.direction), inv_direction(1 / ray.direction.norm()) {
    }
};

class AABB {
    Vec3 min = Vec3::inf();
    Vec3 max = -Vec3::inf();

    AABB(Vec3 min, Vec3 max): min(min), max(max) {}

public:
    AABB() = default;

    static AABB centered(Point3 mid, float radius) {
        return { mid - radius, mid + radius };
    }

    bool empty() const {
        return min.x >= max.x || min.y >= max.y || min.z >= max.z;
    }

    Vec3 get_size() const {
        return max - min;
    }

    bool contains(const Point3& point) const {
        return min.x <= point.x && min.y <= point.y && min.z <= point.z
            && point.x <= max.x && point.y <= max.y && point.z <= max.z;
    }

    void expand(const Vec3& margin) {
        min -= 0.5f * margin;
        max += 0.5f * margin;
    }

    void expand(float margin) {
        min = min - 0.5f * margin;
        max = max + 0.5f * margin;
    }

    void merge(const AABB& other) {
        if (!other.empty()) {
            min = ::min(min, other.min);
            max = ::max(max, other.max);
        }
    }

    bool intersects(const NormalizedRay& ray) const {
        return intersects(InvertedRay(ray));
    }

    NOINLINE bool intersects(const InvertedRay& ray) const {
        const Vec3 v1 = min - ray.origin;
        const Vec3 v2 = max - ray.origin;

        const Vec3 t1 = v1 * ray.inv_direction;
        const Vec3 t2 = v2 * ray.inv_direction;

        const Vec3 tmin = ::min(t1, t2), tmax = ::max(t1,t2);

        return std::min(tmax.x, std::min(tmax.y, tmax.z)) >=
            std::max(tmin.x, std::max(tmin.y, tmin.z));
    }

    friend std::ostream& operator<<(std::ostream& os, const AABB& aabb) {
        return os << "AABB{ min=" << aabb.min << ", max=" << aabb.max << "}";
    }
};
