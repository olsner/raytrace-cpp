#pragma once

#include "base.h"
#include "framebuf.h"

#include <optional>
#include <variant>

using Random = std::minstd_rand;

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

// TODO camera.h
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
        return { origin, corner + u * horizontal + v * vertical - origin };
    }
};

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

class SquaredFloat {
    float value_;
    float sq_;

public:
    SquaredFloat(float value): value_(value), sq_(value*value) {}
    float squared() const { return sq_; }
    float value() const { return value_; }
};

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
        auto discriminant = half_b * half_b - c;
        if (discriminant < 0) {
            return;
        }

        auto distance = -half_b - std::sqrt(discriminant);
        // Many checks here... Can we rely on sqrt producing NaN for negative
        // values perhaps?
        if (distance >= 0 && (distance < out.distance || !out.is_hit())) {
            out.distance = distance;
            out.p = r.at(distance);
            out.set_normal(r, (out.p - center) / radius);
            out.id = id;
        }
    }
};

using Shape = std::variant<Sphere>;

struct ScatterResult {
    Vec3 direction;
    Vec3 attenuation;
};

inline double pow5(const double x)
{
    const auto x2 = x * x;
    const auto x4 = x2 * x2;
    return x4 * x;
}

struct Metal {
    Vec3 albedo;
    float fuzziness = 1.0f;

    ScatterResult scatter(const HitRecord& hit, const Ray& ray, Random& rng) const
    {
        Vec3 reflected = reflect(ray.direction.norm(), hit.normal);
        return { reflected + fuzziness * random_in_unit_sphere(rng), albedo };
    }
};
struct Dielectric {
    float refraction;

    ScatterResult scatter(const HitRecord& hit, const Ray& ray, Random& rng) const
    {
        const Vec3 albedo{ 1, 1, 1 };
        float ratio = hit.front_face ? 1 / refraction : refraction;

        const auto v = ray.direction.norm();
        const auto& n = hit.normal;

        const auto cos_theta = std::min(dot(-v, n), 1.0f);
        const auto sin_theta = std::sqrt(1 - cos_theta * cos_theta);

        const auto thresh = std::uniform_real_distribution<float>(0, 1)(rng);

        if (reflectance(cos_theta, ratio) > thresh || ratio * sin_theta > 1)
            return { reflect(v, n), albedo };

        Vec3 r_out_perp = ratio * (v + cos_theta * n);
        Vec3 r_out_para = -std::sqrt(std::abs(1.0f - r_out_perp.sqlen())) * n;
        auto refracted = r_out_perp + r_out_para;

        return { refracted, albedo };
    }

    // Schlick's approximation
    static float reflectance(float cos, double ratio)
    {
        auto r0 = (1 - ratio) / (1 + ratio);
        r0 *= r0;
        return r0 + (1 - r0) * pow5(1 - cos);
    }
};
struct Lambertian {
    Vec3 albedo;

    ScatterResult scatter(const HitRecord& hit, const Ray& ray, Random& rng) const
    {
        auto direction = hit.normal + random_unit_vector(rng);
        if (direction.near_zero()) {
            printf("Very unlucky - random vector antiparallel to normal\n");
            direction = hit.normal;
        }
        return { direction, albedo };
    }
};

using Material = std::variant<Metal, Dielectric, Lambertian>;

struct Object {
    Shape shape;
    Material material;
};

template <typename T, typename Fun, typename Var>
void visit_one_type(const std::vector<Var> &vec, Fun&& fun)
{
    for (size_t i = 0; i < vec.size(); i++) {
        const auto& var = vec[i];
        if (std::holds_alternative<T>(var)){
            fun(i, std::get<T>(var));
        }
    }
}

inline Vec3 sky_color(const Ray& ray) {
    auto dir = ray.direction.norm();
    // -1..1 -> 0..1
    auto t = 0.5f * dir.y + 0.5f;
    Vec3 blue { 0.5, 0.7, 1 }; // light blue
    Vec3 white { 1, 1, 1 };
    return lerp(blue, white, t);
}

inline Vec3 norm_color(const HitRecord& hit)
{
    Vec3 n = hit.normal;
    return 0.5f * (n + 1);
}

struct Scene {
    // For now: materials are simply duplicated with one array entry per shape.
    // Check if it would be useful to share materials and e.g. have an index
    // in the shape?
    std::vector<Material> materials;

    // 
    std::vector<Shape> shapes;

    Camera camera;

    void Add(const Shape& shape, const Material& material)
    {
        shapes.push_back(shape);
        materials.push_back(material);
        // On second thought, ray/sphere intersection is (I think) simpler than
        // ray/box intersection.
//        bounds.push_back(bounds(shape));
    }

    template <typename T>
    NOINLINE void IntersectShape(HitRecord& out, const Ray& generic_ray) const {
        auto ray = typename T::RayType{ generic_ray };
        visit_one_type<T>(shapes, [&out, ray](int id, const auto &shape) {
            shape.intersect(out, ray, id);
        });
    }

    void Intersect(HitRecord& out, const Ray& ray) const {
        IntersectShape<Sphere>(out, ray);
        // Plus for any other shapes we implement
    }

    NOINLINE Vec3 mtl_color(const HitRecord& hit, const Ray& ray, Random& rng, int ttl) const
    {
        if (ttl > 0) {
            const auto& mat = materials[hit.id];
            auto [direction, attenuation] =
                std::visit([&](const auto &material){
                    return material.scatter(hit, ray, rng);
                }, mat);
            return attenuation * trace(Ray{ hit.p, direction }, rng, ttl - 1);
        }
        return Vec3{};
    }

    // TODO Instead of recursing and ttl, have a way to output new rays with
    // a weight, and maybe we allow up to 4 new rays at some minimum weight.
    NOINLINE Vec3 trace(const Ray& ray, Random& rng, int ttl) const {
        HitRecord hit{};
        Intersect(hit, ray);
        if (hit.is_hit()) {
            return mtl_color(hit, ray, rng, ttl);
        }
        else {
            return sky_color(ray);
        }
    }
};

Scene generate_scene(float width, float height);
