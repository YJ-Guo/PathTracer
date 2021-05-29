#include "diffusearealight.h"

Color3f DiffuseAreaLight::L(const Intersection &isect, const Vector3f &w) const
{
    // return Color3f(0.f);
    if (twoSided) {
        // Suppose DiffuseAreaLights emit in both sides of the hemimsphere
        return emittedLight;
    } else {
        // An area light that emits its energy equally in all directions from its surface
        // Suppose DiffuseAreaLights only emit in one side of the hemimsphere
        return glm::dot(isect.normalGeometric, w) > 0 ? emittedLight : Color3f(0.f);
    }

}

// Follow the PBRT Ch.14.2.3
Color3f DiffuseAreaLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                    Vector3f *wi, Float *pdf) const {
    // Get an Intersection on the surface of its Shape by invoking shape->Sample.
    Intersection pShape = shape->Sample(ref, xi, pdf);

    // Check if the resultant PDF is zero or that the reference Intersection
    // and use distance to determine if the resultant Intersections are the same point in space,
    // and return black if this is the case.
    if (*pdf == 0.f ||
            glm::length(ref.point - pShape.point) < ShadowEpsilon) {
        return Color3f(0.f);
    }

    // Set ωi to the normalized vector from the reference
    // Intersection's point to the Shape's intersection point.
    *wi = glm::normalize(pShape.point - ref.point);

    // Return the light emitted along ωi from our intersection point.
    return this->L(pShape, -*wi);
}

Photon DiffuseAreaLight::Sample_Photon(std::shared_ptr<Sampler> sampler, int numPhotons) const
{
    Photon curPhoton(Point3f(0.f), Color3f(0.f), Vector3f(0.f));
    float pdfDir;
    shape->Sample_Photon(curPhoton, sampler, pdfDir);
    // set power of photons
    curPhoton.color = 0.5f * emittedLight * area / (float)numPhotons;

    return curPhoton;
}

// Follow PBRT Ch.14.2.2
float DiffuseAreaLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    // Intersect sample ray with area light geometry:
    Ray ray = ref.SpawnRay(wi);
    Intersection isectLight;

    // If the ray doesn't hit the geometry
    if(!shape->Intersect(ray, &isectLight))
    {
        return 0.f;
    }

    // Test to avoid dividing by 0
    if (fequal(area, 0.f) || fequal(AbsDot(isectLight.normalGeometric, -wi), 0.f)) {
        return 0.f;
    }

    // Convert light sample weight to solid angle measure:
    float pdf = glm::distance2(ref.point, isectLight.point) / (AbsDot(isectLight.normalGeometric, -wi) * area);

    return pdf;
}
