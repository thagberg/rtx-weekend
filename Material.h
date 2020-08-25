#ifndef RTX_WEEKEND_MATERIAL_H
#define RTX_WEEKEND_MATERIAL_H

#include "Ray.h"
#include "Vector.h"
#include "HitRecord.h"

namespace hvk
{
    using Color = Vector;

    enum class MaterialType
    {
        Diffuse,
        Metal,
        Dielectric
    };


    class Material
    {
    public:
        Material(MaterialType type, const Color& albedo, double ior);
        MaterialType getType() const;
        Color getAlbedo() const;
        double getIOR() const;
    private:
        MaterialType mType;
        Color mAlbedo;
        double mIOR;
    };


    bool ScatterDiffuse(const Ray &r, const Material &material, const HitRecord &hitRecord, Color &attenuation,
                        Ray &scattered);
    bool ScatterMetal(const Ray &r, const Material &material, const HitRecord &hitRecord, Color &attenuation,
                      Ray &scattered);
    bool ScatterDielectric(
            const Ray &r,
            const Material &enterMaterial,
            double leaveIOR,
            const HitRecord &hitRecord,
            Color &attenuation,
            Ray &scattered);
}


#endif //RTX_WEEKEND_MATERIAL_H
