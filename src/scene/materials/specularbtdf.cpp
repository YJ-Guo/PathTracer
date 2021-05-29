#include "specularbTdf.h"

// Detail of Implementing this class is based on BPRT book Ch.8.2.3 & Ch.14.1.3
Color3f SpecularBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.f);
}


float SpecularBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 0.f;
}

Color3f SpecularBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const
{
    // Figure out which eta is incident and which is trasmitted
    bool entering = CosTheta(wo) > 0;
    Float etaI = entering ? etaA : etaB;
    Float etaT = entering ? etaB : etaA;

    // Compute ray direction for specular transmission
    if (!Refract(wo, Faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi)) {
        return Color3f(0.f);
    }

    *pdf = 1.f;
    Color3f ft = T * (Color3f(1.f) - fresnel->Evaluate(CosTheta(*wi)));

    // TODO Later: Account for non-symmetry with transmission to different medium

    return ft / AbsCosTheta(*wi);
}
