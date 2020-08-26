#ifndef RTX_WEEKEND_VECTOR_H
#define RTX_WEEKEND_VECTOR_H

#if defined(WIN32)
#include <DirectXMath.h>

using namespace DirectX;

#endif

#include <utility>

#if defined(WIN32)
using SIMDVEC = XMVECTOR;
#endif

namespace hvk
{
    class Vector
    {
    public:
        Vector();
        Vector(float x, float y, float z);
        explicit Vector(const SIMDVEC& sv)
            : mNativeVec(sv)
        {}
        Vector(const Vector& v)
            : mNativeVec(v.mNativeVec)
        {}
        Vector(Vector&& v)
            : mNativeVec(std::move(v.mNativeVec))
        {}

        Vector Normalized() const;

        SIMDVEC getNativeVec() const;

        // component accessors
        float X() const;
        float Y() const;
        float Z() const;

        float Dot(const Vector& rhs) const;
        Vector Cross(const Vector& rhs) const;
        static float Dot(const Vector& lhs, const Vector& rhs);
        static Vector Cross(const Vector& lhs, const Vector& rhs);
        static Vector Reflect(const Vector& v, const Vector& normal);
        static Vector Refract(
                const Vector& incident,
                const Vector& normal,
                double iorLeave,
                double iorEnter);

        static Vector RandomUnit();

        Vector& operator= (const Vector& rhs);
        Vector& operator+= (const Vector& rhs);
        Vector operator+ (const Vector& rhs) const;
        Vector operator- (const Vector& rhs) const;
        Vector operator* (const Vector& rhs) const;
        Vector operator* (float rhs) const;
        Vector operator/ (float rhs) const;

    public:
        XMVECTOR mNativeVec;
    };
}

hvk::Vector operator* (float lhs, const hvk::Vector& rhs);

#endif //RTX_WEEKEND_VECTOR_H
