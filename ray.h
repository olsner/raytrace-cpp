#pragma once

struct Ray {
    Vec3 origin;
    Vec3 direction;

    Vec3 at(float t) const {
        return origin + t * direction;
    }
};

inline std::ostream& operator<<(std::ostream& os, const Ray& ray)
{
    return os << "Ray{ " << ray.origin << " -> " << ray.direction << " }";
}

struct HitRecord {
    float distance = -1;
    int id;
    Vec3 p;
    Vec3 normal;
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

