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
    double gaussAmp = 0.4;
    double gaussSig = 1;
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

    return createHistogramSize(laplacianDataset, 512);
}

//array data -> 0:x값, 1:분포A, 2:분포B
double wassersteinDist(const std::vector<std::array<double,3>>& data)
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

    double wVar = 0;
    for (int i = 0; i < size - 1; i++)
    {
        wVar += std::abs(bCDF[i] - aCDF[i]) * (data[i + 1][0] - data[i][0]);
    }
    return wVar;
}



export double calcLaplacianWasserstein(const std::vector<std::array<double, 3>>& inputPoints, double inputBoxSize)
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

    std::vector<std::array<double, 3>> target = createHistogramSize(laplacianDataset, 200);
    
    std::vector<std::array<double, 3>> ref;
    ref.push_back({ -14.378373,3.000000,0 });
    ref.push_back({ -14.279998,7.000000,0 });
    ref.push_back({ -14.181624,9.000000,0 });
    ref.push_back({ -14.083250,7.000000,0 });
    ref.push_back({ -13.984875,9.000000,0 });
    ref.push_back({ -13.886501,13.000000,0 });
    ref.push_back({ -13.788127,8.000000,0 });
    ref.push_back({ -13.689752,21.000000,0 });
    ref.push_back({ -13.591378,8.000000,0 });
    ref.push_back({ -13.493004,14.000000,0 });
    ref.push_back({ -13.394630,17.000000,0 });
    ref.push_back({ -13.296255,13.000000,0 });
    ref.push_back({ -13.197881,27.000000,0 });
    ref.push_back({ -13.099507,30.000000,0 });
    ref.push_back({ -13.001132,39.000000,0 });
    ref.push_back({ -12.902758,44.000000,0 });
    ref.push_back({ -12.804384,68.000000,0 });
    ref.push_back({ -12.706009,55.000000,0 });
    ref.push_back({ -12.607635,77.000000,0 });
    ref.push_back({ -12.509261,92.000000,0 });
    ref.push_back({ -12.410886,89.000000,0 });
    ref.push_back({ -12.312512,123.000000,0 });
    ref.push_back({ -12.214138,105.000000,0 });
    ref.push_back({ -12.115764,113.000000,0 });
    ref.push_back({ -12.017389,158.000000,0 });
    ref.push_back({ -11.919015,180.000000,0 });
    ref.push_back({ -11.820641,181.000000,0 });
    ref.push_back({ -11.722266,220.000000,0 });
    ref.push_back({ -11.623892,228.000000,0 });
    ref.push_back({ -11.525518,282.000000,0 });
    ref.push_back({ -11.427143,303.000000,0 });
    ref.push_back({ -11.328769,324.000000,0 });
    ref.push_back({ -11.230395,350.000000,0 });
    ref.push_back({ -11.132021,363.000000,0 });
    ref.push_back({ -11.033646,445.000000,0 });
    ref.push_back({ -10.935272,471.000000,0 });
    ref.push_back({ -10.836898,482.000000,0 });
    ref.push_back({ -10.738523,571.000000,0 });
    ref.push_back({ -10.640149,588.000000,0 });
    ref.push_back({ -10.541775,663.000000,0 });
    ref.push_back({ -10.443400,711.000000,0 });
    ref.push_back({ -10.345026,729.000000,0 });
    ref.push_back({ -10.246652,777.000000,0 });
    ref.push_back({ -10.148277,946.000000,0 });
    ref.push_back({ -10.049903,1027.000000,0 });
    ref.push_back({ -9.951529,1111.000000,0 });
    ref.push_back({ -9.853155,1214.000000,0 });
    ref.push_back({ -9.754780,1317.000000,0 });
    ref.push_back({ -9.656406,1420.000000,0 });
    ref.push_back({ -9.558032,1544.000000,0 });
    ref.push_back({ -9.459657,1751.000000,0 });
    ref.push_back({ -9.361283,1816.000000,0 });
    ref.push_back({ -9.262909,2004.000000,0 });
    ref.push_back({ -9.164534,2080.000000,0 });
    ref.push_back({ -9.066160,2221.000000,0 });
    ref.push_back({ -8.967786,2468.000000,0 });
    ref.push_back({ -8.869412,2591.000000,0 });
    ref.push_back({ -8.771037,2719.000000,0 });
    ref.push_back({ -8.672663,2827.000000,0 });
    ref.push_back({ -8.574289,3059.000000,0 });
    ref.push_back({ -8.475914,3176.000000,0 });
    ref.push_back({ -8.377540,3349.000000,0 });
    ref.push_back({ -8.279166,3575.000000,0 });
    ref.push_back({ -8.180791,3723.000000,0 });
    ref.push_back({ -8.082417,3876.000000,0 });
    ref.push_back({ -7.984043,4118.000000,0 });
    ref.push_back({ -7.885668,4197.000000,0 });
    ref.push_back({ -7.787294,4558.000000,0 });
    ref.push_back({ -7.688920,4661.000000,0 });
    ref.push_back({ -7.590546,4927.000000,0 });
    ref.push_back({ -7.492171,5030.000000,0 });
    ref.push_back({ -7.393797,5212.000000,0 });
    ref.push_back({ -7.295423,5548.000000,0 });
    ref.push_back({ -7.197048,5649.000000,0 });
    ref.push_back({ -7.098674,5755.000000,0 });
    ref.push_back({ -7.000300,5885.000000,0 });
    ref.push_back({ -6.901925,6201.000000,0 });
    ref.push_back({ -6.803551,6339.000000,0 });
    ref.push_back({ -6.705177,6399.000000,0 });
    ref.push_back({ -6.606803,6723.000000,0 });
    ref.push_back({ -6.508428,6821.000000,0 });
    ref.push_back({ -6.410054,7157.000000,0 });
    ref.push_back({ -6.311680,7204.000000,0 });
    ref.push_back({ -6.213305,7383.000000,0 });
    ref.push_back({ -6.114931,7568.000000,0 });
    ref.push_back({ -6.016557,7645.000000,0 });
    ref.push_back({ -5.918182,7906.000000,0 });
    ref.push_back({ -5.819808,8134.000000,0 });
    ref.push_back({ -5.721434,8115.000000,0 });
    ref.push_back({ -5.623059,8252.000000,0 });
    ref.push_back({ -5.524685,8204.000000,0 });
    ref.push_back({ -5.426311,8658.000000,0 });
    ref.push_back({ -5.327937,8567.000000,0 });
    ref.push_back({ -5.229562,8866.000000,0 });
    ref.push_back({ -5.131188,8814.000000,0 });
    ref.push_back({ -5.032814,9064.000000,0 });
    ref.push_back({ -4.934439,9227.000000,0 });
    ref.push_back({ -4.836065,9317.000000,0 });
    ref.push_back({ -4.737691,9357.000000,0 });
    ref.push_back({ -4.639316,9500.000000,0 });
    ref.push_back({ -4.540942,9718.000000,0 });
    ref.push_back({ -4.442568,9651.000000,0 });
    ref.push_back({ -4.344194,9947.000000,0 });
    ref.push_back({ -4.245819,9894.000000,0 });
    ref.push_back({ -4.147445,10073.000000,0 });
    ref.push_back({ -4.049071,10079.000000,0 });
    ref.push_back({ -3.950696,10386.000000,0 });
    ref.push_back({ -3.852322,10380.000000,0 });
    ref.push_back({ -3.753948,10695.000000,0 });
    ref.push_back({ -3.655573,10614.000000,0 });
    ref.push_back({ -3.557199,10557.000000,0 });
    ref.push_back({ -3.458825,10798.000000,0 });
    ref.push_back({ -3.360450,10944.000000,0 });
    ref.push_back({ -3.262076,10725.000000,0 });
    ref.push_back({ -3.163702,11298.000000,0 });
    ref.push_back({ -3.065328,11421.000000,0 });
    ref.push_back({ -2.966953,11459.000000,0 });
    ref.push_back({ -2.868579,11536.000000,0 });
    ref.push_back({ -2.770205,11553.000000,0 });
    ref.push_back({ -2.671830,11888.000000,0 });
    ref.push_back({ -2.573456,11846.000000,0 });
    ref.push_back({ -2.475082,11924.000000,0 });
    ref.push_back({ -2.376707,12138.000000,0 });
    ref.push_back({ -2.278333,12332.000000,0 });
    ref.push_back({ -2.179959,12319.000000,0 });
    ref.push_back({ -2.081585,12496.000000,0 });
    ref.push_back({ -1.983210,12654.000000,0 });
    ref.push_back({ -1.884836,12931.000000,0 });
    ref.push_back({ -1.786462,13159.000000,0 });
    ref.push_back({ -1.688087,13250.000000,0 });
    ref.push_back({ -1.589713,13259.000000,0 });
    ref.push_back({ -1.491339,13591.000000,0 });
    ref.push_back({ -1.392964,13561.000000,0 });
    ref.push_back({ -1.294590,13917.000000,0 });
    ref.push_back({ -1.196216,14253.000000,0 });
    ref.push_back({ -1.097841,14063.000000,0 });
    ref.push_back({ -0.999467,14493.000000,0 });
    ref.push_back({ -0.901093,14788.000000,0 });
    ref.push_back({ -0.802719,14838.000000,0 });
    ref.push_back({ -0.704344,15271.000000,0 });
    ref.push_back({ -0.605970,15731.000000,0 });
    ref.push_back({ -0.507596,15933.000000,0 });
    ref.push_back({ -0.409221,16165.000000,0 });
    ref.push_back({ -0.310847,16283.000000,0 });
    ref.push_back({ -0.212473,16897.000000,0 });
    ref.push_back({ -0.114098,17054.000000,0 });
    ref.push_back({ -0.015724,17454.000000,0 });
    ref.push_back({ 0.082650,18024.000000,0 });
    ref.push_back({ 0.181024,18373.000000,0 });
    ref.push_back({ 0.279399,19109.000000,0 });
    ref.push_back({ 0.377773,19888.000000,0 });
    ref.push_back({ 0.476147,20545.000000,0 });
    ref.push_back({ 0.574522,21895.000000,0 });
    ref.push_back({ 0.672896,22926.000000,0 });
    ref.push_back({ 0.771270,23799.000000,0 });
    ref.push_back({ 0.869645,25128.000000,0 });
    ref.push_back({ 0.968019,27209.000000,0 });
    ref.push_back({ 1.066393,28222.000000,0 });
    ref.push_back({ 1.164768,29263.000000,0 });
    ref.push_back({ 1.263142,30842.000000,0 });
    ref.push_back({ 1.361516,31893.000000,0 });
    ref.push_back({ 1.459890,33171.000000,0 });
    ref.push_back({ 1.558265,34014.000000,0 });
    ref.push_back({ 1.656639,34460.000000,0 });
    ref.push_back({ 1.755013,34611.000000,0 });
    ref.push_back({ 1.853388,34785.000000,0 });
    ref.push_back({ 1.951762,34898.000000,0 });
    ref.push_back({ 2.050136,34970.000000,0 });
    ref.push_back({ 2.148511,34985.000000,0 });
    ref.push_back({ 2.246885,33881.000000,0 });
    ref.push_back({ 2.345259,32655.000000,0 });
    ref.push_back({ 2.443633,31180.000000,0 });
    ref.push_back({ 2.542008,30176.000000,0 });
    ref.push_back({ 2.640382,29444.000000,0 });
    ref.push_back({ 2.738756,28602.000000,0 });
    ref.push_back({ 2.837131,27164.000000,0 });
    ref.push_back({ 2.935505,25789.000000,0 });
    ref.push_back({ 3.033879,24645.000000,0 });
    ref.push_back({ 3.132254,23698.000000,0 });
    ref.push_back({ 3.230628,22816.000000,0 });
    ref.push_back({ 3.329002,21899.000000,0 });
    ref.push_back({ 3.427377,21113.000000,0 });
    ref.push_back({ 3.525751,19958.000000,0 });
    ref.push_back({ 3.624125,18868.000000,0 });
    ref.push_back({ 3.722499,17071.000000,0 });
    ref.push_back({ 3.820874,15237.000000,0 });
    ref.push_back({ 3.919248,13722.000000,0 });
    ref.push_back({ 4.017622,11878.000000,0 });
    ref.push_back({ 4.115997,9594.000000,0 });
    ref.push_back({ 4.214371,7698.000000,0 });
    ref.push_back({ 4.312745,5819.000000,0 });
    ref.push_back({ 4.411120,4187.000000,0 });
    ref.push_back({ 4.509494,2810.000000,0 });
    ref.push_back({ 4.607868,1960.000000,0 });
    ref.push_back({ 4.706242,1264.000000,0 });
    ref.push_back({ 4.804617,881.000000,0 });
    ref.push_back({ 4.902991,636.000000,0 });
    ref.push_back({ 5.001365,442.000000,0 });
    ref.push_back({ 5.099740,230.000000,0 });
    ref.push_back({ 5.198114,41.000000,0 });
    std::wprintf(L"ref의 사이즈는 %d이고 target의 사이즈는 %d이다.\n",ref.size(), target.size());

    if (ref.size() != target.size())
    {
        exit(-1);
    }

    std::vector<std::array<double, 3>> wInput;
    for (int i = 0; i < target.size(); i++) wInput.push_back({ target[i][0],target[i][1],ref[i][1] });
    
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

    return wassersteinDist(wInput);
}