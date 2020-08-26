#include <iostream>
#include <vector>
#include <optional>

#if defined(WIN32)
#include <DirectXMath.h>

using namespace DirectX;
#endif

#include <entt/entt.hpp>

#include "Ray.h"
#include "Sphere.h"
#include "Material.h"
#include "hittest.h"
#include "math.h"
#include "HitRecord.h"
#include "Box.h"
#include "ThreadPool.h"
#include "Camera.h"

using Color = hvk::Vector;

const hvk::Vector kSkyColor1 = hvk::Vector(1.f, 1.f, 1.f);
const hvk::Vector kSkyColor2 = hvk::Vector(0.5f, 0.7f, 1.f);

const uint16_t kNumSamples = 200;
const uint16_t kMaxRayDepth = 50;

const double kMinDepth = 0.01f;
const double kMaxDepth = 5.f;

const double kIORAir = 1.f;

const uint8_t kNumThreads = 24;

struct RayTestResult
{
    Color image;
    Color reflect;
    Color normal;
    Color hit;
    double depth;
};


Color rayColor(const hvk::Ray& r, entt::registry& registry, int depth, std::optional<RayTestResult>& outResult)
{
    if (depth <=0)
    {
        return Color(0.f, 0.f, 0.f);
    }

    hvk::HitRecord earliestHitRecord = {};
    earliestHitRecord.t = std::numeric_limits<double>::max();
    hvk::Material earliestMaterial(hvk::MaterialType::Diffuse, hvk::Color(0.f, 0.f, 0.f), -1.f);

    auto sphereView = registry.view<hvk::Sphere, hvk::Material>();
    for (const auto entity : sphereView)
    {
        const auto& sphere = sphereView.get<hvk::Sphere>(entity);
        const auto& material = sphereView.get<hvk::Material>(entity);
        auto intersection = hvk::hit::SphereRayIntersect(sphere, r);
        if (intersection.has_value() && intersection.value() > 0.f && intersection.value() < earliestHitRecord.t)
        {
            earliestHitRecord.t = intersection.value();
            earliestHitRecord.point = r.PointAt(earliestHitRecord.t);
            earliestHitRecord.normal = (earliestHitRecord.point - sphere.getCenter()).Normalized();
//            if (hvk::Vector::Dot(earliestHitRecord.normal, r.getDirection().Normalized()) > 0.f)
//            {
//                int blah = 5 + 5;
//                std::cout << blah << std::endl;
//            }
            earliestMaterial = material;
        }
    }

    // test for plane intersections
    auto planeView = registry.view<hvk::Plane, hvk::Material>();
    for (const auto entity : planeView)
    {
        const auto& plane = planeView.get<hvk::Plane>(entity);
        const auto& material = planeView.get<hvk::Material>(entity);
        auto intersection = hvk::hit::PlaneRayIntersect(plane, r);
        if (intersection.has_value() && intersection.value() > 0.f && intersection.value() < earliestHitRecord.t)
        {
            earliestHitRecord.t = intersection.value();
            earliestHitRecord.point = r.PointAt(earliestHitRecord.t);
            earliestHitRecord.normal = plane.getDirection().Normalized();
            earliestMaterial = material;
        }
    }

    // test for box intersections
    // on a successful hit test we'll need:
    //  t
    //  the normal of the plane the intersection occurs at
     auto boxView = registry.view<hvk::Box, hvk::Material>();
     for (const auto entity : boxView)
     {
         const auto& box = boxView.get<hvk::Box>(entity);
         const auto& material = boxView.get<hvk::Material>(entity);
         auto intersection = hvk::hit::BoxRayIntersect(box, r);
         if (intersection.has_value() && intersection.value().second > 0.f && intersection.value().second < earliestHitRecord.t)
         {
             earliestHitRecord.t = intersection.value().second;
             earliestHitRecord.point = r.PointAt(earliestHitRecord.t);
             earliestHitRecord.normal = box.getSide(intersection.value().first).getDirection().Normalized();
             earliestMaterial = material;
         }
     }

    if (earliestHitRecord.t < std::numeric_limits<double>::max())
    {
        if (depth == kMaxRayDepth && outResult.has_value())
        {
            auto& result = outResult.value();
            result.hit += earliestHitRecord.point;
            result.depth += earliestHitRecord.t;
            result.normal += 0.5f * Color(
                    earliestHitRecord.normal.X() + 1,
                    earliestHitRecord.normal.Y() + 1,
                    earliestHitRecord.normal.Z() + 1);
            auto reflected = hvk::Vector::Reflect(r.getDirection(), earliestHitRecord.normal);
            result.reflect += 0.5f * Color(reflected.X() + 1, reflected.Y() + 1, reflected.Z() + 1);
            result.image += Color(0.f, 0.f, 0.f);
        }

        hvk::Ray scattered = hvk::Ray(hvk::Vector(), hvk::Vector());
        hvk::Color attenuation(0.f, 0.f, 0.f);
        bool shouldContinue = false;
        if (earliestMaterial.getType() == hvk::MaterialType::Diffuse)
        {
            shouldContinue = hvk::ScatterDiffuse(r, earliestMaterial, earliestHitRecord, attenuation, scattered);
        }
        else if (earliestMaterial.getType() == hvk::MaterialType::Metal)
        {
            shouldContinue = hvk::ScatterMetal(r, earliestMaterial, earliestHitRecord, attenuation, scattered);
            if (!shouldContinue)
            {
                return hvk::Color(0.f, 0.f, 1.f);
            }
        }
        else if (earliestMaterial.getType() == hvk::MaterialType::Dielectric)
        {
            shouldContinue = hvk::ScatterDielectric(r, earliestMaterial, kIORAir, earliestHitRecord, attenuation, scattered);
        }

        if (shouldContinue)
        {
//            // add biasing
//            scattered = hvk::Ray(scattered.getOrigin() + (0.01) * earliestHitRecord.normal.Normalized(), scattered.getDirection());
            return attenuation * rayColor(scattered, registry, depth-1, outResult);
        }

        return hvk::Color(0.f, 0.f, 0.f);
    }
    else
    {
        hvk::Vector unitDirection = r.getDirection();
        auto t = (unitDirection.Y() + 1.f) * 0.5f;
        return (kSkyColor1 * (1.0 - t)) + (kSkyColor2 * t);
    }
}

