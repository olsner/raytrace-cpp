#include "bench.h"
#include "vec.h"
#include "sphere.h"

using Random = std::minstd_rand;

constexpr size_t N = 1048576;

struct Hits {
    float distance[N];
    int id[N];

    void reset() {
        std::fill_n(distance, N, INFINITY);
        std::fill_n(id, N, -1);
    }
};

struct Rays {
    float ox[N];
    float oy[N];
    float oz[N];

    float dx[N];
    float dy[N];
    float dz[N];

    Vec3 origin(int i) const {
        return { ox[i], oy[i], oz[i] };
    }
    Vec3 direction(int i) const {
        return { dx[i], dy[i], dz[i] };
    }

    void fill(Random& rng) {
        for (size_t i = 0; i < N; i++) {
            Vec3 o = random_in_unit_sphere(rng);
            Vec3 d = random_unit_vector(rng);
            ox[i] = o.x;
            oy[i] = o.y;
            oz[i] = o.z;
            dx[i] = d.x;
            dy[i] = d.y;
            dz[i] = d.z;
        }
    }
};

static inline float fast_sqrt(float x)
{
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x)));
}

void intersect(const Sphere &s, Hits& out, const Rays &r, int id) {
    for (size_t i = 0; i < N; i++) {
        const Vec3 oc = r.origin(i) - s.center;
        auto half_b = dot(oc, r.direction(i));
        auto c = oc.sqlen() - s.radius * s.radius;
//        if (half_b * half_b < c) {
//            continue;
//        }

        auto discriminant = half_b * half_b - c;
        auto distance = -half_b - fast_sqrt(discriminant);
        auto mask = -(distance >= 0 && distance < out.distance[i]);
        out.id[i] = (id & mask) | (out.id[i] & ~mask);
        out.distance[i] = std::min(distance, out.distance[i]);
    }
}

int main() {
    int i = 42;
    static Hits hits;
    static Rays rays;
    const Sphere sphere{ { -4, 1, 0 }, 1.0f };
    Random rng;
    rays.fill(rng);
    const double nano_t = bench([&]() {
        hits.reset();
        intersect(sphere, hits, rays, i++);
    });
    double t = nano_t * 1e-9;
    printf("%fs for %zu rays => %f rays/s\n", t, N, N / t);
    size_t nhits = 0;
    for (size_t i = 0; i < N; i++) {
        if (hits.id[i] >= 0) {
            nhits++;
        }
    }
    printf("%zu / %zu hits\n", nhits, N);
}
