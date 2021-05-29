#include "directlightingintegrator.h"

float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    return (nf * fPdf) / (nf * fPdf + ng * gPdf);
}

// Power of 2 is a good value
float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf)
{
    Float f = nf * fPdf;
    Float g = ng * gPdf;
    return (f * f) / (f * f + g * g);
}

// Follow the PBRT Ch.14.3
Color3f DirectLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Color3f DLColor(0.f);
    Color3f BSDFColor(0.f);

    // Use while loop for flow control, each brach can break the loop
    // Use MIS: Direct Light Sampling part
    Vector3f woW = -ray.direction;
    Intersection isect;
    // If the ray hits an object in the scene
    if (scene.Intersect(ray, &isect)) {
        // Add self-emitted light
        DLColor += isect.Le(woW);
        // If the hitted object has material for ProduceBSDF
        if (depth > 0 && isect.objectHit->material != nullptr) {
            // ProduceBSDF with given material
            isect.ProduceBSDF();

            int lightNum = scene.lights.size();
            // If no light source, return it's emitted light
            if (lightNum != 0) {
                // Randomly select a light source from scene.lights
                int lightIndex = std::min((int) (sampler->Get1D() * lightNum), lightNum - 1);
                std::shared_ptr<Light> light = scene.lights[lightIndex];

                // and call its Sample_Li function
                Vector3f DLwi(0.f);
                float DLpdf;
                Color3f Li = light->Sample_Li(isect, sampler->Get2D(), &DLwi, &DLpdf);

                // Should also do shadow test to make correct shadow
                Intersection shadowIsect;
                // Move the intersection point along the normal by some distance
                // to make sure the intersection point is on the outter surface of the
                // geometry
                Ray shadowRay(isect.point + isect.normalGeometric * RayEpsilon, DLwi);

                // Two conditions for shadow:
                // The point in the scene can see the light
                // The light seen is the expected light
                if (scene.Intersect(shadowRay, &shadowIsect) &&
                        light.get() == shadowIsect.objectHit->GetAreaLight()) {

                    // Evaluate the Light Transport Equation with this Li
                    Color3f DLf = isect.bsdf->f(woW, DLwi);
                    float BSDFPdf = isect.bsdf->Pdf(woW, DLwi);

                    // Do the Light Transport integration
                    // Divide the PDF by the number of light sources in scene to
                    // account for choosing a light at random uniformly from the set of all lights in the scene.
                    if (fequal(DLpdf, 0.f) || fequal(BSDFPdf, 0.f)) {  // Avoid dividing by zero
                        DLColor += Color3f(0.f);
                    } else {
                        // Use MIS: Direct Light part weight
                        // Two kinds of weighting Functions
                        //float DLWeight = BalanceHeuristic(1, DLpdf, 1, BSDFPdf);
                        float DLWeight = PowerHeuristic(1, DLpdf, 1, BSDFPdf);
                        // Apply weight to LTE
                        DLColor += DLf * Li * AbsDot(DLwi, isect.normalGeometric) * DLWeight / (DLpdf / lightNum);
                    }
                }

//// *************************************************************************************************************

                // Use MIS: BSDF Sampling part
                // Call samplef and get the returned wiW and pdf
                Vector3f wiB(0.f);
                float Bpdf;
                Color3f Bf = isect.bsdf->Sample_f(woW, &wiB, sampler->Get2D(), &Bpdf);

                // If there is no more light energy for bouncing
                // return current color
                if (fequal(Bpdf, 0.f) || IsBlack(Bf)) {
                    BSDFColor += Color3f(0.f);
                } else {
                    // Make use BSDF sampling technique to generate a ray
                    Ray rayEmit = isect.SpawnRay(glm::normalize(wiB));
                    Intersection Bisect;
                    // If this ray can see the correct light
                    if (scene.Intersect(rayEmit, &Bisect) &&
                            light.get() == Bisect.objectHit->GetAreaLight()) {
                        // Find which light the ray is pointing to
                        const AreaLight *lightPtr = Bisect.objectHit->GetAreaLight();
                        float Dpdf = lightPtr->Pdf_Li(isect, wiB);
                        //float BWeight = BalanceHeuristic(1, Bpdf, 1, Dpdf);
                        float BWeight = PowerHeuristic(1, Bpdf, 1, Dpdf);

                        // Do the Light Transport integration
                        BSDFColor += Bf * Li * AbsDot(wiB, isect.normalGeometric) * BWeight / (Bpdf / lightNum);
                    }
                }
            }
        }
    }
    return DLColor + BSDFColor;
}

