#ifndef RTX_WEEKEND_HITTEST_H
#define RTX_WEEKEND_HITTEST_H

#include <optional>
#include <utility>

#include "Vector.h"
#include "Ray.h"
#include "Sphere.h"
#include "Plane.h"
#include "Box.h"

namespace hvk
{
    namespace hit
    {
        struct Hittable
        {

        };

        std::optional<float> SphereRayIntersect(const Sphere& sphere, const Ray& ray)
        {
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

            const Vector rayToSphere = ray.getOrigin() - sphere.getCenter();
            const auto r = sphere.getRadius();
            const auto a = 1.f; // Ray direction is normalized, so dot(R, R) will always be 1
            const auto b = 2.0 * Vector::Dot(rayToSphere, ray.getDirection());
            const auto rtsDot = Vector::Dot(rayToSphere, rayToSphere);
            const auto c = rtsDot - r * r;
            const auto discriminant = b*b - 4*a*c;
            if (discriminant > 0)
            {
                float root = sqrt(discriminant);
                const auto epsilon = 5 * std::numeric_limits<decltype(root)>::epsilon();
                float rootOne = (-b - root) / (2.0 * a);
                float rootTwo = (-b + root) / (2.0 * a);
//                return std::optional{ std::min(rootOne, rootTwo) };
                if (rootOne > epsilon && rootTwo > epsilon)
                {
                    float closestRoot = std::min(rootOne, rootTwo);
                    return std::optional{ closestRoot };
                }
                else if (rootOne > epsilon)
                {
                    return std::optional{ rootOne };
                }
                else if (rootTwo > epsilon)
                {
                    return std::optional{ rootTwo };
                }
            }
            return std::nullopt;
        }

        std::optional<float> PlaneRayIntersect(const Plane& plane, const Ray& ray)
        {
            // The implicit form of a plane is:
            //  (P1 - P0) . N = 0
            // Where:
            //  P0 is the "origin" or known point on the plane
            //  P1 is another point on the plane
            //  N is the surface normal of the plane

            // Substituting the point on a ray equation:
            //  (R + tD - P0) . N = 0
            // Where:
            //  R is the origin of the ray
            //  D is the direction of the ray
            //  t is the the "time"
            // We get (after solving for t):
            //  t = ((P0 - R) . N) / (D . N)

            const auto denominator = Vector::Dot(ray.getDirection(), plane.getDirection());
            const auto epsilon = std::numeric_limits<decltype(denominator)>::epsilon();
            if (abs(denominator) > epsilon)
            {
                const auto numerator = Vector::Dot(plane.getOrigin() - ray.getOrigin(), plane.getDirection());
                return std::optional { numerator / denominator };
            }

            return std::nullopt;
        }

        std::optional<std::pair<Side, float>> BoxRayIntersect(const Box& box, const Ray& ray)
        {
            // Box intersection is done by first finding a plane which
            // the ray intersects with, and then checking if that point
            // is "inside" each of the box's other planes

            const auto& sides = box.getSides();

            for (size_t i = 0; i < sides.size(); ++i)
            {
                const auto& side = sides[i];
                // Can't intersect if this side is facing the same direction as the ray
                if (Vector::Dot(side.getDirection(), ray.getDirection()) > 0.f)
                {
                    continue;
                }

                auto intersection = PlaneRayIntersect(side, ray);
                // intersected this side, ensure it's inside all other sides
                if (intersection.has_value())
                {
                    bool pointInsideBox = true;
                    auto intersectionPoint = ray.PointAt(intersection.value());
                    Side s = static_cast<Side>(i);
                    for (size_t j = 0; j < sides.size(); ++j)
                    {
                        // don't test against the collision side
                        if (j == i)
                        {
                            continue;
                        }

                        const auto& otherSide = sides[j];
                        auto pointToPlane = otherSide.getOrigin() - intersectionPoint;
                        // if the dot product of the plane's normal and the vector
                        // formed between the intersection point and the plane origin
                        // is negative, then the intersection point is "inside" the box side
                        auto d = Vector::Dot(pointToPlane, otherSide.getDirection());
                        if (d < 0.f)
                        {
                            pointInsideBox = false;
                            break;
                        }
                    }

                    if (pointInsideBox)
                    {
                        return std::optional{ std::make_pair(s, intersection.value()) };
                    }
                }
            }

            return std::nullopt;
        }
    }
}

#endif //RTX_WEEKEND_HITTEST_H
