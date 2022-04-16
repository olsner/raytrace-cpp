#pragma once

#include "base.h"
#include "framebuf.h"
#include "ray.h"
#include "camera.h"
#include "material.h"
#include "sphere.h"

#include <optional>
#include <variant>

using Shape = std::variant<Sphere>;

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
        if(out.is_hit()){
            std::visit([&out,ray](const auto &shape) {
                shape.set_normal(out, ray);
            }, shapes[out.id]);
        }
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
