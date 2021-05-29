#include "lambertbtdf.h"

LambertBTDF::LambertBTDF(const Color3f &T) :
    BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_DIFFUSE)),
    T(T)
{}


Color3f LambertBTDF::f(const Vector3f &wo, const Vector3f &wi) const {
    // Same as BTDF
    return T * InvPi;
}

float LambertBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const {
    return !SameHemisphere(wo, wi) ? AbsCosTheta(wi) * InvPi : 0.f;
}

Color3f LambertBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi, Float *pdf, BxDFType *sampledType) const {
    *wi = WarpFunctions::squareToHemisphereCosine(xi);
    // get to the other side of the hemisphere
    if (wo.z > 0) {
        wi->z *= -1;
    }

    *pdf = Pdf(wo, *wi);

    return f(wo,*wi);
}
