#include "fulllightingintegrator.h"
#include "directlightingintegrator.h"

Color3f FullLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    // Instantiate an accumulated ray color that begins as black
    Color3f L(0.f);
    // An accumulated ray throughput color that begins as white
    // The throughput will be used to determine when your ray path terminates via the Russian Roulette heuristic.
    Color3f throughPut(1.f);

    int bounces = depth;
    Ray FRay = ray;
    // Check for specular materials and deal with it properly
    bool specularBounce = false;

    // Before the recursion limit
    while (bounces > 0) {
        Intersection isect;
        bool foundIsect = scene.Intersect(FRay, &isect);

        // while there is intersection
        Vector3f woW = -FRay.direction;

        // First compute self emission light
        // only compute the self emisson once for first encounter
        if (bounces == depth || specularBounce) {
            if (foundIsect) {
                // Get self emission
                Color3f lightEmit = isect.Le(woW);
                // Add to final color
                L += throughPut * lightEmit;
            }
        }

        // if there is no intersection for this ray
        if (!foundIsect) {
            break;
        }

        // if hit light, stop bouncing
        if(isect.objectHit->GetAreaLight()) {
            break;
        }

        // ProduceBSDF with given material
        isect.ProduceBSDF();

        // Skip for specular for Now
        if (isect.bsdf->BxDFsMatchingFlags(BxDFType(BSDF_ALL & ~BSDF_SPECULAR)) > 0) {
            // Computing the direct lighting component
            // Add the dirct illumination part into account
            Color3f LDirect = DirectLightingPart(FRay, scene, sampler, bounces);
            L += throughPut * LDirect;
        }

        // Computing the ray bounce and global illumination
        // Compute the information for next ray after a bounce
        Vector3f wiN(0.f);
        float NextPDF = 0.f;
        Color3f f = isect.bsdf->Sample_f(woW, &wiN, sampler->Get2D(), &NextPDF);
        // Avoid dividing by 0 or no energy for next light
        if (IsBlack(f) || NextPDF == 0) {
            break;
        }

        throughPut *= f * AbsDot(wiN, isect.normalGeometric) / NextPDF;
        FRay = isect.SpawnRay(glm::normalize(wiN));

        // Russian Roulette Ray Termination
        // Compare the maximum RGB component of throughput
        // to a uniform random number and stop your while loop
        // if said component is smaller than the random number.
        if (depth - bounces > 3) {
            float maxComponent = FLT_MIN;
            maxComponent = glm::max(throughPut.r, throughPut.g);
            maxComponent = glm::max(maxComponent, throughPut.b);

            float q = sampler->Get1D();
            if (maxComponent < (1 - q)) {
                break;
            } else {
                // Divide throughput by the maximum component of throughput
                // to keep the path tracer mathematically correct
                throughPut /= maxComponent;
            }
        }
        --bounces;
    }
    return L;
}