void writeColor(const Color& c)
{
    auto ir = static_cast<int>(255.999 * c.X());
    auto ig = static_cast<int>(255.999 * c.Y());
    auto ib = static_cast<int>(255.999 * c.Z());

    std::cout << ir << ' ' << ig << ' ' << ib << std::endl;
}

void writeBuffers(
        const std::vector<Color>& colors,
        uint16_t imageWidth,
        uint16_t imageHeight,
        const std::optional<std::vector<double>>& depth,
        const std::optional<std::vector<Color>>& normals,
        const std::optional<std::vector<Color>>& reflects,
        const std::optional<std::vector<Color>>& hits)
{
std::vector<std::vector<Color>> buffers;
    buffers.push_back(colors);

    // buffers.push_back(depth);
    // buffers.push_back(normals);
    // buffers.push_back(reflects);
    if (depth.has_value())
    {
        // prepare depth buffer as colors
        std::vector<Color> depthColors;
        depthColors.resize(depth->size());
        double minDepth = std::numeric_limits<double>().max();
        double maxDepth = std::numeric_limits<double>().min();
        for (auto d : depth.value())
        {
            if (d < minDepth)
            {
                minDepth = d;
            }
            if (d > maxDepth)
            {
                maxDepth = d;
            }
        }
        for (size_t i = 0; i < depthColors.size(); ++i)
        {
            const double c = (depth->at(i) - minDepth) / (maxDepth - minDepth);
            depthColors[i] = Color(c, c, c);
        }
        buffers.push_back(depthColors);
    }
    if (normals.has_value())
    {
        buffers.push_back(normals.value());
    }
    if (reflects.has_value())
    {
        buffers.push_back(reflects.value());
    }
    if (hits.has_value())
    {
        buffers.push_back(hits.value());
    }

    const auto numColumns = static_cast<uint16_t>(std::ceil(sqrt(buffers.size())));
    const auto numRows = numColumns;
    const uint16_t width = imageWidth * numColumns;
    const uint16_t height = imageHeight * numRows;

    std::cout << "P3\n" << width << ' ' << height << "\n255\n";

    std::vector<Color> finalBuffer;
    finalBuffer.resize(width * height);

    size_t row = 0;
    size_t column = 0;
    for (const auto& buffer : buffers)
    {
        const size_t viewportRowOffset = row * (width * imageHeight);
        const size_t viewportColumnOffset = column * imageWidth;

        for (size_t scanline = 0; scanline < buffer.size() / imageWidth; ++scanline)
        {
            const size_t srcOffset = scanline * imageWidth;
            const size_t copySize = imageWidth * sizeof(hvk::Color);
            const size_t destScanlineOffset = scanline * width;
            const size_t destOffset = viewportRowOffset + viewportColumnOffset + destScanlineOffset;
            memcpy(
                reinterpret_cast<hvk::Color*>(finalBuffer.data()) + destOffset,
                reinterpret_cast<const hvk::Color*>(buffer.data()) + srcOffset,
                copySize);
        }

        ++column;
        if (column >= numColumns)
        {
            column = 0;
            ++row;
        }
    }

    for (const auto& c : finalBuffer)
    {
        writeColor(c);
    }
}

