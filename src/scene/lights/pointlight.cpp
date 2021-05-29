#include "pointlight.h"

Color3f PointLight::L(const Intersection &isect, const Vector3f &w) const
{
    return emittedLight;
}

Color3f PointLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                     Vector3f *wi, Float *pdf) const
{
    *wi = glm::normalize(pLight - ref.point);
    // There is only one direction to cast ray
    *pdf = 1.f;
    // Dividing radiant intensity by the squared distance to the point p to convert units
    // std::cout << glm::length2(pLight - ref.point) << std::endl;
    return emittedLight / glm::length2(pLight - ref.point);
}

float PointLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    return 0;
}

Color3f PointLight::Sample_Le(const Point2f &u1, const Point2f &u2, Ray *ray, Normal3f *nLight, Float *pdfPos, Float *pdfDir)
{
    *ray = Ray(transform.position(), UniformSampleSphere(u1));
    *nLight = ray->direction;
    *pdfPos = 1;
    *pdfDir = UniformSpherePdf();
    return this->emittedLight;
}
