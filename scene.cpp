#include "scene.h"

#include <random>

Scene<Shape> generate_scene(float width, float height) {
    Scene<Shape> scene;

    std::mt19937 rng;
    std::uniform_real_distribution<float> unif{0.0f, 1.0f};

    const CameraOrientation orientation
        { { 13, 2, 3 }, { 0, 0, 0 }, { 0, 1, 0 } };

    scene.camera = Camera(orientation, 20.0f, width / height, 0.1f, 10.0f);

    //scene.camera = { { 0, 0, 0 }, { 2.0f, 2.0f } };

    const Lambertian grayLambertian{ { 0.5f, 0.5f, 0.5f } };
    scene.Add(Sphere{ { 0, -1000, 0}, 1000.0f }, grayLambertian);

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            const auto choose_mat = unif(rng);
            const Vec3 center(a + 0.9 * unif(rng), 0.2, b + 0.9 * unif(rng));
            const Sphere sphere{ center, 0.2f };

            if ((center - Vec3(4, 0.2, 0)).len() > 0.9) {
                if (choose_mat < 0.8f) {
                    // diffuse
                    const Vec3 randColor { unif(rng), unif(rng), unif(rng)};
                    auto albedo = randColor * randColor;
                    scene.Add(sphere, Lambertian{ albedo });
                } else if (choose_mat < 0.95f) {
                    // metal
                    const Vec3 randColor{ unif(rng), unif(rng), unif(rng)};
                    const auto albedo = 0.5f * (randColor + 1.0f);
                    auto fuzz = 0.5f * unif(rng);
                    scene.Add(sphere, Metal{ albedo, fuzz });
                } else {
                    // glass
                    scene.Add(sphere, Dielectric{ 1.5f });
                }
            }
        }
    }
    scene.Add(Sphere{ { 0, 1, 0  }, 1.0f }, Dielectric{ 1.5f });
    scene.Add(Sphere{ { -4, 1, 0 }, 1.0f }, Lambertian{ { 0.4f, 0.2f, 0.1f } });
    scene.Add(Sphere{ { 4, 1, 0  }, 1.0f }, Metal{ { 0.7f, 0.6f, 0.5f }, 0.0f });
    scene.Finish();
    return scene;
}
