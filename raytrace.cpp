#include "base.h"
#include "framebuf.h"
#include "bench.h"
#include "scene.h"

#include <cmath>
#include <chrono>

namespace {

}

int main() {
    const bool print_verts = false;
    constexpr int WIDTH = 1280, HEIGHT = 800;

    const auto scene = generate_scene(WIDTH, HEIGHT);
    const auto& camera = scene.camera;

    framebuf<RGB24> buf(WIDTH, HEIGHT);

    const uint32_t master_seed = 0xdeadbeef;
    const int max_rays = 50;

    double t = bench([&]() {
        for (int y = 0; y < HEIGHT; y++) {
            const float v = (HEIGHT - 1 - y) * (1.0f / (HEIGHT - 1));
            // Seed each line to prepare for parallelism
            Random rng(master_seed ^ y);
            for (int x = 0; x < WIDTH; x++) {
                const float u = x * (1.0f / (WIDTH - 1));

                auto ray = scene.camera.shoot_ray(u, v);
//                std::cout << "Tracing " << ray << std::endl;
                buf.at(x, y) = scene.trace(ray, rng, max_rays);
                // scene tracer will consume random numbers. Consider re-
                // seeding for every x to make it more reproducible.
                // (This random number generator seems to give very noticable
                // patterns when using x to seed though.)
            }
        }
    });
    std::cout << "Render speed: " << (t * 1e-9) << " s/frame\n";

    buf.save_ppm("frame.ppm");
}
