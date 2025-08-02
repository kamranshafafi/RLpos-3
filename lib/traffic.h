#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <vector>
// Define a structure to represent a sphere
struct Sphere
{
    double x, y, z; // Center coordinates
    double dmax;    // Sphere's dmax
    double SNR;
};

// Function to calculate dmax from SNR, PT, f, and PN
double CalculateDmax(double SNR, double PT, double f, double PN);
// Function to check if a coordinate is inside the intersection of spheres
bool IsCoordinateInIntersection(const std::vector<Sphere> &spheres, double x, double y, double z);

double CalculateMinPT(std::vector<Sphere> &spheres, double initPT, double frequency, double PN,
                      double x, double y, double z);

#endif // TRAFFIC_H