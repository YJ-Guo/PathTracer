#ifndef SPECTRUM_H
#define SPECTRUM_H
// This Spectrum class works to approximately transform the light form wavelength to RGB color
// The implementation is based on the following link.
// https://www.fourmilab.ch/documents/specrend/
#include "globals.h"


class Spectrum {
public:
    Spectrum(float wavelength);

    // Follow the naming convention by John Walker
    // Transform the spectrum wavelength to CIE's xyz value
    void spectrum_to_xyz();

    // Transfrom the CIE's xyz to RGB value
    void xyz_to_rgb();

    // Modify the rgb value from xyz to [0,1]
    void constrain_rgb();

    // Some getter functions
    Color3f getRGB();
    Color3f getXYZ();
    // Return spectrum as (wavelength)
    float getSpectrum();

private:
    // The light in spectrum wavelength(nm)
    float wavelength;
    // The light in CIE standard
    Color3f XYZColor;
    // The light in RGB standard
    Color3f RGBColor;
};

#endif // SPECTRUM_H
