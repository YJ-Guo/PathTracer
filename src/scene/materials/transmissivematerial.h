#pragma once
#include "material.h"

class TransmissiveMaterial : public Material
{
public:
    TransmissiveMaterial(const Color3f &Kt, float roughness, float indexOfRefraction,
                  const std::shared_ptr<QImage> &roughnessMap,
                  const std::shared_ptr<QImage> &textureMap,
                  const std::shared_ptr<QImage> &normalMap)
        : Kt(Kt), roughness(roughness), indexOfRefraction(indexOfRefraction),
          roughnessMap(roughnessMap), textureMap(textureMap), normalMap(normalMap)
    {}

    void ProduceBSDF(Intersection *isect) const;


private:
    Color3f Kt;                    // The spectral transmission of this material.
                                   // This is just the base color of the material
    float roughness;               // Add roughness term to support MircofacetTransmission

    float indexOfRefraction;       // The IoR of this glass. We assume the IoR of
                                   // any external medium is that of a vacuum: 1.0

    // The corresponding roughness Map for MircofacetTransmission
    std::shared_ptr<QImage> roughnessMap;
    std::shared_ptr<QImage> textureMap; // The color obtained from this (assuming it is non-null) is multiplied with the base material color.
    std::shared_ptr<QImage> normalMap;
};
