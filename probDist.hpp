#pragma once
#include <vector>
#include <array>
#include <cmath>
#include <iostream>

double wassersteinDist(const std::vector<std::array<double, 3>>& data)
{
    int size = data.size();
    std::vector<double> aCDF(size);
    std::vector<double> bCDF(size);


    aCDF[0] = data[0][1];
    bCDF[0] = data[0][2];
    for (int i = 1; i < size; i++)
    {
        aCDF[i] = aCDF[i - 1] + data[i][1];
        bCDF[i] = bCDF[i - 1] + data[i][2];
    }

    for (int i = 0; i < size; i++)
    {
        aCDF[i] /= aCDF[size - 1];
        bCDF[i] /= bCDF[size - 1];
    }

    double wDist = 0;
    for (int i = 0; i < size - 1; i++)
    {
        double diff = std::abs(bCDF[i] - aCDF[i]);
        double interval = data[i + 1][0] - data[i][0];
        wDist += diff * interval;

    }
    return wDist;
}

double JSDivergence(const std::vector<std::array<double, 3>>& data)
{
    int size = data.size();
    double aSum = 0, bSum = 0;
    for (int i = 0; i < size; i++)
    {
        aSum += data[i][1];
        bSum += data[i][2];
    }

    if (aSum == 0 || bSum == 0)
    {
        std::cerr << "[Error] One of the distributions sums to zero." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::vector<double> normA(size);
    std::vector<double> normB(size);

    for (int i = 0; i < size; i++)
    {
        normA[i] = data[i][1] / aSum;
        normB[i] = data[i][2] / bSum;
    }

    std::vector<double> mData(size);
    for (int i = 0; i < size; i++)
    {
        mData[i] = 0.5 * (normA[i] + normB[i]);
    }

    double aKL = 0;
    for (int i = 0; i < size; i++)
    {
        if (normA[i] > 0)
        {
            aKL += normA[i] * std::log(normA[i] / mData[i]);
        }
    }

    double bKL = 0;
    for (int i = 0; i < size; i++)
    {
        if (normB[i] > 0)
        {
            bKL += normB[i] * std::log(normB[i] / mData[i]);
        }
    }

    return 0.5 * (aKL + bKL);
}