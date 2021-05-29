#include "shape.h"
#include <QDateTime>

pcg32 Shape::colorRNG = pcg32(QDateTime::currentMSecsSinceEpoch());


void Shape::InitializeIntersection(Intersection *isect, float t, Point3f pLocal) const
{
    isect->point = Point3f(transform.T() * glm::vec4(pLocal, 1));
    ComputeTBN(pLocal, &(isect->normalGeometric), &(isect->tangent), &(isect->bitangent));
    isect->uv = GetUVCoordinates(pLocal);
    isect->t = t;
}

Intersection Shape::Sample(const Intersection &ref, const Point2f &xi, float *pdf) const
{
    // Invoke the two-input Sample to get the Intersection
    Intersection isect = this->Sample(xi, pdf);

    // Generate wi from intersection above
    Vector3f wi = glm::normalize(isect.point - ref.point);

    // Convert the PDF obtained from the other Sample function
    // from a PDF with respect to surface area to a PDF with
    // respect to solid angle at the reference point of intersection.
    // Use the area method mentioned in Q&A
    float surfArea = AbsDot(isect.normalGeometric, -wi) * Area();
    // If divide by zero, set PDF to zero
    *pdf = fequal(surfArea, 0.f) ? 0.f : glm::length2(ref.point - isect.point) / surfArea;

    return isect;
}

float Shape::Pdf(const Intersection &ref, const Vector3f &wi) const{
    return 0.f;
}
