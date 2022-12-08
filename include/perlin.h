#pragma once
#include <cmath>

namespace MinecraftAlter
{
    // Modify based on: https://en.wikipedia.org/wiki/Perlin_noise
    // Fade function from: https://rtouti.github.io/graphics/perlin-noise-algorithm

    // Fade function: smooth the interpolation curve
    float fade(float w) { return ((6 * w - 15) * w + 10) * w * w * w; }

    float interpolate(float a, float b, float w) { return (b - a) * fade(w) + a; }

    typedef struct {
        float x, y;
    } vec2;

    vec2 randomGradient(int ix, int iy) {
        // No precomputed gradients mean this works for any number of grid coordinates
        const unsigned w = 8 * sizeof(unsigned);
        const unsigned s = w / 2; // rotation width
        unsigned a = ix, b = iy;
        a *= 3284157443;
        b ^= a << s | a >> w - s;
        b *= 1911520717;
        a ^= b << s | b >> w - s;
        a *= 2048419325;
        float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
        vec2 v;
        v.x = cos(random);
        v.y = sin(random);
        return v;
    }

    // Computes dot(random gradient vector, offset vector to [x, y]) @ [ix, iy]
    float dotGridGradient(int seed, int ix, int iy, float x, float y) {
        // Get gradient from integer coordinates
        vec2 gradient = randomGradient(ix + seed, iy + seed);

        // Compute the distance vector
        float dx = x - (float)ix;
        float dy = y - (float)iy;

        // Compute the dot-product
        return (dx * gradient.x + dy * gradient.y);
    }

    float perlin(int seed, float x, float y) {
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

        n0 = dotGridGradient(seed, x0, y0, x, y); // dot(random vector, offset) @ [0, 0]
        n1 = dotGridGradient(seed, x1, y0, x, y); // dot(random vector, offset) @ [1, 0]
        ix0 = interpolate(n0, n1, sx); // interpolate along edge y = 0

        n0 = dotGridGradient(seed, x0, y1, x, y); // dot(random vector, offset) @ [0, 1]
        n1 = dotGridGradient(seed, x1, y1, x, y); // dot(random vector, offset) @ [1, 1]
        ix1 = interpolate(n0, n1, sx); // interpolate along edge y = 1

        return interpolate(ix0, ix1, sy); // range [-1, 1]; interpolate along y-direction
    }
}
