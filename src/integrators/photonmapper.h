#ifndef PHOTONMAPPER_H
#define PHOTONMAPPER_H
#include "integrator.h"
#include "scene/kdtree.h"

// This part of code follows the PBRT book's section of Photon Mapper
class PhotonMapper : public Integrator
{
public:
    PhotonMapper(int numPhotons, std::vector<Photon>* photons, std::vector<Photon>* causticPhotons, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit);
    PhotonMapper(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit, KDTree *kdtree, KDTree *causticKDtree, float searchRadius);
    virtual void Render();
    virtual Color3f Li(const Ray& ray, const Scene& scene, std::shared_ptr<Sampler> sampler, int depth) const;
private:
    bool IsGenerated;
    int numPhotons;
    std::vector<Photon>* photons;
    std::vector<Photon>* causticPhotons;

    float searchRadius;
    KDTree *kdtree;
    KDTree *causticKDTree;
};


#endif // PHOTONMAPPER_H
