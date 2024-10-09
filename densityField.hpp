#pragma once
#include <vector>
#include <array>
#include <cmath>
#include <thread>

double calcGaussian(double dx, double dy, double dz, double sigma, double amplitude);
double calcGaussianFast(double dx, double dy, double dz, const double& invSig, double amplitude);
void createDensityFunction(const std::vector<std::array<double, 3>>& inputPoints, double inputBoxSize, double*** density, const int& resolution);
double densityToStdev(double*** density, double boxSize, const int resol);