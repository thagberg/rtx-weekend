#include "Camera.h"
#include "math.h"

namespace hvk
{
    Camera::Camera(
        Vector lookFrom,
        Vector lookAt,
        Vector up,
        double verticalFov,
        double aspectRatio,
        double near,
        double far)
        : mOrigin(0.f, 0.f, 0.0f)
        , mHorizontal()
        , mVertical()
        , mLowerLeft()
    {
        auto theta = math::degreesToRadians(verticalFov);
        auto h = tan(theta / far);
        auto viewportHeight = far * h;
        auto viewportWidth = aspectRatio * viewportHeight;

        auto w = (lookFrom - lookAt).Normalized();
        auto u = Vector::Cross(up, w).Normalized();
        auto v = Vector::Cross(w, u);

        mOrigin = lookFrom;
        mHorizontal = viewportWidth * u;
        mVertical = viewportHeight * v;
        mLowerLeft = mOrigin - (mHorizontal / 2) - (mVertical / 2) - w;
    }

    Ray Camera::GetRay(double u, double v) const
    {
        return Ray(mOrigin, mLowerLeft + (u * mHorizontal) + (v * mVertical) - mOrigin);
    }
}
