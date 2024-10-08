#define _USE_MATH_DEFINES
#include <array>
#include <vector>
#include <thread>
#include <mutex>
#include <cmath>
#include <iostream>

export module stdevCurvature;

export const int RESOLUTION = 128;
export const int HIST_SIZE = 256;

export double*** create3DArray(int x, int y, int z)
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
export double*** copy3DArray(double*** source, int x, int y, int z)
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
export void free3DArray(double*** array, int x, int y)
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
export double loss3DArray(double*** arr1, double*** arr2, int arrSize)
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


export std::vector<std::array<double, 3>> createHistogramDel(const std::vector<double>& inputDataset, double del)
{

    //xy데이터로 히스토그램을 만듭니다
    std::vector<std::array<double, 3>> newPoints;

    double minVal = *std::min_element(inputDataset.begin(), inputDataset.end());
    double maxVal = *std::max_element(inputDataset.begin(), inputDataset.end());
    int numBins = std::ceil((maxVal - minVal) / del); //구간의 숫자

    //std::printf("numBins : %d\n", numBins);

    newPoints.resize(numBins);
    for (int i = 0; i < numBins; i++)
    {
        newPoints[i] = { (double)i,0.0,0.0 };
    }

    for (size_t i = 0; i < inputDataset.size(); i++)
    {
        int binIndex = (int)((inputDataset[i] - minVal) / del);
        if (binIndex == numBins) binIndex--;

        if (binIndex < numBins && binIndex >= 0)
        {
            newPoints[binIndex][1] += 1;
        }
    }

    //std::printf("Histogram data (index, count, placeholder):\n");
    //for (const auto& point : newPoints) std::printf("Bin: %.2f, Count: %.2f, Placeholder: %.2f\n", point[0], point[1], point[2]);

    return newPoints;
}
export std::vector<std::array<double, 3>> createHistogramSize(const std::vector<double>& inputDataset, int inputSize)
{
    double minVal = *std::min_element(inputDataset.begin(), inputDataset.end());
    double maxVal = *std::max_element(inputDataset.begin(), inputDataset.end());
    double del = (maxVal - minVal) / (double)inputSize;
    return createHistogramDel(inputDataset, del);
}
export double calcGaussian(double dx, double dy, double dz, double sigma, double amplitude)
{
    return amplitude * std::exp(-(dx * dx + dy * dy + dz * dz) / (2 * sigma * sigma));
}
export double calcGaussian2(double dx, double dy, double dz, const double& invSig, double amplitude)
{
    double distanceSq = dx * dx + dy * dy + dz * dz;
    return amplitude * std::exp(-distanceSq * invSig);
}
export void createDensityFunction(const std::vector<std::array<double, 3>>& inputPoints, double inputBoxSize, double*** density, const int& resolution)
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
                for (const auto& point : inputPoints) densityValue += calcGaussian2(point[0] - tgtX, point[1] - tgtY, point[2] - tgtZ, INV_SIG, gaussAmp);
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

export double wassersteinDist(const std::vector<std::array<double, 3>>& data)
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


    //exit(-1);
    return wDist;
}

//export double wassersteinDist(const std::vector<std::array<double, 3>>& data)
//{
//    int size = data.size();
//    std::vector<double> aCDF(size);
//    std::vector<double> bCDF(size);
//    aCDF[0] = data[0][1];
//    bCDF[0] = data[0][2];
//    for (int i = 1; i < size; i++)
//    {
//        aCDF[i] = aCDF[i - 1] + data[i][1];
//        bCDF[i] = bCDF[i - 1] + data[i][2];
//    }
//    double wVar = 0;
//    for (int i = 0; i < size - 1; i++)
//    {
//        wVar += std::abs(bCDF[i] - aCDF[i]) * (data[i + 1][0] - data[i][0]);
//    }
//    return wVar;
//}
export double JSDivergence(const std::vector<std::array<double, 3>>& data)
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

export std::vector<std::array<double, 3>> makeGyroidPoints(double boxSize)
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
export std::vector<std::array<double, 3>> makeGyroidLaplacianSet(double boxSize)
{
    double*** density = create3DArray(RESOLUTION, RESOLUTION, RESOLUTION);
    createDensityFunction(makeGyroidPoints(boxSize), boxSize, density, RESOLUTION);

    double*** laplacian = create3DArray(RESOLUTION - 2, RESOLUTION - 2, RESOLUTION - 2);
    double del = boxSize / (RESOLUTION - 1);
    double delSquared = del * del;
    std::vector<double> laplacianSet;
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

                laplacianSet.push_back(laplacian[x - 1][y - 1][z - 1]);
            }
        }
    }

    free3DArray(density, RESOLUTION, RESOLUTION);
    free3DArray(laplacian, RESOLUTION - 2, RESOLUTION - 2);

    return createHistogramSize(laplacianSet, HIST_SIZE);
}
export double densityToStdev(double*** density, double boxSize) //밀도함수를 입력받아 라플라시안을 반환
{
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

    double del = boxSize / (RESOLUTION - 1);
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


    for (int i = 0; i < RESOLUTION - 2; i++)
    {
        for (int j = 0; j < RESOLUTION - 2; j++)
        {
            delete[] laplacian[i][j];
        }
        delete[] laplacian[i];
    }
    delete[] laplacian;

    return std::sqrt(variance);
}
export double densityToHist(double*** density, double boxSize, const std::vector<std::array<double, 3>>& refHist)
{
    double*** laplacian = create3DArray(RESOLUTION - 2, RESOLUTION - 2, RESOLUTION - 2);
    double del = boxSize / (RESOLUTION - 1);
    double delSquared = del * del;
    std::vector<double> laplacianSet;
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
                laplacianSet.push_back(laplacian[x - 1][y - 1][z - 1]);

                //if (getStep() != 0) printf("laplSet : %f\n", laplacianSet[laplacianSet.size() - 1]);
            }
        }
    }

    std::vector<std::array<double, 3>> lapHist = createHistogramSize(laplacianSet, HIST_SIZE);
    std::vector<std::array<double, 3>> finalSet;

    for (int i = 0; i < 256; i++) finalSet.push_back({ lapHist[i][0],lapHist[i][1], refHist[i][1] });


    //std::printf("FinalSet data (lapHist[0], lapHist[1], refHist[1]):\n");
    //for (size_t i = 0; i < finalSet.size(); i++)
    //{
    //    std::printf("Bin: %.2f, LapHistCount: %.2f, RefHistCount: %.2f\n",
    //        finalSet[i][0], finalSet[i][1], finalSet[i][2]);
    //}

    free3DArray(laplacian, RESOLUTION - 2, RESOLUTION - 2);
    double result = wassersteinDist(finalSet);
    return result;
}
