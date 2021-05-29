#ifndef POINTLIGHT_H
#define POINTLIGHT_H
#include "light.h"

// Follow PBRT book Ch.12.3
class PointLight : public AreaLight {
public:
    // An isotropic point light source that emits the same amount of light in all directions
    // But point light dosen't have surface area
    PointLight(const Transform &t, const Color3f& Le)
        :AreaLight(t), pLight(Point3f(t.position())) ,emittedLight(Le)
    {}

    virtual Color3f L(const Intersection &isect, const Vector3f &w) const;

    Color3f Sample_Li(const Intersection &ref, const Point2f &xi,
                                         Vector3f *wi, Float *pdf) const;

    virtual Color3f Sample_Le(const Point2f &u1, const Point2f &u2, Ray *ray, Normal3f *nLight, Float *pdfPos, Float *pdfDir) override;


    virtual float Pdf_Li(const Intersection &ref, const Vector3f &wi) const;

    // Member variables
    const Point3f pLight;
    const Color3f emittedLight;
};

#endif // POINTLIGHT_H
