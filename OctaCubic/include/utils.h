#pragma once
#include <cmath>

inline float clamp(const float x, const float min, const float max) {
    return x > max ? max : x < min ? min : x;
}

inline float interpolate(const float f1, const float f2, const float p) {
    return f1 + p * (f2 - f1);
}

inline float remainder(float dividend, const int divisor) {
    if (dividend < 0) dividend += static_cast<float>(divisor);
    const int dividendInt = static_cast<int>(dividend);
    const float decimal = dividend - round(dividend);
    return static_cast<float>(dividendInt % divisor) + decimal;
}
