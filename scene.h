#pragma once

#include <atomic>
#include <optional>
#include <variant>

#include "base.h"
#include "framebuf.h"
#include "ray.h"
#include "camera.h"
#include "material.h"
#include "sphere.h"
#include "bvh.h"

using Shape = std::variant<Sphere>;

using Material = std::variant<Metal, Dielectric, Lambertian>;

inline Vec3 sky_color(const Ray& ray) {
    auto dir = ray.direction.norm();
    // -1..1 -> 0..1
    auto t = 0.5f * dir.y + 0.5f;
    Vec3 blue { 0.5, 0.7, 1 }; // light blue
    Vec3 white { 1, 1, 1 };
    return lerp(blue, white, t) * ray.color;
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
            auto [direction, color] =
                std::visit([&](const auto &material){
                    return material.scatter(hit, ray, rng);
                }, mat);
            // Minimum value required to affect output pixel value (I think).
            constexpr float MIN_LIGHT = 1.0f / 255 / 100;
            if (std::max(color.x, color.y) > MIN_LIGHT || color.z > MIN_LIGHT) {
                return trace(Ray(hit.p, direction, color), rng, ttl - 1);
            } else {
                return color;
            }
        }
        return ray.color;
    }

    // TODO Instead of recursing, have a way to output new rays?
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
