#pragma once

#include <optional>

#include "aabb.h"

enum class Axis { X, Y, Z };

static Axis next_axis(Axis axis) {
    if (axis == Axis::X) return Axis::Y;
    if (axis == Axis::Y) return Axis::Z;
    if (axis == Axis::Z) return Axis::X;
    abort(); // or std::unreachable
}

static Axis largest_axis(const AABB& bounds) {
    Vec3 size = bounds.get_size();
    if (size.x > size.z && size.x > size.y) {
        return Axis::X;
    } else if (size.y > size.z) {
        return Axis::Y;
    } else {
        return Axis::Z;
    }
}

template<typename T>
bool compare_centers(const T& a, const T& b, Axis axis) {
    Point3 p1 = a.get_center();
    Point3 p2 = b.get_center();
    switch (axis) {
    case Axis::X: return p1.x < p2.x;
    case Axis::Y: return p1.y < p2.y;
    case Axis::Z: return p1.z < p2.z;
    default: return false;
    }
}

inline std::ostream& operator<<(std::ostream& os, Axis axis) {
    switch (axis) {
    case Axis::X: return os << "X";
    case Axis::Y: return os << "Y";
    case Axis::Z: return os << "Z";
    default: return os << static_cast<int>(axis);
    }
}

template <typename T>
class BVH {
    Axis axis = Axis::X;
    std::vector<T> items;
    AABB bounds;

    std::vector<BVH> subvolumes;

public:
    BVH() {}
    BVH(std::vector<T> in_items):
        items(std::move(in_items))
    {
        for (const auto& item : items) {
            bounds.merge(item.get_bounds());
        }
        bounds.expand(0.001f);
        axis = largest_axis(bounds);
        if (items.size() > 3) {
            std::sort(items.begin(), items.end(), [=](const T& a, const T& b) {
                return compare_centers(a, b, axis);
            });
            auto it1 = items.begin();
            auto it2 = it1 + items.size() / 2;
            auto it3 = items.end();
            if (it1 != it2) {
                subvolumes.emplace_back(std::vector<T>(it1, it2));
            } else {
                printf("it1..it2 empty?\n");
            }
            if (it2 != it3) {
                subvolumes.emplace_back(std::vector<T>(it2, it3));
            } else {
                printf("it2..it3 empty?\n");
            }
        }
    }

    Axis get_axis() const {
        return axis;
    }

    const AABB& get_bounds() const {
        return bounds;
    }

    const std::vector<BVH>& get_subvolumes() const {
        return subvolumes;
    }

    const std::vector<T>& get_items() const {
        return items;
    }

    bool intersects(const InvertedRay& ray) const {
        return bounds.intersects(ray);
    }

    template <typename... Args>
    void intersect(const InvertedRay& ray, Args&&... args) const {
        if (!intersects(ray)) {
            //printf("Skipping %zu objects :)\n", items.size());
            return;
        }
        if (subvolumes.size()) {
            for (const auto& subvol : subvolumes) {
                subvol.intersect(ray, std::forward<Args>(args)...);
            }
        } else {
            //printf("Reached leaf with %zu objects.\n", items.size());
            for (const auto& item : items) {
                item.intersect(ray, std::forward<Args>(args)...);
            }
        }
    }

    template <typename Ray, typename... Args>
    void intersect(const Ray& ray, Args&&... args) const {
        intersect(InvertedRay(ray), std::forward<Args>(args)...);
    }

    void dump(std::ostream& os, std::string indent) const {
        os << indent << "{ " << items.size() << " items, axis=" << axis << ", bounds=" << bounds << "\n";
        std::string indent2 = indent + "  ";
        if (subvolumes.size()) {
            os << indent2 << subvolumes.size() << " subvolumes:\n";
            for (const auto& subvol : subvolumes) {
                subvol.dump(os, indent2);
            }
        } else {
            for (const auto& item : items) {
                os << indent2 << item.id << ": " << item.get_center() << "\n";
            }
        }
        os << indent << "}\n";
    }

    friend std::ostream& operator<<(std::ostream& os, const BVH& bvh) {
        bvh.dump(os << "BVH ", "");
        return os;
    }
};

