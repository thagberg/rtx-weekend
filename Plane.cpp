#include "Plane.h"

namespace hvk
{
    Plane::Plane(Vector origin, Vector direction)
        : mOrigin(origin)
        , mDirection(direction)
    {
    }

    Plane::Plane()
        : mOrigin()
        , mDirection()
    {
    }

    Vector Plane::getOrigin() const
    {
        return mOrigin;
    }

    Vector Plane::getDirection() const
    {
        return mDirection;
    }
}