Color3f DirectLightingPart(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler,int depth) {
    Color3f DLColor(0.f);
    Color3f BSDFColor(0.f);

    // Use while loop for flow control, each brach can break the loop
    // Use MIS: Direct Light Sampling part
    Vector3f woW = -ray.direction;
    Intersection isect;
    // If the ray hits an object in the scene
    if (scene.Intersect(ray, &isect)) {
        // Add self-emitted light
        DLColor += isect.Le(woW);
        // If the hitted object has material for ProduceBSDF
        if (depth > 0 && isect.objectHit->material != nullptr) {
            // ProduceBSDF with given material
            isect.ProduceBSDF();

            int lightNum = scene.lights.size();
            // If no light source, return it's emitted light
            if (lightNum != 0) {
                // Randomly select a light source from scene.lights
                int lightIndex = std::min((int) (sampler->Get1D() * lightNum), lightNum - 1);
                std::shared_ptr<Light> light = scene.lights[lightIndex];

                // and call its Sample_Li function
                Vector3f DLwi(0.f);
                float DLpdf;
                Color3f Li = light->Sample_Li(isect, sampler->Get2D(), &DLwi, &DLpdf);

                // Should also do shadow test to make correct shadow
                Intersection shadowIsect;
                // Move the intersection point along the normal by some distance
                // to make sure the intersection point is on the outter surface of the
                // geometry
                Ray shadowRay(isect.point + isect.normalGeometric * RayEpsilon, DLwi);

                // Two conditions for shadow:
                // The point in the scene can see the light
                // The light seen is the expected light
                if (scene.Intersect(shadowRay, &shadowIsect) &&
                        light.get() == shadowIsect.objectHit->GetAreaLight()) {

                    // Evaluate the Light Transport Equation with this Li
                    Color3f DLf = isect.bsdf->f(woW, DLwi);
                    float BSDFPdf = isect.bsdf->Pdf(woW, DLwi);

                    // Do the Light Transport integration
                    // Divide the PDF by the number of light sources in scene to
                    // account for choosing a light at random uniformly from the set of all lights in the scene.
                    if (fequal(DLpdf, 0.f) || fequal(BSDFPdf, 0.f)) {  // Avoid dividing by zero
                        DLColor += Color3f(0.f);
                    } else {
                        // Use MIS: Direct Light part weight
                        // Two kinds of weighting Functions
                        //float DLWeight = BalanceHeuristic(1, DLpdf, 1, BSDFPdf);
                        float DLWeight = PowerHeuristic(1, DLpdf, 1, BSDFPdf);
                        // Apply weight to LTE
                        DLColor += DLf * Li * AbsDot(DLwi, isect.normalGeometric) * DLWeight / (DLpdf / lightNum);
                    }
                }

//// *************************************************************************************************************

                // Use MIS: BSDF Sampling part
                // Call samplef and get the returned wiW and pdf
                Vector3f wiB(0.f);
                float Bpdf;
                Color3f Bf = isect.bsdf->Sample_f(woW, &wiB, sampler->Get2D(), &Bpdf);

                // If there is no more light energy for bouncing
                // return current color
                if (fequal(Bpdf, 0.f) || IsBlack(Bf)) {
                    BSDFColor += Color3f(0.f);
                } else {
                    // Make use BSDF sampling technique to generate a ray
                    Ray rayEmit = isect.SpawnRay(glm::normalize(wiB));
                    Intersection Bisect;
                    // If this ray can see the correct light
                    if (scene.Intersect(rayEmit, &Bisect) &&
                            light.get() == Bisect.objectHit->GetAreaLight()) {
                        // Find which light the ray is pointing to
                        const AreaLight *lightPtr = Bisect.objectHit->GetAreaLight();
                        float Dpdf = lightPtr->Pdf_Li(isect, wiB);
                        //float BWeight = BalanceHeuristic(1, Bpdf, 1, Dpdf);
                        float BWeight = PowerHeuristic(1, Bpdf, 1, Dpdf);

                        // Do the Light Transport integration
                        BSDFColor += Bf * Li * AbsDot(wiB, isect.normalGeometric) * BWeight / (Bpdf / lightNum);
                    }
                }
            }
        }
    }
    return DLColor + BSDFColor;
}
