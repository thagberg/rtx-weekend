#include "Camera.h"
#include "math.h"

namespace hvk
{
    Camera::Camera(double verticalFov, double aspectRatio, double near, double far)
        : mOrigin(0.f, 0.f, 0.5f)
        , mHorizontal()
        , mVertical()
        , mLowerLeft()
    {
        auto theta = math::degreesToRadians(verticalFov);
        auto h = tan(theta / far);
        auto viewportHeight = far * h;
        auto viewportWidth = aspectRatio * viewportHeight;

        auto focalLength = 1.f;

        mHorizontal = Vector(viewportWidth, 0.f, 0.f);
        mVertical = Vector(0.f, viewportHeight, 0.f);
        mLowerLeft = mOrigin - (mHorizontal / 2) - (mVertical / 2) - Vector(0.f, 0.f, focalLength);
    }

    Ray Camera::GetRay(double u, double v) const
    {
        return Ray(mOrigin, mLowerLeft + (u * mHorizontal) + (v * mVertical) - mOrigin);
    }
}
