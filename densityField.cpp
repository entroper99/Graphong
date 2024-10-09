#include "densityField.hpp"
#include <vector>
#include <array>
#include <cmath>
#include <thread>

double calcGaussian(double dx, double dy, double dz, double sigma, double amplitude)
{
    return amplitude * std::exp(-(dx * dx + dy * dy + dz * dz) / (2 * sigma * sigma));
}
double calcGaussianFast(double dx, double dy, double dz, const double& invSig, double amplitude)
{
    double distanceSq = dx * dx + dy * dy + dz * dz;
    return amplitude * std::exp(-distanceSq * invSig);
}

void createDensityFunction(const std::vector<std::array<double, 3>>& inputPoints, double inputBoxSize, double*** density, const int& resolution)
{
    //std::printf("RESOL : %d, pointsSize : %d, inputBoxSize : %f\n", RESOLUTION, inputPoints.size(), inputBoxSize);
    const double SIG = 0.68;
    const double SIG2 = SIG * SIG;
    const double INV_SIG = 1.0 / (2.0 * SIG2);

    double gaussAmp = 1.0;
    double del = inputBoxSize / (resolution - 1);
    int totalSize = resolution * resolution * resolution;
    int numThreads = std::thread::hardware_concurrency();
    //std::printf("cpu number : %d\n", numThreads);
    int blockSize = totalSize / numThreads;

    auto calculateDensityPart = [&density, &inputPoints, inputBoxSize, del, INV_SIG, gaussAmp, resolution](int startIdx, int endIdx)
        {
            //for (const auto& point : inputPoints)std::printf("inputPoints : %f,%f,%f\n", point[0],point[1],point[2]);
            for (int i = startIdx; i < endIdx; ++i)
            {
                int xIdx = i / (resolution * resolution);
                int yIdx = (i / resolution) % resolution;
                int zIdx = i % resolution;

                double tgtX = xIdx * del - inputBoxSize / 2.0;
                double tgtY = yIdx * del - inputBoxSize / 2.0;
                double tgtZ = zIdx * del - inputBoxSize / 2.0;


                //if (getStep() != 0) std::printf("(%d,%d,%d) -> (%f,%f,%f)\n", xIdx, yIdx, zIdx, tgtX, tgtY, tgtZ);

                double densityValue = 0;
                for (const auto& point : inputPoints) densityValue += calcGaussianFast(point[0] - tgtX, point[1] - tgtY, point[2] - tgtZ, INV_SIG, gaussAmp);
                density[xIdx][yIdx][zIdx] = densityValue;
                //std::printf("density input : %f\n", densityValue);
            }
        };

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i)
    {
        int startIdx = i * blockSize;
        int endIdx = (i == numThreads - 1) ? totalSize : startIdx + blockSize;
        threads.emplace_back(calculateDensityPart, startIdx, endIdx);
    }
    for (auto& thread : threads)  thread.join();
}

double densityToStdev(double*** density, double boxSize, const int resol)
{
    double*** laplacian = new double** [resol - 2];
    for (int i = 0; i < resol - 2; i++)
    {
        laplacian[i] = new double* [resol - 2];
        for (int j = 0; j < resol - 2; j++)
        {
            laplacian[i][j] = new double[resol - 2];
            for (int k = 0; k < resol - 2; k++) laplacian[i][j][k] = 0.0;
        }
    }
    double del = boxSize / (resol - 1);
    double delSquared = del * del;
    for (int x = 1; x < resol - 1; x++)
    {
        for (int y = 1; y < resol - 1; y++)
        {
            for (int z = 1; z < resol - 1; z++)
            {
                laplacian[x - 1][y - 1][z - 1] = 0;
                laplacian[x - 1][y - 1][z - 1] += (density[x + 1][y][z] - 2 * density[x][y][z] + density[x - 1][y][z]);
                laplacian[x - 1][y - 1][z - 1] += (density[x][y + 1][z] - 2 * density[x][y][z] + density[x][y - 1][z]);
                laplacian[x - 1][y - 1][z - 1] += (density[x][y][z + 1] - 2 * density[x][y][z] + density[x][y][z - 1]);
                laplacian[x - 1][y - 1][z - 1] /= delSquared;
            }
        }
    }

    double variance = 0.0;
    double totalVal = 0.0;
    for (int x = 0; x < resol - 2; x++)
    {
        for (int y = 0; y < resol - 2; y++)
        {
            for (int z = 0; z < resol - 2; z++)
            {
                totalVal += laplacian[x][y][z];
            }
        }
    }
    double mean = totalVal / ((resol - 2) * (resol - 2) * (resol - 2));
    for (int x = 0; x < resol - 2; x++)
    {
        for (int y = 0; y < resol - 2; y++)
        {
            for (int z = 0; z < resol - 2; z++)
            {
                variance += std::pow(laplacian[x][y][z] - mean, 2.0);
            }
        }
    }
    variance /= (double)((resol - 2) * (resol - 2) * (resol - 2));


    for (int i = 0; i < resol - 2; i++)
    {
        for (int j = 0; j < resol - 2; j++)
        {
            delete[] laplacian[i][j];
        }
        delete[] laplacian[i];
    }
    delete[] laplacian;

    return std::sqrt(variance);
}