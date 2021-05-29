#include "bsdf.h"
#include <warpfunctions.h>

BSDF::BSDF(const Intersection& isect, float eta /*= 1*/)
// Properly set worldToTangent and tangentToWorld
    : worldToTangent(),
      tangentToWorld(Matrix3x3(isect.tangent, isect.bitangent, isect.normalGeometric)),
      normal(isect.normalGeometric),
      eta(eta),
      numBxDFs(0),
      bxdfs{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
{
    worldToTangent = glm::transpose(tangentToWorld);
}


void BSDF::UpdateTangentSpaceMatrices(const Normal3f& n, const Vector3f& t, const Vector3f b)
{
    // Update worldToTangent and tangentToWorld based on the normal, tangent, and bitangent
    tangentToWorld = Matrix3x3(glm::normalize(t), glm::normalize(b), glm::normalize(n));
    worldToTangent = glm::transpose(tangentToWorld);
}


// Follow the method mentioned in PBRT book Ch.9.1
Color3f BSDF::f(const Vector3f &woW, const Vector3f &wiW, BxDFType flags /*= BSDF_ALL*/) const
{
    // Transfrom the wi and wo into tangent coordinate
    Vector3f wiT = worldToTangent * wiW;
    Vector3f woT = worldToTangent * woW;

    bool reflect = glm::dot(wiW, normal) * glm::dot(woW, normal) > 0;

    Color3f resultColor(0.f);

    for (int i = 0; i < numBxDFs; ++i) {

        if (bxdfs[i]->MatchesFlags(flags) &&
                ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
                 (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION)))) {

            resultColor += bxdfs[i]->f(woT, wiT);
        }
    }

    return resultColor;
}

// Use the input random number _xi_ to select
// one of our BxDFs that matches the _type_ flags.

// After selecting our random BxDF, rewrite the first uniform
// random number contained within _xi_ to another number within
// [0, 1) so that we don't bias the _wi_ sample generated from
// BxDF::Sample_f.

// Convert woW and wiW into tangent space and pass them to
// the chosen BxDF's Sample_f (along with pdf).
// Store the color returned by BxDF::Sample_f and convert
// the _wi_ obtained from this function back into world space.

// Iterate over all BxDFs that we DID NOT select above (so, all
// but the one sampled BxDF) and add their PDFs to the PDF we obtained
// from BxDF::Sample_f, then average them all together.

// Finally, iterate over all BxDFs and sum together the results of their
// f() for the chosen wo and wi, then return that sum.


// Follow the method in PBRT Ch.14.1.6
Color3f BSDF::Sample_f(const Vector3f &woW, Vector3f *wiW, const Point2f &xi,
                       float *pdf, BxDFType type, BxDFType *sampledType) const
{
    // Choose which BxDF to sample
    int matchingComps = BxDFsMatchingFlags(type);
    if (matchingComps == 0) {
        *pdf = 0;
        return Color3f(0.f);
    }

    int comp = std::min((int)std::floor(xi[0] * matchingComps), matchingComps - 1);

    // Get BxDF pointer for chosen component
    BxDF *bxdf = nullptr;
    int count = comp;
    for (int i = 0; i < numBxDFs; ++i) {
        if (bxdfs[i]->MatchesFlags(type) && count-- == 0) {
            bxdf = bxdfs[i];
            break;
        }
    }

    // Remap BxDF sample u to [0,1)^2
    Point2f uRemapped(xi[0] * matchingComps - comp, xi[1]);

    // Sample chosen BxDF
    Vector3f wiT = worldToTangent * woW;
    Vector3f woT = worldToTangent * woW;

    *pdf = 0;

    if (sampledType) {
        *sampledType = bxdf->type;
    }

    Color3f resultColor = bxdf->Sample_f(woT, &wiT, uRemapped, pdf, sampledType);
    if (*pdf == 0) {
        return Color3f(0.f);
    }

    *wiW = tangentToWorld * wiT;

    // Compute overall PDF with all matching BxDFs
    // Skip the SPECULAR portion of BxDF for its special delta distribution
    if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1) {
        for (int i = 0; i < numBxDFs; ++i) {
            if (bxdfs[i] != bxdf && bxdfs[i]->MatchesFlags(type)) {
                *pdf += bxdfs[i]->Pdf(woT, wiT);
            }
        }
    }

    if (matchingComps > 1) {
        *pdf /= matchingComps;
    }

    // Compute value of BSDF for sampled direction
    // Skip the SPECULAR portion of BxDF for its special delta distribution
    if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1) {
        bool reflect = glm::dot(*wiW, normal) * glm::dot(woW, normal) > 0;
        resultColor = Color3f(0.f, 0.f, 0.f);
        for (int i = 0; i < numBxDFs; ++i) {

            if (bxdfs[i]->MatchesFlags(type) &&
                    ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
                     (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION)))) {

                resultColor += bxdfs[i]->f(woT, wiT);
            }
        }
    }

    return resultColor;
}

// Looping over the BxDFs and calling their Pdf() methods to find the PDF for an arbitrary sampled direction.
// Follow the code flow by PBRT-V3
float BSDF::Pdf(const Vector3f &woW, const Vector3f &wiW, BxDFType flags) const
{
    // if there is no BxDF
    if (numBxDFs == 0) {
        return 0.f;
    }

    Vector3f woT = worldToTangent * woW;
    Vector3f wiT = worldToTangent * wiW;

    if (woT.z == 0) {
        return 0.f;
    }

    float pdf = 0;
    int matchingComps = 0;

    // iterate over all matching bxdfs and accumulate their pdf
    for (int i = 0; i < numBxDFs; ++i) {
        if (bxdfs[i]->MatchesFlags(flags)) {
            ++matchingComps;
            pdf += bxdfs[i]->Pdf(woT, wiT);
        }
    }

    // divide by number of matchings for proper brightness
    float resultPdf = matchingComps > 0 ? pdf / matchingComps : 0.f;

    return resultPdf;
}

// Follow the PBRT book Ch. 14.1
Color3f BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi,
                       Float *pdf, BxDFType *sampledType) const
{
    // Samples the unit hemisphere with a cosine-weighted distribution
    *wi = WarpFunctions::squareToHemisphereCosine(xi);

    // flipping the direction if necessary
    if (wo.z < 0) {
        wi->z *= -1;
    }

    *pdf = Pdf(wo, *wi);

    return f(wo, *wi);

//    // Samples the unit hemisphere with a Uniform distribution
//    *wi = WarpFunctions::squareToHemisphereUniform(xi);

//    // flipping the direction if necessary
//    if (wo.z < 0) {
//        wi->z *= -1;
//    }

//    *pdf = Pdf(wo, *wi);

//    return f(wo, *wi);
}


float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    // The PDF for Cosine weighted hemisphere sampling
    // pdf = cos(theta) / PI
    // should check if wi and wo are on same side
    return SameHemisphere(wo, wi) ? AbsCosTheta(wi) * InvPi : 0;

    // The PDF for uniform hemisphere sampling
//    return SameHemisphere(wo, wi) ? Inv2Pi : 0;
}

BSDF::~BSDF()
{
    for(int i = 0; i < numBxDFs; i++)
    {
        delete bxdfs[i];
    }
}
