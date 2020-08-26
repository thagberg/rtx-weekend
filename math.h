#define _USE_MATH_DEFINES

#include <random>
#include <cmath>

#ifndef RTX_WEEKEND_MATH_H
#define RTX_WEEKEND_MATH_H

namespace hvk
{
    namespace math
    {
        template<typename T, T lower, T upper>
        T getRandom()
        {
            static std::mt19937 generator;
            static std::uniform_real_distribution<T> distribution(lower, upper);
            return distribution(generator);
        }

        double degreesToRadians(double degrees);
    }
}

#endif //RTX_WEEKEND_MATH_H
