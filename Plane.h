#ifndef RTX_WEEKEND_PLANE_H
#define RTX_WEEKEND_PLANE_H

#include "Vector.h"

namespace hvk
{
    class Plane
    {
    public:
        Plane(Vector origin, Vector direction);
        Plane();

        Vector getDirection() const;
        Vector getOrigin() const;
    private:
        Vector mOrigin;
        Vector mDirection;
    };
}

#endif //RTX_WEEKEND_PLANE_H