int main() {
    entt::registry registry;

    // Image setup
    const auto aspectRatio = 16.f / 9.f;
    const uint16_t imageWidth = 600;
    const uint16_t imageHeight = static_cast<uint16_t>(imageWidth / aspectRatio);
    std::vector<Color> writeOutBuffer;
    writeOutBuffer.resize(imageHeight * imageWidth);
    std::vector<Color> normalBuffer;
    normalBuffer.resize(imageHeight * imageWidth);
    std::vector<Color> reflectBuffer;
    reflectBuffer.resize(imageHeight * imageWidth);
    std::vector<double> depthBuffer;
    depthBuffer.resize(imageHeight * imageWidth);
    std::vector<Color> hitBuffer;
    hitBuffer.resize(imageHeight * imageWidth);

    // Camera setup
    const hvk::Camera camera(
            hvk::Vector(-1.5f, .6f, 0.8f),
            hvk::Vector(0.f, 0.f, -1.f),
            hvk::Vector(0.f, 1.f, 0.f),
            60.f,
            aspectRatio,
            0.001f,
            2.f);

    // World / Scene
    auto sphereEntity = registry.create();
    registry.emplace<hvk::Sphere>(sphereEntity, hvk::Vector(0.0f, 0.0f, -1.f), 0.5f);
    registry.emplace<hvk::Material>(sphereEntity, hvk::MaterialType::Diffuse, hvk::Color(1.f, 0.f, 0.f), -1.f);

//    auto groundEntity = registry.create();
//    registry.emplace<hvk::Sphere>(groundEntity, hvk::Vector(0, -100.5f, -1.f), 100.f);
//    registry.emplace<hvk::Material>(groundEntity, hvk::MaterialType::Diffuse, hvk::Color(0.f, 1.f, 0.f), -1.f);

//    auto metalSphere1 = registry.create();
//    registry.emplace<hvk::Sphere>(metalSphere1, hvk::Vector(-1.0f, 0.f, -1.f), 0.5f);
//    registry.emplace<hvk::Material>(metalSphere1, hvk::MaterialType::Metal, hvk::Color(1.f, 1.f, 1.f));

    auto metalSphere2 = registry.create();
    registry.emplace<hvk::Sphere>(metalSphere2, hvk::Vector(1.0f, 0.f, -1.f), 0.5f);
    registry.emplace<hvk::Material>(metalSphere2, hvk::MaterialType::Metal, hvk::Color(1.f, 1.f, 1.f), -1.f);

    auto behindSphere = registry.create();
    registry.emplace<hvk::Sphere>(behindSphere, hvk::Vector(0.5f, 0.0f, 1.f), 0.5f);
    registry.emplace<hvk::Material>(behindSphere, hvk::MaterialType::Diffuse, hvk::Color(1.f, 0.f, 1.f), -1.f);

    auto glassSphere = registry.create();
    registry.emplace<hvk::Sphere>(glassSphere, hvk::Vector(2.f, 0.f, -1.f), 0.5f);
    registry.emplace<hvk::Material>(glassSphere, hvk::MaterialType::Dielectric, hvk::Color(0.8f, 0.8f, 0.8f), 1.5);

    auto smallGlass = registry.create();
    registry.emplace<hvk::Sphere>(smallGlass, hvk::Vector(0.3f, -.3f, -0.4f), 0.2f);
    registry.emplace<hvk::Material>(smallGlass, hvk::MaterialType::Dielectric, hvk::Color(1.f, 1.f, 1.f), 1.5);

    auto smallGlass2 = registry.create();
    registry.emplace<hvk::Sphere>(smallGlass2, hvk::Vector(-0.32f, -.3f, -0.42f), 0.2f);
    registry.emplace<hvk::Material>(smallGlass2, hvk::MaterialType::Dielectric, hvk::Color(1.f, 1.f, 1.f), 1.5);

    auto smallMetalSphere = registry.create();
    registry.emplace<hvk::Sphere>(smallMetalSphere, hvk::Vector(-0.68f, -.3f, -0.69f), 0.25f);
    registry.emplace<hvk::Material>(smallMetalSphere, hvk::MaterialType::Metal, hvk::Color(0.7f, 0.2f, 0.7f), -1.f);

    auto groundPlane = registry.create();
    registry.emplace<hvk::Plane>(groundPlane, hvk::Vector(0.f, -0.5f, 0.f), hvk::Vector(0.f, 1.f, 0.f));
    registry.emplace<hvk::Material>(groundPlane, hvk::MaterialType::Diffuse, hvk::Color(0.8f, 0.8f, 0.8f), -1.f);

    auto smallMetalBox = registry.create();
    registry.emplace<hvk::Box>(
            smallMetalBox,
            hvk::Plane(hvk::Vector(0.f, 0.5f, -4.f), hvk::Vector(0.f, 1.f, 0.f)), // top
            hvk::Plane(hvk::Vector(0.f, -0.5f, -4.f), hvk::Vector(0.f, -1.f, 0.f)), // bottom
            hvk::Plane(hvk::Vector(0.f, 0.f, -3.5f), hvk::Vector(0.f, 0.f, 1.f)), // front
            hvk::Plane(hvk::Vector(0.f, 0.f, -4.5f), hvk::Vector(0.f, 0.f, -1.f)), // back
            hvk::Plane(hvk::Vector(-0.5f, 0.f, -4.f), hvk::Vector(-1.f, 0.f, 0.f)), // left
            hvk::Plane(hvk::Vector(0.5f, 0.f, -4.f), hvk::Vector(1.f, 0.f, 0.f))); // right
    registry.emplace<hvk::Material>(smallMetalBox, hvk::MaterialType::Metal, hvk::Color(0.8f, 0.6f, 0.1f), -1.f);

    auto glassBox = registry.create();
    registry.emplace<hvk::Box>(
            glassBox,
            hvk::Plane(hvk::Vector(-0.5f, 0.0f, -2.5f), hvk::Vector(0.f, 1.f, 0.f)), // top
            hvk::Plane(hvk::Vector(-0.5f, -0.5f, -2.5f), hvk::Vector(0.f, -1.f, 0.f)), // bottom
            hvk::Plane(hvk::Vector(-0.5f, -0.25f, -2.f), hvk::Vector(0.f, 0.f, 1.f)), // front
            hvk::Plane(hvk::Vector(-0.5f, -0.25f, -2.5f), hvk::Vector(0.f, 0.f, -1.f)), // back
            hvk::Plane(hvk::Vector(-1.f, -0.25f, -2.5f), hvk::Vector(-1.f, 0.f, 0.f)), // left
            hvk::Plane(hvk::Vector(-0.5f, -0.25f, -2.5f), hvk::Vector(1.f, 0.f, 0.f))); // right
    registry.emplace<hvk::Material>(glassBox, hvk::MaterialType::Dielectric, hvk::Color(.9f, .9f, .9f), 1.5f);

//    auto diffuseBox = registry.create();
//    registry.emplace<hvk::Box>(
//            diffuseBox,
//            hvk::Plane(hvk::Vector(-1.5f, 1.f, -2.f), hvk::Vector(0.f, 1.f, 0.f)),
//            hvk::Plane(hvk::Vector(-1.5f, -0.5f, -2.f), hvk::Vector(0.f, -1.f, 0.f)),
//            hvk::Plane(hvk::Vector(-1.5f, 0.25f, -1.f), hvk::Vector(0.f, 0.f, 1.f)),
//            hvk::Plane(hvk::Vector(-1.5f, 0.25f, -3.f), hvk::Vector(0.f, 0.f, -1.f)),
//            hvk::Plane(hvk::Vector(-2.5f, 0.25f, -2.f), hvk::Vector(-1.f, 0.f, 0.f)),
//            hvk::Plane(hvk::Vector(-1.f, 0.25f, -2.f), hvk::Vector(1.f, 0.f, 0.f)));
//    registry.emplace<hvk::Material>(diffuseBox, hvk::MaterialType::Diffuse, hvk::Color(0.66, 0.2, 0.8));

    // auto metalBox = registry.create();
    // registry.emplace<hvk::Box>(
    //         metalBox,
    //         hvk::Plane(hvk::Vector(-1.5f, 1.f, -2.f), hvk::Vector(0.f, 1.f, 0.f)),
    //         hvk::Plane(hvk::Vector(-1.5f, -0.5f, -2.f), hvk::Vector(0.f, -1.f, 0.f)),
    //         hvk::Plane(hvk::Vector(-1.5f, 0.25f, 0.f), hvk::Vector(0.f, 0.f, 1.f)),
    //         hvk::Plane(hvk::Vector(-1.5f, 0.25f, -3.f), hvk::Vector(0.f, 0.f, -1.f)),
    //         hvk::Plane(hvk::Vector(-2.5f, 0.25f, -2.f), hvk::Vector(-1.f, 0.f, 0.f)),
    //         hvk::Plane(hvk::Vector(-1.f, 0.25f, -2.f), hvk::Vector(1.f, 0.f, 0.f)));
    // registry.emplace<hvk::Material>(metalBox, hvk::MaterialType::Metal, hvk::Color(.8f, .8f, .8f), -1.f);

    {
        // Create thread pool
        hvk::ThreadPool pool(kNumThreads);

        // Render
        for (int i = imageHeight - 1; i >= 0; --i)
        {
            for (int j = 0; j < imageWidth; ++j)
            {
                pool.QueueWork([&, i, j]()
               {
                   Color pixelColor(0.f, 0.f, 0.f);
                   auto result = std::make_optional(RayTestResult{});
                   for (size_t s = 0; s < kNumSamples; ++s)
                   {
                       auto u = static_cast<double>(j + hvk::math::getRandom<double, 0.0, 1.0>()) /
                                (imageWidth - 1);
                       auto v = static_cast<double>(i + hvk::math::getRandom<double, 0.0, 1.0>()) /
                                (imageHeight - 1);

                       hvk::Ray skyRay = camera.GetRay(u, v);
                       pixelColor += rayColor(skyRay, registry, kMaxRayDepth, result);
                   }
                   const size_t writeIndex = ((imageHeight - 1) - i) * imageWidth + j;
                   // const hvk::Color normalizedHit = 0.5f * hvk::Color(
                   //         result->hit.X() / viewportWidth + 1,
                   //         result->hit.Y() / viewportHeight + 1,
                   //         -result->hit.Z() / 1.5f);
                   writeOutBuffer[writeIndex] = (pixelColor / kNumSamples);
                   depthBuffer[writeIndex] = (result->depth / kNumSamples);
                   normalBuffer[writeIndex] = (result->normal / kNumSamples);
                   reflectBuffer[writeIndex] = (result->reflect / kNumSamples);
                   // hitBuffer[writeIndex] = (normalizedHit / kNumSamples);
               });
            }
        }
    }

    writeBuffers(
        writeOutBuffer,
        imageWidth,
        imageHeight,
        std::make_optional(depthBuffer),
        std::make_optional(normalBuffer),
        std::make_optional(reflectBuffer),
        std::nullopt);
        // std::make_optional(hitBuffer));

    return 0;
}
