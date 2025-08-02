#include <cmath>

#include "traffic.h"

// Function to calculate dmax from SNR, PT, f, and PN
double CalculateDmax(double SNR, double PT, double f, double PN)
{
    double dmax = std::pow(10, (PT - SNR - 20 * std::log10(f) - 20 * std::log10(4 * M_PI / 299792458.0) - PN) / 20);
    return dmax;
}

// Function to check if a coordinate is inside the intersection of spheres
bool IsCoordinateInIntersection(const std::vector<Sphere> &spheres, double x, double y, double z)
{
    // Iterate through spheres and check if the coordinate is inside each sphere
    for (const Sphere &sphere : spheres)
    {
        double distance = std::sqrt(std::pow(x - sphere.x, 2) + std::pow(y - sphere.y, 2) + std::pow(z - sphere.z, 2));
        if (distance > sphere.dmax)
        {
            return false; // Outside this sphere
        }
    }
    return true; // Inside all spheres
}

double CalculateMinPT(std::vector<Sphere> &spheres, double initPT, double frequency, double PN,
                      double x, double y, double z)
{
    double PT = initPT;

    for (;;)
    {
        for (auto &s : spheres)
        {
            // Calculate dmax for this sphere
            s.dmax = CalculateDmax(s.SNR, PT, frequency, PN);
        }

        if (IsCoordinateInIntersection(spheres, x, y, z))
        {
            break;
        }
        ++PT;
    }

    return PT;
}
