#pragma once

using Random = std::minstd_rand;

struct ScatterResult {
    Vec3 direction;
    Vec3 color;
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
        Vec3 reflected = reflect(ray.direction, hit.normal);
        return { reflected + fuzziness * random_in_unit_sphere(rng),
            albedo * ray.color };
    }
};
struct Dielectric {
    float refraction;

    ScatterResult scatter(const HitRecord& hit, const Ray& ray, Random& rng) const
    {
        float ratio = hit.front_face ? 1 / refraction : refraction;

        const auto v = ray.direction;
        const auto& n = hit.normal;

        const auto cos_theta = std::min(dot(-v, n), 1.0f);
        const auto sin_theta = std::sqrt(1 - cos_theta * cos_theta);

        const auto thresh = std::uniform_real_distribution<float>(0, 1)(rng);

        if (reflectance(cos_theta, ratio) > thresh || ratio * sin_theta > 1)
            return { reflect(v, n), ray.color };

        Vec3 r_out_perp = ratio * (v + cos_theta * n);
        Vec3 r_out_para = -std::sqrt(std::abs(1.0f - r_out_perp.sqlen())) * n;
        auto refracted = r_out_perp + r_out_para;

        return { refracted, ray.color };
    }

    // Schlick's approximation
    static float reflectance(double cos, double ratio)
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
        return { direction, albedo * ray.color };
    }
};

