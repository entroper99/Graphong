#include <array>
#include <vector>
#include <thread>
#include <mutex>
#include <cmath>

export module stdevCurvature;

const int RESOLUTION = 128;
double calcGaussian(double dx, double dy, double dz, double sigma, double amplitude)
{
    return amplitude * std::exp(-(dx * dx + dy * dy + dz * dz) / (2 * sigma * sigma));
}

export void createDensityFunction(const std::vector<std::array<double, 3>>& inputPoints, double inputBoxSize, double*** density)
{
    double gaussAmp = 1.0;
    double gaussSig = 0.68;
    double del = inputBoxSize / (RESOLUTION - 1);
    int totalSize = RESOLUTION * RESOLUTION * RESOLUTION;
    int numThreads = std::thread::hardware_concurrency();
    int blockSize = totalSize / numThreads;

    auto calculateDensityPart = [&density, &inputPoints, inputBoxSize, del, gaussSig, gaussAmp](int startIdx, int endIdx)
        {
            for (int i = startIdx; i < endIdx; ++i)
            {
                int xIdx = i / (RESOLUTION * RESOLUTION);
                int yIdx = (i / RESOLUTION) % RESOLUTION;
                int zIdx = i % RESOLUTION;

                double tgtX = xIdx * del - inputBoxSize / 2.0;
                double tgtY = yIdx * del - inputBoxSize / 2.0;
                double tgtZ = zIdx * del - inputBoxSize / 2.0;

                double densityValue = 0;
                for (const auto& point : inputPoints) densityValue += calcGaussian(point[0] - tgtX, point[1] - tgtY, point[2] - tgtZ, gaussSig, gaussAmp);
                density[xIdx][yIdx][zIdx] = densityValue;
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

export double calcStdevCurvature(const std::vector<std::array<double, 3>>& inputPoints, double inputBoxSize)
{
    double*** density = new double** [RESOLUTION];
    for (int i = 0; i < RESOLUTION; i++)
    {
        density[i] = new double* [RESOLUTION];
        for (int j = 0; j < RESOLUTION; j++)
        {
            density[i][j] = new double[RESOLUTION];
            for (int k = 0; k < RESOLUTION; k++) density[i][j][k] = 0.0;
        }
    }
    createDensityFunction(inputPoints, inputBoxSize, density);

    double*** laplacian = new double** [RESOLUTION - 2];
    for (int i = 0; i < RESOLUTION-2; i++)
    {
        laplacian[i] = new double* [RESOLUTION - 2];
        for (int j = 0; j < RESOLUTION - 2; j++)
        {
            laplacian[i][j] = new double[RESOLUTION - 2];
            for (int k = 0; k < RESOLUTION - 2; k++) laplacian[i][j][k] = 0.0;
        }
    }
    double del = inputBoxSize / (RESOLUTION - 1);
    double delSquared = del * del;
    for (int x = 1; x < RESOLUTION - 1; x++)
    {
        for (int y = 1; y < RESOLUTION - 1; y++)
        {
            for (int z = 1; z < RESOLUTION - 1; z++)
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
    for (int x = 0; x < RESOLUTION - 2; x++)
    {
        for (int y = 0; y < RESOLUTION - 2; y++)
        {
            for (int z = 0; z < RESOLUTION - 2; z++)
            {
                totalVal += laplacian[x][y][z];
            }
        }
    }
    double mean = totalVal / ((RESOLUTION - 2) * (RESOLUTION - 2) * (RESOLUTION - 2));
    for (int x = 0; x < RESOLUTION - 2; x++)
    {
        for (int y = 0; y < RESOLUTION - 2; y++)
        {
            for (int z = 0; z < RESOLUTION - 2; z++)
            {
                variance += std::pow(laplacian[x][y][z] - mean, 2.0);
            }
        }
    }
    variance /= (double)((RESOLUTION - 2) * (RESOLUTION - 2) * (RESOLUTION - 2));
    double stdev = std::sqrt(variance);

    for (int i = 0; i < RESOLUTION; i++)
    {
        for (int j = 0; j < RESOLUTION; j++)
        {
            delete[] density[i][j];
        }
        delete[] density[i];
    }
    delete[] density;

    for (int i = 0; i < RESOLUTION - 2; i++)
    {
        for (int j = 0; j < RESOLUTION - 2; j++)
        {
            delete[] laplacian[i][j];
        }
        delete[] laplacian[i];
    }
    delete[] laplacian;

    return stdev;
}


export std::vector<std::array<double,3>> createHistogramDel(const std::vector<double>& inputDataset, double del)
{
    //xy데이터로 히스토그램을 만듭니다
    std::vector<std::array<double, 3>> newPoints;

    double minVal = *std::min_element(inputDataset.begin(), inputDataset.end());
    double maxVal = *std::max_element(inputDataset.begin(), inputDataset.end());
    int numBins = std::ceil((maxVal - minVal) / del); //구간의 숫자

    newPoints.resize(numBins);
    for (int i = 0; i < numBins; i++) newPoints[i] = { minVal + i * del,0.0,0.0 };

    for (int i = 0; i < inputDataset.size(); i++)
    {
        int binIndex = (int)((inputDataset[i] - minVal) / del);
        if (binIndex == numBins) binIndex--;

        if (binIndex < numBins && binIndex >= 0)
        {
            newPoints[binIndex][1] += 1;
        }
    }
    return newPoints;
}

export std::vector<std::array<double, 3>> createHistogramSize(const std::vector<double>& inputDataset, int inputSize)
{
    double minVal = *std::min_element(inputDataset.begin(), inputDataset.end());
    double maxVal = *std::max_element(inputDataset.begin(), inputDataset.end());
    double del = (maxVal - minVal) / (double)inputSize;
    return createHistogramDel(inputDataset, del);
}

export std::vector<std::array<double, 3>> calcLaplacianHistogram(const std::vector<std::array<double, 3>>& inputPoints, double inputBoxSize)
{
    double*** density = new double** [RESOLUTION];
    for (int i = 0; i < RESOLUTION; i++)
    {
        density[i] = new double* [RESOLUTION];
        for (int j = 0; j < RESOLUTION; j++)
        {
            density[i][j] = new double[RESOLUTION];
            for (int k = 0; k < RESOLUTION; k++) density[i][j][k] = 0.0;
        }
    }
    createDensityFunction(inputPoints, inputBoxSize, density);

    double*** laplacian = new double** [RESOLUTION - 2];
    for (int i = 0; i < RESOLUTION - 2; i++)
    {
        laplacian[i] = new double* [RESOLUTION - 2];
        for (int j = 0; j < RESOLUTION - 2; j++)
        {
            laplacian[i][j] = new double[RESOLUTION - 2];
            for (int k = 0; k < RESOLUTION - 2; k++) laplacian[i][j][k] = 0.0;
        }
    }
    std::vector<double> laplacianDataset;
    double del = inputBoxSize / (RESOLUTION - 1);
    double delSquared = del * del;
    for (int x = 1; x < RESOLUTION - 1; x++)
    {
        for (int y = 1; y < RESOLUTION - 1; y++)
        {
            for (int z = 1; z < RESOLUTION - 1; z++)
            {
                laplacian[x - 1][y - 1][z - 1] = 0;
                laplacian[x - 1][y - 1][z - 1] += (density[x + 1][y][z] - 2 * density[x][y][z] + density[x - 1][y][z]);
                laplacian[x - 1][y - 1][z - 1] += (density[x][y + 1][z] - 2 * density[x][y][z] + density[x][y - 1][z]);
                laplacian[x - 1][y - 1][z - 1] += (density[x][y][z + 1] - 2 * density[x][y][z] + density[x][y][z - 1]);
                laplacian[x - 1][y - 1][z - 1] /= delSquared;

                laplacianDataset.push_back(laplacian[x - 1][y - 1][z - 1]);
            }
        }
    }


    for (int i = 0; i < RESOLUTION; i++)
    {
        for (int j = 0; j < RESOLUTION; j++)
        {
            delete[] density[i][j];
        }
        delete[] density[i];
    }
    delete[] density;

    for (int i = 0; i < RESOLUTION - 2; i++)
    {
        for (int j = 0; j < RESOLUTION - 2; j++)
        {
            delete[] laplacian[i][j];
        }
        delete[] laplacian[i];
    }
    delete[] laplacian;

    return createHistogramDel(laplacianDataset, 0.2);
}