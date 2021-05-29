#include "disc.h"
#include "warpfunctions.h"

float Disc::Area() const
{
    // Only need to get scale on the unit disc
    Vector3f scaleVec = transform.getScale();
    return scaleVec.x * scaleVec.y * Pi;
}

// Refer to PBRT Ch.14.2.2 for Shape sampling
Intersection Disc::Sample(const Point2f &xi, Float *pdf) const {
    // Generate a world-space point on the surface of the shape
    // Use homogenerous coordinates for spatial transformation
    Point3f pd = WarpFunctions::squareToDiskConcentric(xi);
    Vector4f pObj(pd.x, pd.y, 0.f, 1.f);

    // Create an Intersection to return
    Intersection it;

    // Set the point and normal of this Intersection to the correct values
    it.normalGeometric = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
    it.point = Point3f(transform.T() * pObj);

    // Calculate pdf based on sueface area
    *pdf = fequal(Area(), 0.f) ? 0.f : 1 / Area();
    return it;
}

bool Disc::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    Point3f P = Point3f(t * r_loc.direction + r_loc.origin);
    //Check that P is within the bounds of the disc (not bothering to take the sqrt of the dist b/c we know the radius)
    float dist2 = (P.x * P.x + P.y * P.y);
    if(t > 0 && dist2 <= 1.f)
    {
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

void Disc::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    *nor = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
    CoordinateSystem(*nor, tan, bit);
}


Point2f Disc::GetUVCoordinates(const Point3f &point) const
{
    return glm::vec2((point.x + 1)/2.f, (point.y + 1)/2.f);
}

