#include "base.h"
#include "framebuf.h"
#include "bench.h"
#include "scene.h"

#include <algorithm>
#include <cmath>
#include <chrono>
#include <execution>

int main() {
    const bool print_verts = false;
    constexpr int WIDTH = 1280, HEIGHT = 800;

    const auto scene = generate_scene(WIDTH, HEIGHT);
    const auto& camera = scene.camera;

#ifdef __SSE__
    // Sets denormals-are-zero and flush-to-zero, which appears to make no
    // difference whatsoever.
    _mm_setcsr(_mm_getcsr() | 0x8040);
#endif

    framebuf<RGB24> buf(WIDTH, HEIGHT);

    const uint32_t master_seed = 0xdeadbeef;
    const int max_rays = 100;
    const int samples_per_pixel = 32;
    const float sample_weight = 1.0f / samples_per_pixel;
    std::uniform_real_distribution<float> offset_dist_u(0, 1.0f / (WIDTH - 1));
    std::uniform_real_distribution<float> offset_dist_v(0, 1.0f / (HEIGHT - 1));

    // Dumb
    std::vector<int> rows(HEIGHT);
    std::iota(rows.begin(), rows.end(), 0);

    double t = bench([&]() {
        std::for_each(std::execution::par_unseq, rows.begin(), rows.end(),
        [&](int y){
            const float v = (HEIGHT - 1 - y) * (1.0f / (HEIGHT - 1));
            // Seed each line to prepare for parallelism
            Random rng(master_seed ^ y);
            for (int x = 0; x < WIDTH; x++) {
                Vec3 sum{};
                const float u = x * (1.0f / (WIDTH - 1));
                for (int i = 0; i < samples_per_pixel; i++) {
                    const float off_u = offset_dist_u(rng);
                    const float off_v = offset_dist_v(rng);
                    const auto ray = scene.camera.shoot_ray(u + off_u, v + off_v);
                    //std::cout << "Tracing " << ray << std::endl;
                    sum = sum + scene.trace(ray, rng, max_rays);
                }
                buf.at(x, y) = sum * sample_weight;
            }
        });
    });
    std::cout << "Render speed: " << (t * 1e-9) << " s/frame\n";

    buf.save_ppm("frame.ppm");
}
