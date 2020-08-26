#include "math.h"
#include <cmath>
#include <DirectXMath.h>

namespace hvk
{
    namespace math
    {
        double degreesToRadians(double degrees)
        {
            return degrees * DirectX::XM_PI / 180.f;
        }
    }
}