#include <iostream>
#include <vector>

#if defined(WIN32)
#include <DirectXMath.h>

using namespace DirectX;
#endif

#include <entt/entt.hpp>

#include "Ray.h"
#include "Sphere.h"
#include "hittest.h"

using Color = hvk::Vector;

const hvk::Vector kSkyColor1 = XMVectorSet(1.f, 1.f, 1.f, 0.f);
const hvk::Vector kSkyColor2 = XMVectorSet(0.5f, 0.7f, 1.f, 0.f);

const hvk::Sphere kTestSphere(hvk::Vector(0.f, 0.f, -1.f), 0.5f);


Color rayColor(const hvk::Ray& r)
{
    auto intersection = hvk::hit::SphereRayIntersect(kTestSphere, r);
    if (intersection.has_value() && intersection.value() > 0.f)
    {
        // we hit the sphere, so calculate the surface normal of the hit-point
        const auto hitPoint = r.PointAt(intersection.value());
        const auto normal = (hitPoint - kTestSphere.getCenter()).Normalized();
        return 0.5f * Color(normal.X()+1, normal.Y()+1, normal.Z()+1);
    }
    hvk::Vector unitDirection = r.getDirection();
    auto t = 0.5f * (unitDirection.Y() + 1.f);
    return (1.0 - t) * kSkyColor1 + t * kSkyColor2;
}

void writeColor(const Color& c)
{
    auto ir = static_cast<int>(255.999 * c.X());
    auto ig = static_cast<int>(255.999 * c.Y());
    auto ib = static_cast<int>(255.999 * c.Z());

    std::cout << ir << ' ' << ig << ' ' << ib << std::endl;
}

int main() {

    // Image setup
    const auto aspectRatio = 16.f / 9.f;
    const uint16_t imageWidth = 600;
    const uint16_t imageHeight = static_cast<uint16_t>(imageWidth / aspectRatio);

    // Camera setup
    const auto viewportHeight = 2.f;
    const auto viewportWidth = aspectRatio * viewportHeight;
    const auto focalLength = 1.f;

    const hvk::Vector origin(0.f, 0.f, 0.f);
    const hvk::Vector horizontal(viewportWidth, 0.f, 0.f);
    const hvk::Vector vertical(0.f, viewportHeight, 0.f);
    const auto lowerLeftCorner = origin - (horizontal / 2) - (vertical / 2) - hvk::Vector(0.f, 0.f, focalLength);

    std::vector<Color> writeOutBuffer;
    writeOutBuffer.resize(imageHeight * imageWidth);

    std::cout << "P3\n" << imageWidth << ' ' << imageHeight << "\n255\n";

    // Render
    for (int i = imageHeight-1; i >= 0; --i)
    {
        for (int j = 0; j < imageWidth; ++j)
        {
            auto u = static_cast<double>(j) / (imageWidth-1);
            auto v = static_cast<double>(i) / (imageHeight-1);

            hvk::Ray skyRay(origin, lowerLeftCorner + (u * horizontal) + (v * vertical) - origin);
            Color pixelColor = rayColor(skyRay);
            writeOutBuffer[((imageHeight-1) - i) * imageWidth + j] = pixelColor;
        }
    }

    for (const auto& c : writeOutBuffer)
    {
        writeColor(c);
    }

    return 0;
}
