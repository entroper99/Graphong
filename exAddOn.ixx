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

export inline double calcGaussian(double dx, double dy, double dz, double sigma, double amplitude)
{
    return amplitude * std::exp(-(dx * dx + dy * dy + dz * dz) / (2 * sigma * sigma));
}
export inline double calcGaussianFast(double dx, double dy, double dz, const double& invSig, double amplitude)
{
    double distanceSq = dx * dx + dy * dy + dz * dz;
    return amplitude * std::exp(-distanceSq * invSig);
}
export double createDensityFunction(const std::vector<std::array<double, 3>>& inputPoints, double inputBoxSize, double*** density, const int& resolution)
{
    //std::printf("RESOL : %d, pointsSize : %d, inputBoxSize : %f\n", RESOLUTION, pbcPoints.size(), inputBoxSize);
    const double SIG = 0.68;
    const double SIG2 = SIG * SIG;
    const double INV_SIG = 1.0 / (2.0 * SIG2);

    double gaussAmp = 1.0;
    int totalSize = resolution * resolution * resolution;
    int numThreads = std::thread::hardware_concurrency();
    //std::printf("cpu number : %d\n", numThreads);
    int blockSize = totalSize / numThreads;

    double cutoff = 3.0 * SIG; //진폭의 99.7프로를 포함하는 값 
    double cutoffSq = cutoff * cutoff;

    auto calculateDensityPart = [&density, &inputPoints, inputBoxSize, SIG, INV_SIG, gaussAmp, cutoffSq, resolution](int startIdx, int endIdx)
        {
            //for (const auto& point : pbcPoints)std::printf("pbcPoints : %f,%f,%f\n", point[0],point[1],point[2]);
            for (int i = startIdx; i < endIdx; ++i)
            {
                int xIdx = i / (resolution * resolution);
                int yIdx = (i / resolution) % resolution;
                int zIdx = i % resolution;

                //10의 박스사이즈 2면은 2.5 7.5
                // (boxSize/resolution)/2.0

                double tgtX = (inputBoxSize / resolution) / 2.0 + xIdx * (inputBoxSize / resolution) - inputBoxSize / 2.0;
                double tgtY = (inputBoxSize / resolution) / 2.0 + yIdx * (inputBoxSize / resolution) - inputBoxSize / 2.0;
                double tgtZ = (inputBoxSize / resolution) / 2.0 + zIdx * (inputBoxSize / resolution) - inputBoxSize / 2.0;

                //if (getStep() != 0) std::printf("(%d,%d,%d) -> (%f,%f,%f)\n", xIdx, yIdx, zIdx, tgtX, tgtY, tgtZ);
                double densityValue = 0;


                for (const auto& point : inputPoints)
                {
                    double dx = point[0] - tgtX;
                    double dy = point[1] - tgtY;
                    double dz = point[2] - tgtZ;

                    dx -= inputBoxSize * std::round(dx / inputBoxSize);
                    dy -= inputBoxSize * std::round(dy / inputBoxSize);
                    dz -= inputBoxSize * std::round(dz / inputBoxSize);

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

export double getDirichletEnergy(double*** density, const int& resolution, double boxVolume)
{
    double dirichletEnergy = 0.0;
    double h = 0.1;

    for (int x = 0; x < resolution; ++x)
    {
        for (int y = 0; y < resolution; ++y)
        {
            for (int z = 0; z < resolution; ++z)
            {
                int xPrev = (x - 1 + resolution) % resolution;
                int xNext = (x + 1) % resolution; 
                int yPrev = (y - 1 + resolution) % resolution;
                int yNext = (y + 1) % resolution;
                int zPrev = (z - 1 + resolution) % resolution;
                int zNext = (z + 1) % resolution;

                double gradX = (density[xNext][y][z] - density[xPrev][y][z]) / (2.0 * h);
                double gradY = (density[x][yNext][z] - density[x][yPrev][z]) / (2.0 * h);
                double gradZ = (density[x][y][zNext] - density[x][y][zPrev]) / (2.0 * h);
                double gradSquared = gradX * gradX + gradY * gradY + gradZ * gradZ;

                dirichletEnergy += gradSquared;
            }
        }
    }

    dirichletEnergy *= (boxVolume / (resolution * resolution * resolution));
    return dirichletEnergy;
}

export double*** createLaplacian(double*** density, int resol, double boxSize)
{
    double*** laplacian = create3DArray(resol, resol, resol);
    double del = boxSize / (resol - 1);
    double delSquared = del * del;
    for (int x = 0; x < resol; x++)
    {
        for (int y = 0; y < resol; y++)
        {
            for (int z = 0; z < resol; z++)
            {
                laplacian[x][y][z] = 0;

                int x_next = (x + 1) % resol;
                int x_prev = (x - 1 + resol) % resol;
                int y_next = (y + 1) % resol;
                int y_prev = (y - 1 + resol) % resol;
                int z_next = (z + 1) % resol;
                int z_prev = (z - 1 + resol) % resol;

                laplacian[x][y][z] += (density[x_next][y][z] - 2 * density[x][y][z] + density[x_prev][y][z]);
                laplacian[x][y][z] += (density[x][y_next][z] - 2 * density[x][y][z] + density[x][y_prev][z]);
                laplacian[x][y][z] += (density[x][y][z_next] - 2 * density[x][y][z] + density[x][y][z_prev]);
                laplacian[x][y][z] /= delSquared;
            }
        }
    }
    return laplacian;
}

export double*** createDerivative3(double*** density, int resol, double boxSize)
{
    double*** deriv3 = create3DArray(resol, resol, resol);
    double del = boxSize / (resol - 1);
    double delCubed = del * del * del;

    for (int x = 0; x < resol; x++)
    {
        for (int y = 0; y < resol; y++)
        {
            for (int z = 0; z < resol; z++)
            {
                int x_next = (x + 1) % resol;
                int x_prev = (x - 1 + resol) % resol;
                int x_next2 = (x + 2) % resol;
                int x_prev2 = (x - 2 + resol) % resol;

                int y_next = (y + 1) % resol;
                int y_prev = (y - 1 + resol) % resol;
                int y_next2 = (y + 2) % resol;
                int y_prev2 = (y - 2 + resol) % resol;

                int z_next = (z + 1) % resol;
                int z_prev = (z - 1 + resol) % resol;
                int z_next2 = (z + 2) % resol;
                int z_prev2 = (z - 2 + resol) % resol;

                deriv3[x][y][z] = (density[x_next2][y][z] - 2 * density[x_next][y][z] + 2 * density[x_prev][y][z] - density[x_prev2][y][z]) / (2 * delCubed);
                deriv3[x][y][z] += (density[x][y_next2][z] - 2 * density[x][y_next][z] + 2 * density[x][y_prev][z] - density[x][y_prev2][z]) / (2 * delCubed);
                deriv3[x][y][z] += (density[x][y][z_next2] - 2 * density[x][y][z_next] + 2 * density[x][y][z_prev] - density[x][y][z_prev2]) / (2 * delCubed);
            }
        }
    }
    return deriv3;
}

export double*** createDerivative4(double*** density, int resol, double boxSize)
{
    double*** deriv4 = create3DArray(resol, resol, resol);
    double del = boxSize / (resol - 1);
    double delFourth = del * del * del * del;

    for (int x = 0; x < resol; x++)
    {
        for (int y = 0; y < resol; y++)
        {
            for (int z = 0; z < resol; z++)
            {
                // 주기적인 경계 조건을 적용
                int x_next = (x + 1) % resol;
                int x_prev = (x - 1 + resol) % resol;
                int x_next2 = (x + 2) % resol;
                int x_prev2 = (x - 2 + resol) % resol;
                int x_next3 = (x + 3) % resol;
                int x_prev3 = (x - 3 + resol) % resol;
                int x_next4 = (x + 4) % resol;
                int x_prev4 = (x - 4 + resol) % resol;

                int y_next = (y + 1) % resol;
                int y_prev = (y - 1 + resol) % resol;
                int y_next2 = (y + 2) % resol;
                int y_prev2 = (y - 2 + resol) % resol;
                int y_next3 = (y + 3) % resol;
                int y_prev3 = (y - 3 + resol) % resol;
                int y_next4 = (y + 4) % resol;
                int y_prev4 = (y - 4 + resol) % resol;

                int z_next = (z + 1) % resol;
                int z_prev = (z - 1 + resol) % resol;
                int z_next2 = (z + 2) % resol;
                int z_prev2 = (z - 2 + resol) % resol;
                int z_next3 = (z + 3) % resol;
                int z_prev3 = (z - 3 + resol) % resol;
                int z_next4 = (z + 4) % resol;
                int z_prev4 = (z - 4 + resol) % resol;

                // 4차 중앙 차분을 이용한 4차 도함수 계산
                deriv4[x][y][z] = (-density[x_next4][y][z] + 4 * density[x_next3][y][z] - 6 * density[x_next2][y][z] + 4 * density[x_next][y][z] - 6 * density[x][y][z] + 4 * density[x_prev][y][z] - 6 * density[x_prev2][y][z] + 4 * density[x_prev3][y][z] - density[x_prev4][y][z]) / delFourth;
                deriv4[x][y][z] += (-density[x][y_next4][z] + 4 * density[x][y_next3][z] - 6 * density[x][y_next2][z] + 4 * density[x][y_next][z] - 6 * density[x][y][z] + 4 * density[x][y_prev][z] - 6 * density[x][y_prev2][z] + 4 * density[x][y_prev3][z] - density[x][y_prev4][z]) / delFourth;
                deriv4[x][y][z] += (-density[x][y][z_next4] + 4 * density[x][y][z_next3] - 6 * density[x][y][z_next2] + 4 * density[x][y][z_next] - 6 * density[x][y][z] + 4 * density[x][y][z_prev] - 6 * density[x][y][z_prev2] + 4 * density[x][y][z_prev3] - density[x][y][z_prev4]) / delFourth;
            }
        }
    }
    return deriv4;
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

//export struct Point
//{
//    double x, y, z;
//
//    bool operator==(const Point& other) const
//    {
//        return x == other.x && y == other.y && z == other.z;
//    }
//
//    double distance(const Point& other) const
//    {
//        double dx = x - other.x;
//        double dy = y - other.y;
//        double dz = z - other.z;
//        return std::sqrt(dx * dx + dy * dy + dz * dz);
//    }
//};
//
//export struct Triangle
//{
//    Point p1, p2, p3;
//
//    bool operator==(const Triangle& other) const
//    {
//        return (p1 == other.p1 || p1 == other.p2 || p1 == other.p3) &&
//            (p2 == other.p1 || p2 == other.p2 || p2 == other.p3) &&
//            (p3 == other.p1 || p3 == other.p2 || p3 == other.p3);
//    }
//
//    double getArea() const
//    {
//        double a = p1.distance(p2);
//        double b = p2.distance(p3);
//        double c = p3.distance(p1);
//        double s = (a + b + c) / 2.0;
//        return std::sqrt(s * (s - a) * (s - b) * (s - c));
//    }
//
//    bool containsPoint(const Point& p) const
//    {
//        double totalArea = getArea();
//        double subArea1 = Triangle{ p, p2, p3 }.getArea();
//        double subArea2 = Triangle{ p1, p, p3 }.getArea();
//        double subArea3 = Triangle{ p1, p2, p }.getArea();
//        double sum = subArea1 + subArea2 + subArea3;
//        return std::abs(totalArea - sum) < 1e-6;
//    }
//};
//
//
//export std::vector<Triangle> getTrianglesFromScalar(const Eigen::Tensor<double, 3>& scalarField, double isoLevel, double tolerance)
//{
//    const int dimX = scalarField.dimension(0);
//    const int dimY = scalarField.dimension(1);
//    const int dimZ = scalarField.dimension(2);
//
//    Eigen::MatrixXd GV(dimX * dimY * dimZ, 3);
//    Eigen::VectorXd GS(dimX * dimY * dimZ);
//
//    for (int z = 0; z < dimZ; z++) {
//        for (int y = 0; y < dimY; y++) {
//            for (int x = 0; x < dimX; x++) {
//                int idx = x + y * dimX + z * dimX * dimY;
//                GV.row(idx) << x, y, z;
//                GS(idx) = scalarField(x, y, z);
//            }
//        }
//    }
//
//    for (int i = 0; i < GS.size(); i++) {
//        if (std::abs(GS(i) - isoLevel) > tolerance) {
//            GS(i) = (GS(i) > isoLevel) ? isoLevel + tolerance * 2 : isoLevel - tolerance * 2;
//        }
//    }
//
//    Eigen::MatrixXd V;
//    Eigen::MatrixXi F;
//    igl::marching_cubes(GS, GV, dimX, dimY, dimZ, isoLevel, V, F);
//
//    std::vector<Triangle> triangles;
//    triangles.reserve(F.rows());
//
//    for (int i = 0; i < F.rows(); i++) {
//        Triangle tri;
//        Eigen::Vector3d v1 = V.row(F(i, 0));
//        Eigen::Vector3d v2 = V.row(F(i, 1));
//        Eigen::Vector3d v3 = V.row(F(i, 2));
//
//        tri.p1 = Point(v1.x(), v1.y(), v1.z());
//        tri.p2 = Point(v2.x(), v2.y(), v2.z());
//        tri.p3 = Point(v3.x(), v3.y(), v3.z());
//
//        triangles.push_back(tri);
//    }
//
//    return triangles;
//}
//
//export double getAreaFromTriangles(const std::vector<Triangle>& triangles)
//{
//    double totalArea = 0.0;
//    for (const auto& triangle : triangles) totalArea += triangle.getArea();
//    return totalArea;
//}
