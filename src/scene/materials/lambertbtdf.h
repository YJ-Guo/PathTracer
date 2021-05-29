#ifndef LAMBERTBTDF_H
#define LAMBERTBTDF_H
#pragma once
#include "bsdf.h"
#include <warpfunctions.h>

// Follow the instrcutions in PBRT LambertTransmission
class LambertBTDF : public BxDF{
public:
    LambertBTDF(const Color3f &T);
    Color3f f(const Vector3f &wo, const Vector3f &wi) const;
    Color3f Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi, Float *pdf, BxDFType *sampledType) const;
    float Pdf(const Vector3f &wo, const Vector3f &wi) const;

private:
    // The color of this BRDF
    // T for transmission
    const Color3f T;
};

#endif // LAMBERTBTDF_H
