#include "naiveintegrator.h"

Color3f NaiveIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Color3f resColor(0.f);
    Vector3f woW = -ray.direction;
    Intersection isect;
    // If the ray hits an object in the scene
    if (scene.Intersect(ray, &isect)) {
        // Add self-emitted light
        resColor += isect.Le(woW);
        // If not reach recursion limit and
        // the hitted object has material for ProduceBSDF
        if (depth > 0 && isect.objectHit->material != nullptr) {
            // ProduceBSDF with given material
            isect.ProduceBSDF();

            // Call samplef and get the returned wiW and pdf
            Vector3f wiW(0.f);
            float pdf;
            Color3f f = isect.bsdf->Sample_f(woW, &wiW, sampler->Get2D(), &pdf);

            // Make rayEmit the Li for other point
            Ray rayEmit = isect.SpawnRay(glm::normalize(wiW));
            Color3f newResColor = Li(rayEmit, scene, sampler, --depth);

            // If there is no more light energy for bouncing
            // return current color
            if (fequal(pdf, 0.f)) {
                return resColor;
            }
            else {
                // Do the Light Transport integration
                resColor += f * newResColor * AbsDot(wiW, isect.normalGeometric) / pdf;
            }
        }
    }
    return resColor;
}
