#include "specularbrdf.h"

// Detail of Implementing this class is based on BPRT book Ch.8.2.2 & Ch.14.1.3
// f & Pdf return 0 for the reason that there is "zero" possibility that
// another sampling method will randomly find the direction from a delta distribution
Color3f SpecularBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.f);
}


float SpecularBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 0.f;
}

Color3f SpecularBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const
{
    // wi then is the perfect specular reflection direction
    *wi = Vector3f(-wo.x, -wo.y, wo.z);
    *pdf = 1.f;
    return fresnel->Evaluate(CosTheta(*wi)) * R / AbsCosTheta(*wi);
}
