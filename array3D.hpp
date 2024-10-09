#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <array>

double*** create3DArray(int x, int y, int z)
{
    double*** array3D = new double** [x];
    for (int i = 0; i < x; i++)
    {
        array3D[i] = new double* [y];
        for (int j = 0; j < y; j++)
        {
            array3D[i][j] = new double[z];
            for (int k = 0; k < z; k++) array3D[i][j][k] = 0.0;
        }
    }
    return array3D;
}

double*** copy3DArray(double*** source, int x, int y, int z)
{
    double*** copy = new double** [x];
    for (int i = 0; i < x; i++)
    {
        copy[i] = new double* [y];
        for (int j = 0; j < y; j++)
        {
            copy[i][j] = new double[z];
            for (int k = 0; k < z; k++) copy[i][j][k] = source[i][j][k];
        }
    }
    return copy;
}

void free3DArray(double*** array, int x, int y)
{
    for (int i = 0; i < x; i++)
    {
        for (int j = 0; j < y; j++)
        {
            delete[] array[i][j];
        }
        delete[] array[i];
    }
    delete[] array;
}

double loss3DArray(double*** arr1, double*** arr2, int arrSize)
{
    double totalLoss = 0;
    for (int x = 0; x < arrSize; x++)
    {
        for (int y = 0; y < arrSize; y++)
        {
            for (int z = 0; z < arrSize; z++)
            {
                totalLoss += fabs(arr1[x][y][z] - arr2[x][y][z]) * sqrt(x * x + y * y + z * z);
            }
        }
    }
    return totalLoss / (arrSize * arrSize * arrSize);
}

std::vector<std::array<double, 3>> makeGyroidPoints(double boxSize)
{
    std::vector<std::array<double, 3>> gyroidPoints;

    double length = boxSize;// / 2.0;
    double scaleFactor = 2.0 * M_PI / length;
    auto gyroidFunc = [=](double x, double y, double z)->double
        {
            return (std::cos(scaleFactor * x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor * z)) + std::cos(scaleFactor * y) * std::sin(scaleFactor * z) * std::sin(2 * (scaleFactor * x)) + std::cos(scaleFactor * z) * std::sin(scaleFactor * x) * std::sin(2 * (scaleFactor * y)));
        };

    const double del = 0.2;//0.78
    for (double x = -boxSize / 2.0; x <= boxSize / 2.0; x += del)
    {
        for (double y = -boxSize / 2.0; y <= boxSize / 2.0; y += del)
        {
            for (double z = -boxSize / 2.0; z <= boxSize / 2.0; z += del)
            {
                if (gyroidFunc(x, y, z) >= 0.8) gyroidPoints.push_back({ x, y, z });
            }
        }
    }
    return gyroidPoints;
}