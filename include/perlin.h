#pragma once
#include <cmath>

namespace MinecraftAlter
{
    // modified based on https://en.wikipedia.org/wiki/Perlin_noise
    // https://rtouti.github.io/graphics/perlin-noise-algorithm
    float interpolate(float a, float b, float w) {
        // linear
        return (b - a) * w + a;
    }

    typedef struct {
        float x, y;
    } vec2;
    
    // vec2 randomGradient(int ix, int iy) {
    //     int angle = rand() % 360;
    //     float random = (float)angle / 180.0 * 3.14159265;
    //     vec2 v;
    //     v.x = cos(random);
    //     v.y = sin(random);
    //     return v;
    // }
    vec2 randomGradient(int ix, int iy) {
        // No precomputed gradients mean this works for any number of grid coordinates
        const unsigned w = 8 * sizeof(unsigned);
        const unsigned s = w / 2; // rotation width
        unsigned a = ix, b = iy;
        a *= 3284157443; b ^= a << s | a >> w-s;
        b *= 1911520717; a ^= b << s | b >> w-s;
        a *= 2048419325;
        float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
        vec2 v;
        v.x = cos(random); v.y = sin(random);
        return v;
    }

    // Computes the dot product of the distance and gradient vectors.
    float dotGridGradient(int ix, int iy, float x, float y) {
        // Get gradient from integer coordinates
        vec2 gradient = randomGradient(ix, iy);

        // Compute the distance vector
        float dx = x - (float)ix;
        float dy = y - (float)iy;

        // Compute the dot-product
        return (dx * gradient.x + dy * gradient.y);
    }

    float perlin(float x, float y) {
        // grid cell coords
        int x0 = (int)floor(x);
        int x1 = x0 + 1;
        int y0 = (int)floor(y);
        int y1 = y0 + 1;

        // Determine interpolation weights
        float sx = x - (float)x0;
        float sy = y - (float)y0;

        // Interpolate between grid point gradients
        float n0, n1, ix0, ix1;

        n0 = dotGridGradient(x0, y0, x, y);
        n1 = dotGridGradient(x1, y0, x, y);
        ix0 = interpolate(n0, n1, sx);

        n0 = dotGridGradient(x0, y1, x, y);
        n1 = dotGridGradient(x1, y1, x, y);
        ix1 = interpolate(n0, n1, sx);

        return interpolate(ix0, ix1, sy); // range [-1, 1]
    }
}
