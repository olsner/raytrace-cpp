#pragma once
#include <chrono>

static double ns()
{
    return std::chrono::nanoseconds(std::chrono::steady_clock::now().time_since_epoch()).count();
}

template <typename F>
double bench(F&& func)
{
    size_t iters = 1;
    size_t next_iters = 1;
    double elapsed = 0;
    const double min_elapsed = 1e9;
    while (elapsed < min_elapsed)
    {
        double start_time = ns();
        iters = next_iters;
        for (size_t i = 0; i < iters; i++) {
            func();
        }

        double end_time = ns();
        elapsed = end_time - start_time;
        next_iters = iters * 2;
    }

    std::cout << "Repeated benchmark " << iters << " times\n";
    return elapsed / iters;
}

