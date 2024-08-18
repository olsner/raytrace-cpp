#pragma once

struct Ray {
    Point3 origin;
    Vec3 direction;

    Point3 at(float t) const {
        return origin + t * direction;
    }
};

// Ray that guarantees normalized direction vector. (TODO Why isn't that
// already required in Ray?)
struct NormalizedRay {
    Point3 origin;
    Vec3 direction;

    NormalizedRay() = default;
    NormalizedRay(const Point3& origin, const Vec3& direction):
        origin(origin), direction(direction) {}
    NormalizedRay(const Ray& ray):
        origin(ray.origin), direction(ray.direction.norm()) {}

    Point3 at(float t) const {
        return origin + t * direction;
    }
};

inline std::ostream& operator<<(std::ostream& os, const Ray& ray)
{
    return os << "Ray{ " << ray.origin << " -> " << ray.direction << " }";
}

struct HitRecord {
    float distance = -1;
    Point3 p;
    Vec3 normal;
    // TODO We are very far from needing a whole 32 bits for this
    uint32_t id;
    bool front_face;

    bool is_hit() const {
        return distance >= 0;
    }

    template <typename RayType>
    void set_normal(const RayType& ray, const Vec3& outwards) {
        front_face = dot(ray.direction, outwards) < 0;
        normal = front_face ? outwards : -outwards;
    }
};

