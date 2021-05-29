#include "fresnel.h"

Color3f FresnelDielectric::Evaluate(float cosThetaI) const
{
    // Call FrDielectric function in PBRT
    cosThetaI = glm::clamp(cosThetaI, -1.f, 1.f);

    // Creat temp etas for swaping in function
    float tempEtaI = etaI;
    float tempEtaT = etaT;
    // Potentially swap indices of refraction
    bool entering = cosThetaI > 0.f;
    if (!entering) {
        std::swap(tempEtaI, tempEtaT);
        cosThetaI = std::abs(cosThetaI);
    }

    // Compute cosThetaT using Snell's law
    Float sinThetaI = std::sqrt(std::max((Float)0, 1 - cosThetaI * cosThetaI));

    Float sinThetaT = tempEtaI / tempEtaT * sinThetaI;

    // Handle total internal reflection
    if (sinThetaT >= 1) {
        return Color3f(1.f);
    }

    Float cosThetaT = std::sqrt(std::max((Float)0, 1 - sinThetaT * sinThetaT));

    Float Rpar1 = ((tempEtaT * cosThetaI) - (tempEtaI * cosThetaT)) /
            ((tempEtaT * cosThetaI) + (tempEtaI * cosThetaT));

    Float Rperp = ((tempEtaI * cosThetaI) - (tempEtaT * cosThetaT)) /
            ((tempEtaI * cosThetaI) + (tempEtaT * cosThetaT));

    return Color3f((Rpar1 * Rpar1 + Rperp * Rperp) / 2.f);
}

// Use Eqn.8.3 in PBRT to evaluate
Color3f FresnelConductor::Evaluate(float cosThetaI) const {
    cosThetaI = glm::clamp(cosThetaI, -1.f, 1.f);
    Color3f eta = etaT / etaI;
    Color3f etak = k / etaI;

    Float cosThetaI2 = cosThetaI * cosThetaI;
    Float sinThetaI2 = 1.f - cosThetaI2;
    Color3f eta2 = eta * eta;
    Color3f etak2 = etak * etak;

    Color3f t0 = eta2 - etak2 - sinThetaI2;
    Color3f a2plusb2 = glm::sqrt(t0 * t0 + 4.f * eta2 * etak2);
    Color3f t1 = a2plusb2 + cosThetaI2;
    Color3f a = glm::sqrt(0.5f * (a2plusb2 + t0));
    Color3f t2 = (Float)2.f * cosThetaI * a;
    Color3f Rs = (t1 - t2) / (t1 + t2);

    Color3f t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
    Color3f t4 = t2 * sinThetaI2;
    Color3f Rp = Rs * (t3 - t4) / (t3 + t4);

    return Color3f((Float)0.5 * (Rp + Rs));
}
