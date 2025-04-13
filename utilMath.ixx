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
#include <omp.h>

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



export double getDebyeStructureFactor(const std::vector<std::array<double, 3>>& pts,double q,double cutoff,double box_edge) 
{
    size_t N = pts.size();
    if (N < 2) return 1.0;

    if (box_edge <= 0.0) 
    {
        std::cerr << "Error: Invalid box_edge for PBC calculation." << std::endl;
        return 1.0;
    }

    double s = 0.0;

    for (size_t i = 0; i < N; ++i) 
    {
        for (size_t j = i + 1; j < N; ++j) 
        {
            double dx = pts[j][0] - pts[i][0];
            double dy = pts[j][1] - pts[i][1];
            double dz = pts[j][2] - pts[i][2];

            dx = dx - box_edge * std::round(dx / box_edge);
            dy = dy - box_edge * std::round(dy / box_edge);
            dz = dz - box_edge * std::round(dz / box_edge);

            double r = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (r < 1e-12) continue;
            if (cutoff > 0.0 && r >= cutoff) continue;

            double w = 1.0;
            if (cutoff > 0.0) 
            {
                double x = M_PI * r / cutoff;
                if (std::fabs(x) > 1e-12) w = std::sin(x) / x;
            }

            double qr = q * r;
            double st = 1.0;
            if (std::fabs(qr) > 1e-12) st = std::sin(qr) / qr;

            s += st * w;
        }
    }

    return 1.0 + 2.0 / static_cast<double>(N) * s;
}

export double getDebyeStructureFactor_OMP(const std::vector<std::array<double, 3>>& pts,double q,double cutoff,double box_edge) 
{
    size_t N = pts.size();
    if (N < 2) return 1.0;

    if (box_edge <= 0.0) 
    {
        std::cerr << "Error: Invalid box_edge for PBC calculation." << std::endl;
        return 1.0;
    }

    double s = 0.0;

#pragma omp parallel for reduction(+:s) schedule(dynamic)
    for (size_t i = 0; i < N; ++i) 
    {
        for (size_t j = i + 1; j < N; ++j) {
            double dx = pts[j][0] - pts[i][0];
            double dy = pts[j][1] - pts[i][1];
            double dz = pts[j][2] - pts[i][2];

            dx = dx - box_edge * std::round(dx / box_edge);
            dy = dy - box_edge * std::round(dy / box_edge);
            dz = dz - box_edge * std::round(dz / box_edge);

            double r = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (r < 1e-12) continue;
            if (cutoff > 0.0 && r >= cutoff) continue;

            double w = 1.0;
            if (cutoff > 0.0) 
            {
                double x = M_PI * r / cutoff;
                if (std::fabs(x) > 1e-12) w = std::sin(x) / x;
            }

            double qr = q * r;
            double st = 1.0;
            if (std::fabs(qr) > 1e-12) st = std::sin(qr) / qr;
            s += st * w;
        }
    }

    return 1.0 + 2.0 / static_cast<double>(N) * s;
}



double calculate_percentile(const std::vector<double>& sorted_data, double p) 
{
    if (sorted_data.empty() || p < 0.0 || p > 100.0) return std::numeric_limits<double>::quiet_NaN();
    if (sorted_data.size() == 1) return sorted_data[0];
    double index = (static_cast<double>(sorted_data.size()) - 1.0) * p / 100.0;
    size_t lower_idx = static_cast<size_t>(std::floor(index));
    size_t upper_idx = static_cast<size_t>(std::ceil(index));
    if (upper_idx >= sorted_data.size()) upper_idx = sorted_data.size() - 1;
    double weight = index - static_cast<double>(lower_idx);
    return sorted_data[lower_idx] * (1.0 - weight) + sorted_data[upper_idx] * weight;
}



export double calculateHellingerSquaredAutoBins(const std::vector<double>& data1,const std::vector<double>& data2) 
{
    if (data1.empty() && data2.empty()) return 0.0;
    if (data1.empty() || data2.empty()) return 1.0;


    std::vector<double> combined_data = data1;
    combined_data.insert(combined_data.end(), data2.begin(), data2.end());
    size_t n = combined_data.size();

    if (n < 2) return 0.0;

    auto minmax = std::minmax_element(combined_data.begin(), combined_data.end());
    double overall_min = *minmax.first;
    double overall_max = *minmax.second;

    if (overall_max <= overall_min) return 0.0;


    std::sort(combined_data.begin(), combined_data.end());

    double q1 = calculate_percentile(combined_data, 25.0);
    double q3 = calculate_percentile(combined_data, 75.0);
    double iqr = q3 - q1;

    int num_bins = 0;
    double bin_width_fd = 0.0;

    if (iqr > std::numeric_limits<double>::epsilon())  bin_width_fd = 2.0 * iqr * std::pow(static_cast<double>(n), -1.0 / 3.0);

    if (bin_width_fd <= std::numeric_limits<double>::epsilon() * (overall_max - overall_min) || bin_width_fd <= 0) num_bins = static_cast<int>(std::ceil(std::log2(static_cast<double>(n)) + 1.0));
    else num_bins = static_cast<int>(std::ceil((overall_max - overall_min) / bin_width_fd));

    const int MIN_BINS = 10;
    if (num_bins <= 0) num_bins = MIN_BINS;
    num_bins = std::max(MIN_BINS, num_bins);

    double bin_width = (overall_max - overall_min) / static_cast<double>(num_bins);
    if (bin_width <= 0) throw std::runtime_error("Error: Final calculated bin width is zero or negative.");

    std::vector<double> counts1(num_bins, 0.0);
    std::vector<double> counts2(num_bins, 0.0);

    for (double val : data1) 
    {
        int bin_index = static_cast<int>((val - overall_min) / bin_width);
        if (bin_index >= num_bins) bin_index = num_bins - 1;
        if (bin_index < 0) bin_index = 0;
        counts1[bin_index]++;
    }
    for (double val : data2) 
    {
        int bin_index = static_cast<int>((val - overall_min) / bin_width);
        if (bin_index >= num_bins) bin_index = num_bins - 1;
        if (bin_index < 0) bin_index = 0;
        counts2[bin_index]++;
    }

    double total_count1 = static_cast<double>(data1.size());
    double total_count2 = static_cast<double>(data2.size());
    double norm_factor1 = total_count1 * bin_width;
    double norm_factor2 = total_count2 * bin_width;
    if (norm_factor1 <= 0) norm_factor1 = 1.0;
    if (norm_factor2 <= 0) norm_factor2 = 1.0;

    std::vector<double> pdf1(num_bins);
    std::vector<double> pdf2(num_bins);
    for (int i = 0; i < num_bins; ++i) 
    {
        pdf1[i] = counts1[i] / norm_factor1;
        pdf2[i] = counts2[i] / norm_factor2;
    }

    double bc = 0.0;
    for (int i = 0; i < num_bins; ++i) bc += std::sqrt(pdf1[i] * pdf2[i]);
    bc *= bin_width;

    double h_squared = 1.0 - bc;
    h_squared = std::max(0.0, std::min(1.0, h_squared));

    return h_squared;
}