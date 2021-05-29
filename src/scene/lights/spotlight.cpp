#include "spotlight.h"

float SpotLight::Falloff(const Vector3f &w) const
{
    Vector3f wl = glm::normalize(transform.invT3() * w);
    float cosTheta = wl.z;
    if (cosTheta < cosTotalWidth) return 0;
    if (cosTheta >= cosFalloffStart) return 1;
    float delta = (cosTheta - cosTotalWidth) / (cosFalloffStart - cosTotalWidth);
    return (delta * delta) * (delta * delta);
}

Color3f SpotLight::L(const Intersection &isect, const Vector3f &w) const
{
    return emittedLight;
}

Color3f SpotLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                     Vector3f *wi, Float *pdf) const
{
    *wi = glm::normalize(pLight - ref.point);
    *pdf = 1.f;
    // return emittedLight;
    // std::cout << Falloff(-*wi) / glm::length2(transform.position() - ref.point) << std::endl;
    return emittedLight * Falloff(-*wi) / glm::length2(pLight - ref.point);
}

float SpotLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    return 0;
}
