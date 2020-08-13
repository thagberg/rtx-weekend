#ifndef RTX_WEEKEND_SPHERE_H
#define RTX_WEEKEND_SPHERE_H

#include "Vector.h"

namespace hvk
{
    class Sphere
    {
    public:
        Sphere(const Vector& origin, float radius)
            : mCenter(origin)
            , mRadius(radius)
        {}
        Sphere()
            : Sphere({}, 0.f)
        {}

        Vector getCenter() const { return mCenter; }
        float getRadius() const { return mRadius; }

        Sphere& operator= (const Sphere& rhs)
        {
            if (&rhs == this)
            {
                return *this;
            }
            mCenter = rhs.mCenter;
            mRadius = rhs.mRadius;
            return *this;
        }

    private:
        Vector mCenter;
        float mRadius;
    };
}


#endif //RTX_WEEKEND_SPHERE_H
