#include "Material.h"

namespace hvk
{
    Material::Material(MaterialType type, const Color &albedo)
        : mType(type)
        , mAlbedo(albedo)
    {}

    MaterialType Material::getType() const
    {
        return mType;
    }

    Color Material::getAlbedo() const
    {
        return mAlbedo;
    }

    bool ScatterDiffuse(const Ray &r, const Material &material, const HitRecord &hitRecord, Color &attenuation,
                        Ray &scattered)
    {
        Vector scatterDirection = hitRecord.normal + Vector::RandomUnit();
        scattered = Ray(hitRecord.point, scatterDirection);
        attenuation = material.getAlbedo();
        return true;
    }

    bool ScatterMetal(const Ray &r, const Material &material, const HitRecord &hitRecord, Color &attenuation,
                      Ray &scattered)
    {
        Vector reflectedDirection = Vector::Reflect(r.getDirection(), hitRecord.normal);
        scattered = Ray(hitRecord.point, reflectedDirection);
        attenuation = material.getAlbedo();
        auto reflectDot = Vector::Dot(reflectedDirection, hitRecord.normal);
        bool goodReflect = reflectDot > 0;
        if (!goodReflect)
        {
            int dummy = 1 + 1;
        }
        return goodReflect;
    }
}
