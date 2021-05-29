#include "photonmapper.h"
#include "directlightingintegrator.h"
#include <iostream>

PhotonMapper::PhotonMapper(int numPhotons, std::vector<Photon> *photons, std::vector<Photon>* causticPhotons, Scene *s, std::shared_ptr<Sampler> sampler, int recursionLimit)
    : Integrator(Bounds2i(Point2i(0,0), Point2i(0,0)), s, sampler, recursionLimit), IsGenerated(true), numPhotons(numPhotons), photons(photons), causticPhotons(causticPhotons)
{}

PhotonMapper::PhotonMapper(Bounds2i bounds, Scene *s, std::shared_ptr<Sampler> sampler, int recursionLimit, KDTree *kdtree, KDTree *causticKDtree, float searchRadius)
    : Integrator(bounds, s, sampler, recursionLimit), IsGenerated(false), numPhotons(0), photons(nullptr), kdtree(kdtree), causticKDTree(causticKDtree), searchRadius(searchRadius)
{}

void PhotonMapper::Render() {
    // If the Photon has not been generated, then generate photons
    if(IsGenerated) {
        // Averaging photons to each light source
        int photonEachLight = numPhotons / scene->lights.size();
        bool SpecularBounce = false;
        for (auto l : scene->lights) {
            for (int i = 0; i < photonEachLight; ++i) {
                int totalPhotons = 8 * photonEachLight;
                // Currently only consider light of Square plane shape
                Photon curPhoton = l->Sample_Photon(sampler, totalPhotons);
                Ray ray(curPhoton.pos + 0.00001f * curPhoton.wi, curPhoton.wi);
                Color3f throughput(1.f);
                for (int j = 0; j < recursionLimit; ++j) {
                    Vector3f woW = -glm::normalize(ray.direction);
                    Intersection isect;
                    if (scene->Intersect(ray, &isect)) {
                        if (isect.objectHit->material != nullptr) {
                            isect.ProduceBSDF();
                            // Prepare the sample f variables
                            Vector3f wiW(0.f);
                            float pdf = 0.f;
                            BxDFType flags;
                            Color3f f = isect.bsdf->Sample_f(woW, &wiW, sampler->Get2D(), &pdf, BSDF_ALL, &flags);
                            wiW = glm::normalize(wiW);

                            if (IsBlack(f) || pdf == 0.f) break;

                            // Update photon position
                            curPhoton.pos = isect.point;

                            // For specular materials we need to handle differently
                            if (!(flags & BSDF_SPECULAR)) {
                                curPhoton.color *= (throughput / (float)scene->lights.size());
                                curPhoton.wi = woW;
                                curPhoton.PrimHit= isect.objectHit;
                                curPhoton.NormalHit = isect.normalGeometric;

                                // If previous hit is a specular material, then this photon would be stored
                                // in the caustic photon tree.
                                if (SpecularBounce) {
                                    causticPhotons->push_back(curPhoton);
                                }
                                else {
                                    photons->push_back(curPhoton);
                                }
                                // Reset the specular flag
                                SpecularBounce = false;
                            }
                            else {
                                SpecularBounce = true;
                                ray = isect.SpawnRay(wiW);
                                continue;
                            }
                            // Update photon's shooting direction, radiance and throughput
                            curPhoton.wi = wiW;
                            f = isect.bsdf->f(wiW, woW);
                            pdf = isect.bsdf->Pdf(wiW, woW);
                            throughput *= (f * AbsDot(woW, isect.bsdf->normal) / pdf);
                            ray = isect.SpawnRay(wiW);
                        }
                        else {
                            break;
                        }
                    }
                }
            }
        }
    }
    else {
        Integrator::Render(); // Invokes Li for each ray from a pixel
    }
}

