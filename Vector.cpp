#include "Vector.h"

#include "math.h"

hvk::Vector operator* (float lhs, const hvk::Vector& rhs)
{
    return rhs * lhs;
}

namespace hvk
{
#if defined(WIN32)
    Vector::Vector(float x, float y, float z)
            : mNativeVec(XMVectorSet(x, y, z, 0.f))
    {
    }

    SIMDVEC Vector::getNativeVec() const
    {
        return mNativeVec;
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

    Vector& Vector::operator+= (const Vector& rhs)
    {
        mNativeVec += rhs.mNativeVec;
        return *this;
    }

    float Vector::Dot(const Vector& lhs, const Vector& rhs)
    {
        return lhs.Dot(rhs);
    }

    Vector Vector::RandomUnit()
    {
        auto azimuth = math::getRandom<double, 0.f, 2 * XM_PI>();
        auto polar = math::getRandom<double, 0.f, 2 * XM_PI>();
        auto radial = 1.f;
        return Vector(sin(polar) * cos(azimuth), sin(polar) * sin(azimuth), cos(polar));
    }

    Vector Vector::Reflect(const Vector &v, const Vector &normal)
    {
        return v - 2 * Vector::Dot(v, normal) * normal;
    }

    double _SchlickFresnelApproximation(double cosineTheta, double refraction)
    {
        // Schlick Approximation formula:
        //  F = F0 + (1 - F0) * (1 - cos(theta))^5
        //  F0 = (n - 1)^2 / (n` + 1)^2
        // cos(theta) = I . N

        auto f0 = (1 - refraction) / (1 + refraction);
        f0 = f0 * f0;
        return f0 + (1 - f0) * pow((1 - cosineTheta), 5);
    }

    Vector Vector::Refract(const Vector &incident, const Vector &normal, double iorLeave, double iorEnter)
    {
        auto IN = Vector::Dot(incident.Normalized(), normal.Normalized());
        if (IN > 0.f)
        {
            double temp = iorLeave;
            iorLeave = iorEnter;
            iorEnter = temp;
        }
        // assert(IN >= 0.f);
        double eta = iorLeave / iorEnter;
        double k = 1 - (eta * eta) * (1 - (IN * IN));
        if (k < 0)
        {
            // total internal reflection
            return Reflect(incident, normal);
        }
        else
        {
            double reflectProbability = _SchlickFresnelApproximation(IN, eta);
            if (math::getRandom<double, 0.f, 1.f>() < reflectProbability)
            {
                return Reflect(incident, normal);
            }
            return eta * incident + (eta * IN - sqrt(k)) * normal;
        }
    }
}
