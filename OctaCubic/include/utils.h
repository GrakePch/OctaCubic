#pragma once
#include <cmath>

inline float interpolate(const float f1, const float f2, const float p) {
    return f1 + p * (f2 - f1);
}

inline float remainder(float dividend, const int divisor) {
    if (dividend < 0) dividend += static_cast<float>(divisor);
    const int dividendInt = static_cast<int>(dividend);
    const float decimal = dividend - round(dividend);
    return static_cast<float>(dividendInt % divisor) + decimal;
}

inline std::string dToDecimalStr(const double d) {
    const std::string tempStr = std::to_string(d);
    return tempStr.substr(0, tempStr.find('.') + 2);
}