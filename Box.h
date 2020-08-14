#ifndef RTX_WEEKEND_BOX_H
#define RTX_WEEKEND_BOX_H

#include "Plane.h"
#include <utility>
#include <vector>
#include <array>

namespace hvk
{
    template <typename T>
    constexpr auto to_underlying(T t)
    {
        return static_cast<std::underlying_type_t<T>>(t);
    }

    enum class Side
    {
        Top = 0,
        Bottom,
        Front,
        Back,
        Left,
        Right,
        NumSides
    };

    using BoxSides = std::array<Plane, to_underlying(Side::NumSides)>;

    class Box
    {
    public:
        Box(const Plane& t, const Plane& b, const Plane& f, const Plane& ba, const Plane& l, const Plane& r);

        Plane getSide(Side s) const;
        const BoxSides& getSides() const;

    private:
        BoxSides mSides;
    };
}

#endif //RTX_WEEKEND_BOX_H
