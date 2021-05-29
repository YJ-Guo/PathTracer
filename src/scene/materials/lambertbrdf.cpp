#include "lambertbrdf.h"
#include <warpfunctions.h>

Color3f LambertBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    // Through the integration over the hemisphere
    return R * InvPi;
}

Color3f LambertBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                        Float *pdf, BxDFType *sampledType) const
{
    // Samples the unit hemisphere with a cosine-weighted distribution
    *wi = WarpFunctions::squareToHemisphereCosine(u);

    // flipping the direction if necessary
    if (wo.z < 0) {
        wi->z *= -1;
    }

    *pdf = Pdf(wo, *wi);

    return f(wo, *wi);
}

float LambertBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    // The PDF for Cosine weighted hemisphere sampling
    // pdf = cos(theta) / PI
    // should check if wi and wo are on same side
    return SameHemisphere(wo, wi) ? AbsCosTheta(wi) * InvPi : 0;
}
