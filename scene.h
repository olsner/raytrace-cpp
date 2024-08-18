#pragma once

#include "base.h"
#include "framebuf.h"
#include "ray.h"
#include "camera.h"
#include "material.h"
#include "sphere.h"
#include "bvh.h"

#include <optional>
#include <variant>

using Shape = std::variant<Sphere>;

using Material = std::variant<Metal, Dielectric, Lambertian>;

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

template<typename T>
struct Scene {
    struct Object {
        T shape;
        // TODO Consider an index instead of the data?
        Material material;
        int id;

        Object(int id, const T& shape, const Material& material): shape(shape), material(material), id(id) {}

        AABB get_bounds() const {
            return std::visit([&](const auto &shape) {
                return shape.get_bounds();
            }, shape);
        }

        Point3 get_center() const {
            return std::visit([&](const auto &shape) {
                return shape.get_center();
            }, shape);
        }

        void intersect(const Ray& ray, HitRecord& out) const {
            std::visit([&](const auto &shape) {
                shape.intersect(ray, out, id);
            }, shape);
        }
    };

    std::vector<Object> objects;
    // TODO BVH doesn't need to copy around materials and stuff...
    std::optional<BVH<Object>> bvh;

    Camera camera;

    void Add(T shape, const Material& material)
    {
        objects.emplace_back(objects.size(), shape, material);
    }

    void Finish()
    {
        bvh = BVH(objects);
    }

    const Material& GetMaterialOfObject(size_t id) const {
        return objects[id].material;
    }

    template <typename S>
    NOINLINE void IntersectShape(HitRecord& out, const Ray& ray) const {
        if (bvh.has_value()) {
            bvh->intersect(ray, out);
        } else {
            for (const auto& object : objects) {
                object.intersect(ray, out);
            }
        }
        if(out.is_hit()){
            std::visit([&](const auto &shape) {
                shape.set_normal(out, ray);
            }, objects[out.id].shape);
        }
    }

    void Intersect(HitRecord& out, const Ray& ray) const {
        IntersectShape<Sphere>(out, ray);
        // Plus for any other shapes we implement
    }

    NOINLINE Vec3 mtl_color(const HitRecord& hit, const Ray& ray, Random& rng, int ttl) const
    {
        if (ttl > 0) {
            const auto& mat = GetMaterialOfObject(hit.id);
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

    void Dump(std::ostream& os = std::cout) const {
        if (bvh.has_value()) {
            os << *bvh << "\n";
        } else {
            os << "No BVH present\n";
        }
    }
};

Scene<Shape> generate_scene(float width, float height);
