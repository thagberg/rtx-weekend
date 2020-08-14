#include "Box.h"

namespace hvk
{
    Box::Box(const Plane &t, const Plane &b, const Plane &f, const Plane &ba, const Plane &l, const Plane &r)
        : mSides()
    {
        mSides[to_underlying(Side::Top)] = t;
        mSides[to_underlying(Side::Bottom)] = b;
        mSides[to_underlying(Side::Front)] = f;
        mSides[to_underlying(Side::Back)] = ba;
        mSides[to_underlying(Side::Left)] = l;
        mSides[to_underlying(Side::Right)] = r;
    }

    Plane Box::getSide(Side s) const
    {
        auto sideIndex = to_underlying(s);
        return mSides[sideIndex];
    }

    const BoxSides& Box::getSides() const
    {
        return mSides;
    }
}