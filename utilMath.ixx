#define _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS
#include <SDL.h>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <random>
#include <complex>
#include <Eigen/Dense>
#include <Eigen/Core>
#include <unsupported/Eigen/CXX11/Tensor>
#include <fftw3.h>

export module utilMath;

import std;
import constVar;

export void printRotationMatrix(Eigen::Matrix3d inputMat)
{
    Eigen::Matrix3d checkMat = inputMat;
    double sy = std::sqrt(checkMat(0, 0) * checkMat(0, 0) + checkMat(1, 0) * checkMat(1, 0));
    bool isSingular = sy < 1e-6;
    double x, y, z;
    if (!isSingular)
    {
        x = std::atan2(checkMat(2, 1), checkMat(2, 2));
        y = std::atan2(-checkMat(2, 0), sy);
        z = std::atan2(checkMat(1, 0), checkMat(0, 0));
    }
    else
    {
        x = std::atan2(-checkMat(1, 2), checkMat(1, 1));
        y = std::atan2(-checkMat(2, 0), sy);
        z = 0;
    }

    x *= 180.0 / M_PI;
    y *= 180.0 / M_PI;
    z *= 180.0 / M_PI;

    std::cout << "x축 회전: " << x << "도" << std::endl;
    std::cout << "y축 회전: " << y << "도" << std::endl;
    std::cout << "z축 회전: " << z << "도" << std::endl;
}


export Eigen::Matrix3d angleToMatrix(double xAngle, double yAngle, double zAngle)
{
    double xRad = xAngle * DEGREE_TO_RADIAN; //x축 회전
    double yRad = yAngle * DEGREE_TO_RADIAN; //y축 회전
    double zRad = zAngle * DEGREE_TO_RADIAN; //z축 회전

    Eigen::Matrix3d rotX, rotY, rotZ;
    rotX << 1, 0, 0,
        0, cos(xRad), -sin(xRad),
        0, sin(xRad), cos(xRad);

    rotY << cos(yRad), 0, sin(yRad),
        0, 1, 0,
        -sin(yRad), 0, cos(yRad);

    rotZ << cos(zRad), -sin(zRad), 0,
        sin(zRad), cos(zRad), 0,
        0, 0, 1;

    return rotZ * rotY * rotX;
}


export double computeShellAverageAmplitude(const std::vector<std::array<double, 3>>& points,double boxX, double boxY, double boxZ,double kMag,int numSamples = 1000,bool   returnSquaredAmplitude = false)
{
    std::mt19937 rng(12345);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);

    double accumulatedValue = 0.0;

    const size_t N = points.size();

    for (int i = 0; i < numSamples; i++)
    {
        double u = uniform(rng);
        double v = uniform(rng);
        double theta = std::acos(1.0 - 2.0 * u);
        double phi = 2.0 * M_PI * v;

        double kx_unit = std::sin(theta) * std::cos(phi);
        double ky_unit = std::sin(theta) * std::sin(phi);
        double kz_unit = std::cos(theta);

        double kx = kMag * kx_unit;
        double ky = kMag * ky_unit;
        double kz = kMag * kz_unit;

        std::complex<double> sumF(0.0, 0.0);

        for (const auto& r : points)
        {
            double rx = r[0];
            double ry = r[1];
            double rz = r[2];

            double dot = kx * rx + ky * ry + kz * rz;

            double realPart = std::cos(dot);
            double imagPart = -std::sin(dot);

            sumF += std::complex<double>(realPart, imagPart);
        }

        double amp = std::abs(sumF);
        double val = returnSquaredAmplitude ? (amp * amp) : amp;

        accumulatedValue += val;
    }

    double meanVal = accumulatedValue / static_cast<double>(numSamples);

    return meanVal;
}


export double computeDirectionalAmplitude(
    const std::vector<std::array<double, 3>>& points,
    double kx,
    double ky,
    double kz,
    bool returnSquaredAmplitude = false
)
{
    std::complex<double> sumF(0.0, 0.0);

    for (const auto& r : points)
    {
        double rx = r[0];
        double ry = r[1];
        double rz = r[2];

        double dot = kx * rx + ky * ry + kz * rz;

        // e^(-i dot) = cos(dot) - i*sin(dot)
        double realPart = std::cos(dot);
        double imagPart = -std::sin(dot);

        sumF += std::complex<double>(realPart, imagPart);
    }

    double amp = std::abs(sumF);

    return returnSquaredAmplitude ? (amp * amp) : amp;
}


export double getDebyeStructureFactor(const std::vector<std::array<double, 3>>& pts, double q, double cutoff) 
{
    size_t N = pts.size(); if (N < 2)return 1.0; double s = 0.0;
    for (size_t i = 0; i < N; ++i) 
    {
        for (size_t j = i + 1; j < N; ++j) 
        {
            double dx = pts[j][0] - pts[i][0], dy = pts[j][1] - pts[i][1], dz = pts[j][2] - pts[i][2];
            double r = std::sqrt(dx * dx + dy * dy + dz * dz); if (r < 1e-12)continue; if (cutoff > 0.0 && r >= cutoff)continue;
            double w = 1.0; if (cutoff > 0.0) { double x = M_PI * r / cutoff; if (std::fabs(x) > 1e-12)w = std::sin(x) / x; }
            double qr = q * r; double st = 1.0; if (std::fabs(qr) > 1e-12)st = std::sin(qr) / qr; s += st * w;
        }
    }
    return 1.0 + 2.0 / (N * 1.0 * N * 1.0) * s;
}