#ifndef RTX_WEEKEND_CAMERA_H
#define RTX_WEEKEND_CAMERA_H

#include "Vector.h"
#include "Ray.h"

namespace hvk
{
    class Camera
    {
    public:
        Camera(double verticalFov, double aspectRatio, double near, double far);
        Ray GetRay(double u, double v) const;

    private:
        Vector mOrigin;
        Vector mHorizontal;
        Vector mVertical;
        Vector mLowerLeft;
    };

}

#endif //RTX_WEEKEND_CAMERA_H
