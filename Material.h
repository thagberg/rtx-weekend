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
        Metal
    };


    class Material
    {
    public:
        Material(MaterialType type, const Color& albedo);
        MaterialType getType() const;
        Color getAlbedo() const;
    private:
        MaterialType mType;
        Color mAlbedo;
    };


    bool ScatterDiffuse(const Ray &r, const Material &material, const HitRecord &hitRecord, Color &attenuation,
                        Ray &scattered);
    bool ScatterMetal(const Ray &r, const Material &material, const HitRecord &hitRecord, Color &attenuation,
                      Ray &scattered);
}


#endif //RTX_WEEKEND_MATERIAL_H
