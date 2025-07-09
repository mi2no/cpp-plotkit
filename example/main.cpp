#define _USE_MATH_DEFINES
#include <math.h>

#include "../path.hpp"
#define __DISABLE_CANVAS // No canvas support in this build
#include "../plot.hpp"

int main() {

    constexpr double f = .5; // sine freq
    constexpr double f_s = 30, T = 10; // sampling freq & total time
    constexpr size_t size = T * f_s; // sample count

    double * const x = (double*)malloc(sizeof(double) * size << 1), * const y = x + size;

    { // signal generation
        double t = 0;
        const double t_inc = 1 / f_s;
        for (size_t i = 0; i < size; ++i, t += t_inc) {
            x[i] = t;
            y[i] = sin(2 * M_PI * f * t);
        }
    }

    _plot p;
    p.add({x, size, _plot::REF}, {y, size, _plot::REF}, size, 0xFF0000FF);
    p.set_title("Sine wave example");

    path_f* d = p.to_paths(960, 960); // convert plot to path array
    svg::save(d, p.layer_count(), "plot.svg");
    delete[] d;

    free(x);

}