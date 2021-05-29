#pragma once
#include "material.h"

// Follow PBRT metal maertial class
class MetalMaterial : public Material {
public:
    MetalMaterial(const Color3f &eta, const Color3f &k, Float roughness)
        : eta(eta), k(k), roughness(roughness)
    {}

    void ProduceBSDF(Intersection *isect) const;

private:
    Color3f eta;
    Color3f k;
    Float roughness;
};
