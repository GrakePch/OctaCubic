#pragma once
#include <cmath>
#include <valarray>

namespace MinecraftAlter
{
    // Modify based on: https://en.wikipedia.org/wiki/Perlin_noise
    int permutation[] = {
        151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36,
        103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0,
        26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56,
        87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
        77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55,
        46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132,
        187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109,
        198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126,
        255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183,
        170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43,
        172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112,
        104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162,
        241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106,
        157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205,
        93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
    }; // 0 - 255

    // Fade function from: https://rtouti.github.io/graphics/perlin-noise-algorithm
    // Fade function: smooth the interpolation curve
    float fade(float w) { return ((6 * w - 15) * w + 10) * w * w * w; }

    float interpolate(float a, float b, float w) { return (b - a) * fade(w) + a; }

    typedef struct {
        float x, y;
    } vec2;

    // Random Gradient modified from: https://www.bilibili.com/video/BV11V4y1M7N6
    vec2 randomGradient(int ix, int iy) {
        // Mask x, y in the range [0, 127]
        int mx = ix & 127;
        int my = iy & 127;

        // Hash value @ (ix, iy)
        int hv = permutation[permutation[mx] + my];
        int mhv = hv & 3;
        float modeProj = sqrt(2) / 2;
        return {
            (mhv == 0 || mhv == 2? 1 : -1) * modeProj,
            (mhv == 1 || mhv == 2? 1 : -1) * modeProj
        };
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
