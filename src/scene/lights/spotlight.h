#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H
#include "light.h"

class SpotLight : public AreaLight {
    // A Spot Light has no surface area and doesn't emit light uniformly in all directions
    // We have to compare the angle between the ray going towards the spotlight's point
    // and the spot light's look direction to see if that spot light should transmit light
    // along wi.
public:
    SpotLight(const Transform &t, const Color3f& Le, float totalWidth, float falloffStart)
        :AreaLight(t), pLight(Point3f(t.position())), emittedLight(Le),
          cosTotalWidth(glm::cos(glm::radians(totalWidth))),
          cosFalloffStart(glm::cos(glm::radians(falloffStart)))
    {}

    virtual Color3f L(const Intersection &isect, const Vector3f &w) const;

    Color3f Sample_Li(const Intersection &ref, const Point2f &xi,
                                         Vector3f *wi, Float *pdf) const;

    virtual float Pdf_Li(const Intersection &ref, const Vector3f &wi) const;

    float Falloff(const Vector3f &w) const;

    const Point3f pLight;
    const Color3f emittedLight;
    const float cosTotalWidth, cosFalloffStart;
};

#endif // SPOTLIGHT_H
