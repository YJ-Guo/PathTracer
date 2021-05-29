#include "orennayarbrdf.h"
#include "warpfunctions.h"


OrenNayarBRDF::OrenNayarBRDF(const Color3f &R, float sigma) :
    BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)),
    R(R)
{
    sigma = glm::radians(sigma);
    float sigma2 = sigma * sigma;
    A = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
    B = 0.45f * sigma2 / (sigma2 + 0.09f);
}

Color3f OrenNayarBRDF::f(const Vector3f &wo, const Vector3f &wi) const {
    float sinThetaI = SinTheta(wi);
    float sinThetaO = SinTheta(wo);

    // Compute cosine term of Oren-Nayar Model
    float maxCos = 0;

    if (sinThetaI > 1e-4 && sinThetaO > 1e-4) {
        float sinPhiI = SinPhi(wi), cosPhiI = CosPhi(wi);
        float sinPhiO = SinPhi(wo), cosPhiO = CosPhi(wo);
        float dCos = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
        maxCos = std::max(0.f, dCos);
    }

    // Compute sine and tangent terms of Oren-Nayar model
    float sinAlpha, tanBeta;
    if (AbsCosTheta(wi) > AbsCosTheta(wo)) {
        sinAlpha = sinThetaO;
        tanBeta = sinThetaI / AbsCosTheta(wi);
    } else {
        sinAlpha = sinThetaI;
        tanBeta = sinThetaO / AbsCosTheta(wo);
    }

    return R * InvPi * (A + B * maxCos * sinAlpha * tanBeta);
}
