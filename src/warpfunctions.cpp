#define _USE_MATH_DEFINES
#include "warpfunctions.h"
#include <math.h>

Point3f WarpFunctions::squareToDiskUniform(const Point2f &sample)
{
    // a "polar" mapping where one square axis maps to a disc radius
    // and the other axis maps to an angle on the disc
    float theta = 2.f * Pi * sample.y;  // convert y to radian
    float r = sqrt(sample.x);             // convert x to radius

    return glm::vec3(r * glm::cos(theta), r * glm::sin(theta), 0.f);
}

Point3f WarpFunctions::squareToDiskConcentric(const Point2f &sample)
{
    // implement from Peter Shirley's warping method
    // https://doi.org/10.1080/10867651.1997.10487479

    float phi, r, u, v;
    float a = 2.f * sample.x - 1;   // (a,b) is now on [-1,-1]^2
    float b = 2.f * sample.y - 1;

    if (a > -b) {                   // region 1 or 2
        if (a > b) {                // region 1, also |a| > |b|
            r = a;
            phi = (Pi / 4.f) * (b / a);
        } else {                    // region 2, also |b| > |a|
            r = b;
            phi = (Pi / 4.f) * (2 - (a / b));
        }
    } else {                        // region 3 or 4
        if (a < b) {                // region 3, also |a| >= |b|, a != 0
            r = -a;
            phi = (Pi / 4.f) * (4 + (b / a));
        } else {                    // region 4, |b| >= |a|, but a == 0, b == 0 could occur
            r = -b;
            if (!fequal(b, 0.f)) {
                phi = (Pi / 4.f) * (6 - (a / b));
            } else {
                phi = 0;
            }
        }
    }

    u = r * glm::cos(phi);
    v = r * glm::sin(phi);

    return glm::vec3(u, v, 0.f);
}

float WarpFunctions::squareToDiskPDF(const Point3f &sample)
{
    return InvPi;
}

Point3f WarpFunctions::squareToSphereUniform(const Point2f &sample)
{
    // refer to PPT of sampling on P.22
     float z = 1.f - 2.f * sample.x;
     float x = glm::cos(2.f * Pi * sample.y) * sqrt(1.f - z * z);
     float y = glm::sin(2.f * Pi * sample.y) * sqrt(1.f - z * z);

     return glm::vec3(x, y, z);
}

float WarpFunctions::squareToSphereUniformPDF(const Point3f &sample)
{
    return Inv4Pi;
}

Point3f WarpFunctions::squareToSphereCapUniform(const Point2f &sample, float thetaMin)
{
    // the input thetaMin is in degree
    // throw std::runtime_error("You haven't yet implemented sphere cap warping!");
    // consider sphere cap as part of hemisphere
    // use thetaMin vs 180 from hemisphere to shrink

    float theta = glm::acos(sample.x) * (180.f - thetaMin) / 180.f;
    float phi = 2.f * Pi * sample.y;

    return glm::vec3(glm::sin(theta) * glm::cos(phi),
                     glm::sin(theta) * glm::sin(phi),
                     glm::cos(theta));
}

float WarpFunctions::squareToSphereCapUniformPDF(const Point3f &sample, float thetaMin)
{

    return Inv2Pi * (1.f / ( 1.f - glm::cos(glm::radians(180.f - thetaMin))));
}

Point3f WarpFunctions::squareToHemisphereUniform(const Point2f &sample)
{
    float z = sample.x;
    float x = glm::cos(2.f * Pi * sample.y) * sqrt(1 - z * z);
    float y = glm::sin(2.f * Pi * sample.y) * sqrt(1 - z * z);

    return glm::vec3(x, y, z);
}

float WarpFunctions::squareToHemisphereUniformPDF(const Point3f &sample)
{
    return Inv2Pi;
}

Point3f WarpFunctions::squareToHemisphereCosine(const Point2f &sample)
{
    glm::vec3 diskVec = squareToDiskConcentric(sample);
    return glm::vec3(diskVec.x, diskVec.y, sqrt(1 - diskVec.x * diskVec.x - diskVec.y * diskVec.y));
}

float WarpFunctions::squareToHemisphereCosinePDF(const Point3f &sample)
{
    return sample.z * InvPi;
}
