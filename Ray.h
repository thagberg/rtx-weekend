#ifndef RTX_WEEKEND_RAY_H
#define RTX_WEEKEND_RAY_H

#include <utility>

#include "Vector.h"

using namespace DirectX;

namespace hvk
{
    class Ray
    {
    public:
        Ray(Vector origin, Vector direction)
            : mOrigin(origin)
            , mDirection(direction)
        {

        }

        Ray(const Ray& r)
            : mOrigin(r.mOrigin)
            , mDirection(r.mDirection)
        {

        }

        Vector PointAt(float t) const
        {
            return mOrigin + (mDirection.Normalized() * t);
        }

        Vector getDirection() const { return mDirection.Normalized(); }
        Vector getOrigin() const { return mOrigin; }

    private:
        Vector mOrigin;
        Vector mDirection;
    };
}


#endif //RTX_WEEKEND_RAY_H
