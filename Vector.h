#ifndef RTX_WEEKEND_VECTOR_H
#define RTX_WEEKEND_VECTOR_H

#include <utility>

#if defined(WIN32)
#include <DirectXMath.h>

using namespace DirectX;

using SIMDVEC = XMVECTOR;
#endif

namespace hvk
{
    class Vector
    {
    public:
        Vector();
        Vector(float x, float y, float z);
        Vector(const SIMDVEC& sv)
            : mNativeVec(sv)
        {}
        Vector(const Vector& v)
            : mNativeVec(v.mNativeVec)
        {}
        Vector(Vector&& v)
            : mNativeVec(std::move(v.mNativeVec))
        {}

        Vector Normalized() const;

        // component accessors
        float X() const;
        float Y() const;
        float Z() const;

        float Dot(const Vector& rhs) const;
        static float Dot(const Vector& lhs, const Vector& rhs);

        Vector& operator= (const Vector& rhs);
        Vector operator+ (const Vector& rhs) const;
        Vector operator- (const Vector& rhs) const;
        Vector Vector::operator* (const Vector& rhs) const;
        Vector Vector::operator* (float rhs) const;
        Vector Vector::operator/ (float rhs) const;

    private:
        SIMDVEC mNativeVec;
    };

#if defined(WIN32)
    Vector::Vector(float x, float y, float z)
        : mNativeVec(XMVectorSet(x, y, z, 0.f))
    {
    }

    float Vector::X() const
    {
        return XMVectorGetX(mNativeVec);
    }

    float Vector::Y() const
    {
        return XMVectorGetY(mNativeVec);
    }

    float Vector::Z() const
    {
        return XMVectorGetZ(mNativeVec);
    }

    float Vector::Dot(const Vector& rhs) const
    {
        return XMVectorGetX(XMVector3Dot(mNativeVec, rhs.mNativeVec));
    }

    Vector Vector::Normalized() const
    {
        return Vector(XMVector3Normalize(mNativeVec));
    }

    Vector Vector::operator+ (const Vector& rhs) const
    {
        return Vector(mNativeVec + rhs.mNativeVec);
    }

    Vector Vector::operator- (const Vector& rhs) const
    {
        return Vector(mNativeVec - rhs.mNativeVec);
    }

    Vector Vector::operator* (const Vector& rhs) const
    {
        return Vector(mNativeVec * rhs.mNativeVec);
    }

    Vector Vector::operator* (float rhs) const
    {
        return Vector(mNativeVec * rhs);
    }

    Vector Vector::operator/ (float rhs) const
    {
        return Vector(mNativeVec / rhs);
    }
#endif

    Vector::Vector()
        : Vector(0.f, 0.f, 0.f)
    {

    }

    Vector& Vector::operator= (const Vector& rhs)
    {
        if (&rhs == this)
        {
            return *this;
        }
        mNativeVec = rhs.mNativeVec;
        return *this;
    }

    Vector operator* (float lhs, const Vector& rhs)
    {
        return rhs * lhs;
    }

    float Vector::Dot(const Vector& lhs, const Vector& rhs)
    {
        return lhs.Dot(rhs);
    }
}

#endif //RTX_WEEKEND_VECTOR_H
