#ifndef RTX_WEEKEND_HITTEST_H
#define RTX_WEEKEND_HITTEST_H

#include <optional>

#include "Vector.h"
#include "Ray.h"
#include "Sphere.h"

namespace hvk
{
    namespace hit
    {
        std::optional<float> SphereRayIntersect(const Sphere& sphere, const Ray& ray)
        {
            Vector rayToSphere = ray.getOrigin() - sphere.getCenter();

            // This is the quadratic equation:
            //  (V . V)t^2 + (S . V)t + (S . S) - r^2 = 0
            // Where:
            //  V = ray direction vector
            //  S = vector from the ray's origin to the sphere's center
            //  r = radius of the sphere
            // The quadratic comes from:
            //  (A + tV - C) . (A + tV - C) - r^2 = 0
            // Where:
            //  A = the ray's origin
            // From the equation for a sphere which is:
            //  x^2 + y^2 + z^2 = r^2

            const auto r = sphere.getRadius();
            const auto a = 1.f; // Ray direction is normalized, so dot(R, R) will always be 1
            const auto b = 2.0 * Vector::Dot(rayToSphere, ray.getDirection());
            const auto rtsDot = Vector::Dot(rayToSphere, rayToSphere);
            const auto c = rtsDot - r * r;
            const auto discriminant = b*b - 4*a*c;
            if (discriminant > 0)
            {
                float rootOne = (-b - sqrt(discriminant)) / (2.0 * a);
                float rootTwo = (-b + sqrt(discriminant)) / (2.0 * a);
                float closestRoot = std::min(rootOne, rootTwo);
                return std::optional{ closestRoot };
            }
            return std::nullopt;
        }
    }
}

#endif //RTX_WEEKEND_HITTEST_H
