#ifndef RTX_WEEKEND_HITRECORD_H
#define RTX_WEEKEND_HITRECORD_H

#include "Vector.h"

namespace  hvk
{
    struct HitRecord
    {
        Vector point;
        Vector normal;
        double t;
        bool frontFace;
    };
}


#endif //RTX_WEEKEND_HITRECORD_H
