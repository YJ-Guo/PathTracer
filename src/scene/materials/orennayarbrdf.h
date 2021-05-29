#ifndef ORENNAYAR_H
#define ORENNAYAR_H
#pragma once
#include "bsdf.h"

// Follow the book PBRT
class OrenNayarBRDF : public BxDF {
public:
    OrenNayarBRDF(const Color3f &R, float sigma);
    Color3f f(const Vector3f &wo, const Vector3f &wi) const;

private:
    const Color3f R;
    float A, B;

};

#endif // ORENNAYAR_H
