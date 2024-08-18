#pragma once

struct Ray {
    Point3 origin;
    Vec3 direction;
    Vec3 inverted_direction;
    Vec3 color;

    Ray(const Point3& origin, const Vec3& direction, const Vec3& color):
        origin(origin), direction(direction.norm()),
        inverted_direction(1 / direction.norm()),
        color(color) {}

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

    void set_normal(const Ray& ray, const Vec3& outwards) {
        front_face = dot(ray.direction, outwards) < 0;
        normal = front_face ? outwards : -outwards;
    }
};

