#define _USE_MATH_DEFINES
#include <math.h>
#include <array>
#include <vector>
#include <thread>
#include <mutex>
#include <cmath>
#include <numeric>
#include <iostream>

export module exAddOn;


export inline double calcGaussian(double dx, double dy, double dz, double sigma, double amplitude)
{
    return amplitude * std::exp(-(dx * dx + dy * dy + dz * dz) / (2 * sigma * sigma));
}
export inline double calcGaussianFast(double dx, double dy, double dz, const double& invSig, double amplitude)
{
    double distanceSq = dx * dx + dy * dy + dz * dz;
    return amplitude * std::exp(-distanceSq * invSig);
}

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

export double createDensityFunction(const std::vector<std::array<double, 3>>& inputPoints, double inputBoxSize, double*** density, const int& resolution)
{
    std::vector<std::array<double, 3>> pbcPoints;

    for (size_t i = 0; i < inputPoints.size(); i++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            for (int dy = -1; dy <= 1; dy++)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    pbcPoints.push_back({ inputPoints[i][0] + dx * inputBoxSize,inputPoints[i][1] + dy * inputBoxSize,inputPoints[i][2] + dz * inputBoxSize });
                }
            }
        }
    }

    //std::printf("RESOL : %d, pointsSize : %d, inputBoxSize : %f\n", RESOLUTION, pbcPoints.size(), inputBoxSize);
    const double SIG = 0.68;
    const double SIG2 = SIG * SIG;
    const double INV_SIG = 1.0 / (2.0 * SIG2);

    double gaussAmp = 1.0;
    double del = inputBoxSize / (resolution - 1);
    int totalSize = resolution * resolution * resolution;
    int numThreads = std::thread::hardware_concurrency();
    //std::printf("cpu number : %d\n", numThreads);
    int blockSize = totalSize / numThreads;

    double cutoff = 3.0 * SIG; //진폭의 99.7프로를 포함하는 값 
    double cutoffSq = cutoff * cutoff;

    auto calculateDensityPart = [&density, &pbcPoints, inputBoxSize, del, SIG, INV_SIG, gaussAmp, cutoffSq, resolution](int startIdx, int endIdx)
        {
            //for (const auto& point : pbcPoints)std::printf("pbcPoints : %f,%f,%f\n", point[0],point[1],point[2]);
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


                for (const auto& point : pbcPoints)
                {
                    double dx = point[0] - tgtX;
                    double dy = point[1] - tgtY;
                    double dz = point[2] - tgtZ;
                    double dist = dx * dx + dy * dy + dz * dz;
                    if (dist < cutoffSq)
                    {
                        densityValue += calcGaussianFast(dx, dy, dz, INV_SIG, gaussAmp);
                    }
                }
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

    double totalVal = 0;
    for (int x = 0; x < resolution; x++)
    {
        for (int y = 0; y < resolution; y++)
        {
            for (int z = 0; z < resolution; z++)
            {
                totalVal += density[x][y][z];
            }
        }
    }

    double meanVal = totalVal / (double)(resolution * resolution * resolution);
    //std::wprintf(L"밀도 함수의 평균값은 %lf이다.\n", totalVal / (double)(resolution * resolution * resolution));
    return meanVal;

}

export double densityToStdev(double*** density, double boxSize, const int resol)
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

export int densityToSurface(double*** density, const int& resolution, const double& cutoff, const double& tolerance)
{
    int pointNumber = 0;
    for (int x = 0; x < resolution; x++)
    {
        for (int y = 0; y < resolution; y++)
        {
            for (int z = 0; z < resolution; z++)
            {
                if (density[x][y][z] > cutoff - tolerance && density[x][y][z] < cutoff + tolerance)
                {
                    pointNumber++;
                }
            }
        }
    }
    return pointNumber;
}

//@brief resolution의 density를 입력하면 resolution-2의 라플라시안을 반환
export double*** createLaplacian(double*** density, int resolution, double boxSize)
{
    double*** laplacian = create3DArray(resolution - 2, resolution - 2, resolution - 2);
    double del = boxSize / (resolution - 1);
    double delSquared = del * del;
    for (int x = 1; x < resolution - 1; x++)
    {
        for (int y = 1; y < resolution - 1; y++)
        {
            for (int z = 1; z < resolution - 1; z++)
            {
                laplacian[x - 1][y - 1][z - 1] = 0;
                laplacian[x - 1][y - 1][z - 1] += (density[x + 1][y][z] - 2 * density[x][y][z] + density[x - 1][y][z]);
                laplacian[x - 1][y - 1][z - 1] += (density[x][y + 1][z] - 2 * density[x][y][z] + density[x][y - 1][z]);
                laplacian[x - 1][y - 1][z - 1] += (density[x][y][z + 1] - 2 * density[x][y][z] + density[x][y][z - 1]);
                laplacian[x - 1][y - 1][z - 1] /= delSquared;
            }
        }
    }
    return laplacian;
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

export std::vector<std::array<double,3>> arrayToHistogram(double*** array, const int& resolution, int histoSize)
{
    std::vector<double> linearData;
    for (int x = 0; x < resolution; x++)
    {
        for (int y = 0; y < resolution; y++)
        {
            for (int z = 0; z < resolution; z++)
            {
                linearData.push_back(array[x][y][z]);
            }
        }
    }
    return createHistogramSize(linearData, histoSize);
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
    return wDist;
}

export double wassersteinDist(const std::vector<std::array<double, 3>>& data1, const std::vector<std::array<double, 3>>& data2)
{
    if (data1.size() != data2.size()) std::cerr << "Size of input data1 and data2 is different in wassersteinDist.\n";
    std::vector<std::array<double, 3>> newData;
    for (int i = 0; i < data1.size(); i++) newData.push_back({ data1[i][0],data1[i][1],data2[i][1] });
    return wassersteinDist(newData);
}


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

export double JSDivergence(const std::vector<std::array<double, 3>>& data1, const std::vector<std::array<double, 3>>& data2)
{
    if(data1.size()!=data2.size()) std::cerr << "Size of input data1 and data2 is different in JSDivergence.\n";
    std::vector<std::array<double, 3>> newData;
    for (int i = 0; i < data1.size(); i++) newData.push_back({ data1[i][0],data1[i][1],data2[i][1] });
    return JSDivergence(newData);
}