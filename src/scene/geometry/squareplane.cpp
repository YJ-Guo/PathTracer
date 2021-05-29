#include "squareplane.h"

float SquarePlane::Area() const
{
    // Only need to get scale on the unit squareplane
    Vector3f scaleVec = transform.getScale();
    return scaleVec.x * scaleVec.y;
}

Intersection SquarePlane::Sample(const Point2f &xi, Float *pdf) const {
    // Generate a world-space point on the surface of the shape
    // Remap the xi from [0,1) to [-0.5, 0.5)
    Vector4f pObj(xi.x - 0.5, xi.y - 0.5, 0.f, 1.f);

    // Create an Intersection to return
    Intersection it;

    // Set the point and normal of this Intersection to the correct values
    it.normalGeometric = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
    it.point = Point3f(transform.T() * pObj);

    // Calculate pdf based on sueface area
    *pdf = fequal(Area(), 0.f) ? 0.f : 1 / Area();
    return it;
}

bool SquarePlane::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    Point3f P = Point3f(t * r_loc.direction + r_loc.origin);
    //Check that P is within the bounds of the square
    if(t > 0 && P.x >= -0.5f && P.x <= 0.5f && P.y >= -0.5f && P.y <= 0.5f)
    {
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

void SquarePlane::Sample_Photon(Photon &inPhoton, std::shared_ptr<Sampler> sampler, float &pdf) const
{
    Point2f xi = sampler->Get2D();
    Point2f xd = sampler->Get2D();
    Vector3f wi = WarpFunctions::squareToSphereCapUniform(xd, 145.f);
    pdf = WarpFunctions::squareToHemisphereCosinePDF(wi);

    glm::vec4 localPoint = glm::vec4(xi[0]-0.5f, xi[1]-0.5f, 0.f, 1.f);
    Point3f worldPoint = glm::vec3(transform.T() * localPoint);
    Vector3f worldDir = glm::normalize(glm::vec3(transform.T() * glm::vec4(wi, 0.f)));
    inPhoton.wi = worldDir;
    inPhoton.pos = worldPoint;
}

void SquarePlane::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    *nor = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
    CoordinateSystem(*nor, tan, bit);
}


Point2f SquarePlane::GetUVCoordinates(const Point3f &point) const
{
    return Point2f(point.x + 0.5f, point.y + 0.5f);
}