Color3f PhotonMapper::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const {
    Color3f result(0.f);
    Vector3f woW = -ray.direction;
    Intersection isect;
    Color3f throughput(1.f); // Throughput for specular material;
    if (scene.Intersect(ray, &isect)) {
        if (isect.objectHit->material == nullptr) {
            result += isect.Le(woW);
        } else {
            isect.ProduceBSDF();
            float rangeArea = (1.f / 3.f) * Pi * searchRadius * searchRadius;
            Ray curRay = ray;
            // Prepare the sample f variables
            Vector3f wiW(0.f);
            float curPDF;
            BxDFType curFlag;
            Color3f curF;
            curF = isect.bsdf->Sample_f(woW, &wiW, sampler->Get2D(), &curPDF, BSDF_ALL, &curFlag);
            // Check for specular surfaces
            int specCounter = 0;
            std::vector<Vector3f> checkWiVec;
            checkWiVec.push_back(wiW);

            if (curFlag & BxDFType::BSDF_SPECULAR) {
                curRay = Ray(isect.point + 0.1f * wiW, wiW);
                throughput *= ((curF * AbsDot(woW, isect.bsdf->normal)) / curPDF);
                // For every specular suface, we dirctly find the reflection direction
                while (curFlag & BxDFType::BSDF_SPECULAR) {
                    specCounter++;
                    Intersection curIsect;
                    woW = -wiW;
                    if (scene.Intersect(curRay, &curIsect)) {
                        if(curIsect.objectHit->material == nullptr) {
                            return curIsect.Le(woW);
                        }
                    } else {
                        return Color3f(0.f);
                    }
                    // Produce BSDF at this intersection
                    curIsect.ProduceBSDF();
                    curF = curIsect.bsdf->Sample_f(woW, &wiW, sampler->Get2D(), &curPDF, BSDF_ALL, &curFlag);
                    checkWiVec.push_back(wiW);
                    if(curFlag & BxDFType::BSDF_SPECULAR) {
                        throughput *= ((curF * AbsDot(woW, curIsect.bsdf->normal)) / curPDF);
                    }
                    if(!(curFlag & BxDFType::BSDF_SPECULAR)) {
                        isect = curIsect;
                        break;
                    }
                    curRay = curIsect.SpawnRay(wiW);
                }
            }

            // For direct and indirect light
            std::vector<const Photon*> rangePhotons = kdtree->particlesInSphere(isect.point, searchRadius);
            Color3f photonResult(0.f);
            int countedPhoton = 0;
            for (auto p : rangePhotons) {
                float pdf = isect.bsdf->Pdf(woW, p->wi);
                Color3f pf = isect.bsdf->f(woW, p->wi);
                if ((p->PrimHit == isect.objectHit) && Parallel(p->NormalHit, isect.normalGeometric)) {
                    ++countedPhoton;
                    if(IsBlack(pf) || pdf <= 0.f) {
                        continue;
                    }
                    float disWeight = 1 - (glm::length(p->pos - isect.point) / searchRadius);
                    Color3f curPhoton = (pf * p->color * disWeight) * AbsDot(isect.normalGeometric, p->wi) / pdf;
                    photonResult += curPhoton;
                }
            }

            // Accumulate valid photon data
            if (countedPhoton != 0) {
                photonResult /= rangeArea;
                result = photonResult * throughput;
            }

            // For caustic photons
            // Similar to direct/indirect but different photon set
            std::vector<const Photon*> rangeCausticPhotons = causticKDTree->particlesInSphere(isect.point, searchRadius);
            Color3f causticPhotonResult(0.f);
            int countedCausticPhoton = 0;
            for (const Photon *p : rangeCausticPhotons) {
                float pdf = isect.bsdf->Pdf(woW, p->wi);
                Color3f pf = isect.bsdf->f(woW, p->wi);
                if ((p->PrimHit == isect.objectHit) && Parallel(p->NormalHit, isect.normalGeometric)) {
                    ++countedCausticPhoton;

                    if(IsBlack(pf) || pdf <= 0.f) {
                        continue;
                    }
                    float disWeight = 1 - (glm::length(p->pos - isect.point) / searchRadius);
                    Color3f curPhoton = (pf * p->color * disWeight) * AbsDot(isect.normalGeometric, p->wi) / pdf;
                    causticPhotonResult += curPhoton;
                }
            }

            // Accumulate valid photon data
            if(countedCausticPhoton != 0) {
                causticPhotonResult /= rangeArea;
                result += (causticPhotonResult * throughput);
            }
        }
    }
    return result;
}

