#define _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS
#include <SDL.h>
#include <Eigen/Dense>
#include <Eigen/Core>
#include <unsupported/Eigen/CXX11/Tensor>
#include <fftw3.h>
#include <igl/marching_cubes.h>

export module Func;

import std;
import globalVar;
import constVar;
import Shapes;
import utilMath;
import nanoTimer;
import ThreadPool;
import exAddOn;

std::mutex density_mutex;

export struct Func
{
    std::wstring funcName = L"NULL";
    funcFlag funcType = funcFlag::none;
    std::vector<Point> myPoints;
    std::vector<Point> myInterPoints;
    SDL_Color myColor = { 0xff,0xff,0xff };

    std::unordered_map<Point, double, decltype(pointHash)> scalar;
    double scalarInfimum = 0.0;
    double scalarSupremum = 1.0;

    //삼각분할 관련 변수
    Triangle superTriangle;
    std::vector < Triangle > triangles;
    std::vector<Point> selectPts;
    std::vector<Point> unselectPts = myPoints;
    std::vector<Triangle> checkTriangles;
    std::vector<Triangle> badTriangles;
    std::vector<Point> badPts;
    std::vector<Edge> edges;
    bool deleteSupertri = false;
    bool triFirstRun = true;

    inline static bool hasTranslation = false;
    inline static bool hasRotation = false;
    inline static Eigen::Vector3d transVec;
    inline static Eigen::Matrix3d rotMat;

    double period = 0.0;
    bool interLine = false;

    std::function<double(double, double, double)> scalarFunc;

    double latticeConstant = 0;

    //bool hasFourierRef = false;
    //std::vector<std::array<std::complex<double>, 4>> fourierRef;

    Func(funcFlag inputType) : funcType(inputType) , scalarFunc([](double x, double y, double z) -> double { return 0.0; })
    {
        funcSet.push_back(this);
    };

    ~Func()
    {
        funcSet.erase(std::find(funcSet.begin(), funcSet.end(), this));
    }

    std::vector<std::array<double, 3>> getRawPoints()
    {
        std::vector<std::array<double, 3>> rtnPoints;
        for (int i = 0; i < myPoints.size(); i++) rtnPoints.push_back({myPoints[i].x,myPoints[i].y,myPoints[i].z});
        return rtnPoints;
    }

    void singleTriangulation()
    {
        if (triFirstRun == true)
        {
            double totalX = 0, totalZ = 0;
            double maxX = -99999, minX = 99999, maxZ = -99999, minZ = 99999;
            for (int i = 0; i < myPoints.size(); i++)
            {
                totalX += myPoints[i].x;
                if (myPoints[i].x > maxX) maxX = myPoints[i].x;
                if (myPoints[i].x < minX) minX = myPoints[i].x;

                totalZ += myPoints[i].z;
                if (myPoints[i].z > maxZ) maxZ = myPoints[i].z;
                if (myPoints[i].z < minZ) minZ = myPoints[i].z;
            }
            double avgX = totalX / myPoints.size(), avgZ = totalZ / myPoints.size();
            double distX = std::fabs(maxX - minX), distZ = std::fabs(maxZ - minZ);
            double maxDist = std::max(distX, distZ);
            double triSize = maxDist * 8.0;
            superTriangle.p1 = { avgX, 0, avgZ - triSize };
            superTriangle.p2 = { avgX - triSize, 0, avgZ + triSize };
            superTriangle.p3 = { avgX + triSize, 0, avgZ + triSize };

            checkTriangles = { superTriangle };
            selectPts = { superTriangle.p1,superTriangle.p2,superTriangle.p3 };

            unselectPts = myPoints;
            std::sort(unselectPts.begin(), unselectPts.end(), [](const Point& a, const Point& b)
                {
                    double distA = a.x * a.x + a.z * a.z;
                    double distB = b.x * b.x + b.z * b.z;
                    return distA < distB;
                });
            triFirstRun = false;
            std::wprintf(L"슈퍼트라이앵글을 생성하였다..\n");
        }

        if (unselectPts.size() == 0)
        {
            if (deleteSupertri == false)
            {
                std::wprintf(L"슈퍼트라이앵글 관련 삼각형을 제거하였다.\n");
                for (auto it = triangles.begin(); it != triangles.end();)
                {

                    if (it->p1 == superTriangle.p1 || it->p1 == superTriangle.p2 || it->p1 == superTriangle.p3
                        || it->p2 == superTriangle.p1 || it->p2 == superTriangle.p2 || it->p2 == superTriangle.p3
                        || it->p3 == superTriangle.p1 || it->p3 == superTriangle.p2 || it->p3 == superTriangle.p3)
                    {
                        it = triangles.erase(it);
                    }
                    else
                    {
                        it++;
                    }
                }
                std::wprintf(L"삼각분할을 완료하였다.\n");
                deleteSupertri = true;
                return;
            }
            else
            {
                return;
            }
        }

        Point tgtPt = unselectPts[0];
        //Point tgtPt = unselectPts[randomRange(0, unselectPts.size() - 1)];

        edges.clear();
        badPts.clear();
        badTriangles.clear();

        std::wprintf(L"[루프 스타트] 선택된 점: (%f, %f, %f)\n", tgtPt.x, tgtPt.y, tgtPt.z);

        selectPts.push_back(tgtPt);
        unselectPts.erase(std::find(unselectPts.begin(), unselectPts.end(), tgtPt));

        for (auto tri = checkTriangles.begin(); tri != checkTriangles.end();)
        {
            bool found = false;
            //std::wprintf(L"검사 대상 삼각형: (%f,%f,%f)-(%f,%f,%f)-(%f,%f,%f)\n", tri->p1.x, tri->p1.y, tri->p1.z, tri->p2.x, tri->p2.y, tri->p2.z, tri->p3.x, tri->p3.y, tri->p3.z);
            for (int i = 0; i < selectPts.size(); i++)
            {
                if (makeCircumcircle(*tri).inCircle(selectPts[i]))
                {
                    if (std::find(badTriangles.begin(), badTriangles.end(), *tri) == badTriangles.end())
                    {
                        badTriangles.push_back(*tri);
                        tri = checkTriangles.erase(tri);
                        found = true;
                        break;
                    }
                    //else std::wprintf(L"이미 존재하는 badTriangle이다!\n");
                    break;
                }
            }

            if (found == false) tri++;
        }

        for (auto tri = badTriangles.begin(); tri != badTriangles.end(); tri++)
        {
            badPts.push_back(tri->p1);
            badPts.push_back(tri->p2);
            badPts.push_back(tri->p3);

            edges.push_back({ tri->p1, tri->p2 });
            edges.push_back({ tri->p2, tri->p3 });
            edges.push_back({ tri->p1, tri->p3 });
        }

        //std::wprintf(L"나쁜 삼각형 개수: %d\n", badTriangles.size());
        //std::wprintf(L"나쁜 점 개수: %d\n", badPts.size());
        //std::wprintf(L"▽엣지 개수 (중복 제거 전): %d\n", edges.size());

        //for (int i = 0; i < edges.size(); i++)
        //{
        //    std::wprintf(L"%d번 엣지 : (%f,%f,%f)-(%f,%f,%f) \n", i, edges[i].p1.x, edges[i].p1.y, edges[i].p1.z, edges[i].p2.x, edges[i].p2.y, edges[i].p2.z);
        //}


        // 다각형 주변 엣지만 남도록 겹치는 엣지들 제거
        std::vector<Edge> edgesCopy = edges;
        for (int i = 0; i < edgesCopy.size(); i++)
        {
            bool tolerance = false;
            for (int j = 0; j < edges.size(); j++)
            {
                if (edges[j] == edgesCopy[i])
                {
                    if (tolerance == false) tolerance = true;
                    else
                    {
                        for (auto it = edges.begin(); it != edges.end();)
                        {
                            if (*it == edgesCopy[i]) it = edges.erase(it);
                            else ++it;
                        }
                        goto loopEnd;
                    }
                }
            }
        loopEnd:;
        }

        //std::wprintf(L"▽엣지 개수 (중복 제거 후): %d\n", edges.size());
        //for (int i = 0; i < edges.size(); i++) std::wprintf(L"%d번 엣지 : (%f,%f,%f)-(%f,%f,%f) \n", i, edges[i].p1.x, edges[i].p1.y, edges[i].p1.z, edges[i].p2.x, edges[i].p2.y, edges[i].p2.z);

        // 뻗어나가는 엣지
        for (int i = 0; i < badPts.size(); i++)
        {
            edges.push_back({ tgtPt, badPts[i] });
        }

        //std::wprintf(L"엣지 개수 (뻗어나가는 엣지 추가 후): %d\n", edges.size());
        //for (int i = 0; i < edges.size(); i++) std::wprintf(L"%d번 엣지 : (%f,%f,%f)-(%f,%f,%f) \n", i, edges[i].p1.x, edges[i].p1.y, edges[i].p1.z, edges[i].p2.x, edges[i].p2.y, edges[i].p2.z);

        auto pointHash = [](const Point& p)
            {
                std::hash<double> hasher;
                return ((hasher(p.x) * 73856093) ^ (hasher(p.y) * 19349663) ^ (hasher(p.z) * 83492791));
            };

        std::unordered_map<Point, std::vector<Point>, decltype(pointHash)> edgeMap(0, pointHash);

        // 엣지 맵 생성
        for (const auto& edge : edges) {
            edgeMap[edge.p1].push_back(edge.p2);
            edgeMap[edge.p2].push_back(edge.p1);
        }

        // 새로운 삼각형 생성
        for (const auto& entry : edgeMap)
        {
            const Point& p1 = entry.first;
            const std::vector<Point>& connectedPoints = entry.second;
            for (size_t i = 0; i < connectedPoints.size(); i++)
            {
                const Point& p2 = connectedPoints[i];
                for (size_t j = i + 1; j < connectedPoints.size(); j++)
                {
                    const Point& p3 = connectedPoints[j];
                    if (std::find(edgeMap[p2].begin(), edgeMap[p2].end(), p3) != edgeMap[p2].end())
                    {
                        Triangle newTri = { p1,p2,p3 };
                        if (std::find(checkTriangles.begin(), checkTriangles.end(), newTri) == checkTriangles.end())
                        {
                            if (std::find(badTriangles.begin(), badTriangles.end(), newTri) == badTriangles.end())
                            {
                                if (newTri.p1 == tgtPt || newTri.p2 == tgtPt || newTri.p3 == tgtPt)
                                {
                                    for (int i = 0; i < selectPts.size(); i++)
                                    {
                                        if (makeCircumcircle(newTri).inCircle(selectPts[i])) break;

                                        if (i == selectPts.size() - 1)
                                        {
                                            //std::wprintf(L"새로운 삼각형: (%f,%f,%f)-(%f,%f,%f)-(%f,%f,%f)\n", newTri.p1.x, newTri.p1.y, newTri.p1.z, newTri.p2.x, newTri.p2.y, newTri.p2.z, newTri.p3.x, newTri.p3.y, newTri.p3.z);
                                            checkTriangles.push_back(newTri);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        triangles = checkTriangles;
        std::wprintf(L"새로운 검사 삼각형 개수: %d\n\n", checkTriangles.size());
    };

    void triangulation()
    {
        for (int i = 0; i < myPoints.size() + 1; i++)
        {
            singleTriangulation();
        }
    };

    void rotation(Eigen::Matrix3d inputMat)
    {
        std::unordered_map<Point, double, decltype(pointHash)> newScalar;
        for (int i = 0; i < myPoints.size(); i++)
        {
            double originX = myPoints[i].x;
            double originY = myPoints[i].y;
            double originZ = myPoints[i].z;

            Eigen::Vector3d vec3;
            vec3 << originX, originY, originZ;

            Eigen::Vector3d resultVec = inputMat * vec3;

            myPoints[i].x = resultVec[0];
            myPoints[i].y = resultVec[1];
            myPoints[i].z = resultVec[2];

            if (funcType == funcFlag::scalarField) newScalar[{myPoints[i].x, myPoints[i].y, myPoints[i].z}] = scalar[{originX, originY, originZ}];
        }
        scalar = newScalar;
    }

    void rotation(std::vector<Point>& inputPoints, Eigen::Matrix3d inputMat)
    {
        for (int i = 0; i < inputPoints.size(); i++)
        {
            Eigen::Vector3d vec3 = { inputPoints[i].x, inputPoints[i].y, inputPoints[i].z };
            Eigen::Vector3d resultVec = inputMat * vec3;
            inputPoints[i].x = resultVec[0];
            inputPoints[i].y = resultVec[1];
            inputPoints[i].z = resultVec[2];
        }
    }

    void translation(double inputX, double inputY, double inputZ)
    {
        std::unordered_map<Point, double, decltype(pointHash)> newScalar;
        for (int i = 0; i < myPoints.size(); i++)
        {

            double originX = myPoints[i].x;
            double originY = myPoints[i].y;
            double originZ = myPoints[i].z;

            myPoints[i].x += inputX;
            myPoints[i].y += inputY;
            myPoints[i].z += inputZ;

            if (funcType == funcFlag::scalarField) newScalar[{myPoints[i].x, myPoints[i].y, myPoints[i].z}] = scalar[{originX, originY, originZ}];
        }
        scalar = newScalar;
    }

    void translation(std::vector<Point>& inputPoints, Eigen::Vector3d inputVec)
    {
        for (int i = 0; i < myPoints.size(); i++)
        {
            double originX = inputPoints[i].x;
            double originY = inputPoints[i].y;
            double originZ = inputPoints[i].z;

            inputPoints[i].x += inputVec[0];
            inputPoints[i].y += inputVec[1];
            inputPoints[i].z += inputVec[2];
        }
    }

    void scalarCalc()
    {
        std::unordered_map<Point, double, decltype(pointHash)> newScalar;
        for (int i = 0; i < myPoints.size(); i++)
        {
            if (funcType == funcFlag::scalarField) newScalar[{myPoints[i].x, myPoints[i].y, myPoints[i].z}] = scalarFunc(myPoints[i].x, myPoints[i].y, myPoints[i].z);
        }
        scalar = newScalar;
    }

    double scalarAvg()
    {
        double totalVal = 0;
        for (int i = 0; i < myPoints.size(); i++)
        {
            totalVal += scalar[{myPoints[i].x, myPoints[i].y, myPoints[i].z}];
        }

        return totalVal / (double)myPoints.size();
    }

    double scalarSquareAvg()
    {
        double totalVal = 0;
        for (int i = 0; i < myPoints.size(); i++)
        {
            double val = scalar[{myPoints[i].x, myPoints[i].y, myPoints[i].z}];
            totalVal += val * val;
        }

        double meanVal = totalVal / (double)myPoints.size();
        return std::sqrt(meanVal);
    }

    int countPointsWithinLattice()
    {
        if (latticeConstant == 0)
        {
            std::wprintf(L"격자상수가 정의되어있지 않다.");
            return 0;
        }

        int count = 0;
        float halfLattice = latticeConstant / 2.0;

        for (const auto& point : myPoints)
        {
            if (std::abs(point.x) <= halfLattice &&
                std::abs(point.y) <= halfLattice &&
                std::abs(point.z) <= halfLattice)
            {
                count++;
            }
        }

        return count;
    }

    void eraseExternalLattice(std::vector<Point>& inputPoints, double inputLatticeConstant)
    {
        auto it = std::remove_if(inputPoints.begin(), inputPoints.end(), [=](const Point& p)
            {
                return std::abs(p.x) > inputLatticeConstant / 2.0 || std::abs(p.y) > inputLatticeConstant / 2.0 || std::abs(p.z) > inputLatticeConstant / 2.0;
            }
        );
        inputPoints.erase(it, inputPoints.end());
    }

    void latticeDuplicate(std::vector<Point>& inputPoints, double inputLatticeConstant)
    {
        std::vector<Point> newMyPoints;
        for (const auto& point : inputPoints)
        {
            for (int dx = -1; dx <= 1; dx++)
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    for (int dz = -1; dz <= 1; dz++)
                    {
                        Point newPoint;
                        newPoint.x = point.x + dx * latticeConstant;
                        newPoint.y = point.y + dy * latticeConstant;
                        newPoint.z = point.z + dz * latticeConstant;
                        newMyPoints.push_back(newPoint);
                    }
                }
            }
        }
        inputPoints = newMyPoints;
    }

    void latticeRotation(std::vector<Point>& inputPoints, double inputLatticeConstant, Eigen::Matrix3d inputMatrix)
    {
        eraseExternalLattice(inputPoints, inputLatticeConstant);
        latticeDuplicate(inputPoints, inputLatticeConstant);
        rotation(inputPoints, inputMatrix);
        eraseExternalLattice(inputPoints, inputLatticeConstant);
    }

    void latticeTranslation(std::vector<Point>& inputPoints, double inputLatticeConstant, Eigen::Vector3d inputVector)
    {
        eraseExternalLattice(inputPoints, inputLatticeConstant);
        latticeDuplicate(inputPoints, inputLatticeConstant);
        translation(inputPoints, inputVector);
        eraseExternalLattice(inputPoints, inputLatticeConstant);
    }

    void sortByCOM()
    {
        std::wprintf(L"COM을 이용하여 중앙정렬을 시작합니다.");
        if (hasTranslation == false)
        {
            //중심점 계산
            double totalX = 0, totalY = 0, totalZ = 0;
            for (const auto& point : myPoints)
            {
                totalX += point.x;
                totalY += point.y;
                totalZ += point.z;
            }
            double centerX = totalX / myPoints.size();
            double centerY = totalY / myPoints.size();
            double centerZ = totalZ / myPoints.size();


            //데이터를 원점을 기준으로 변경
            for (int i = 0; i < myPoints.size(); i++)
            {
                myPoints[i].x -= centerX;
                myPoints[i].y -= centerY;
                myPoints[i].z -= centerZ;
            }
            
            transVec = { -centerX, -centerY, -centerZ };
            hasTranslation = true;
        }
        else
        {
            for (int i = 0; i < myPoints.size(); i++)
            {
                myPoints[i].x += transVec[0];
                myPoints[i].y += transVec[1];
                myPoints[i].z += transVec[2];
            }
        }

        scalarCalc();

        std::cout << "\n평행이동 벡터" << ":\n" << transVec << "\n\n";
        //std::wprintf(L"\033[0;33m이 함수의 평균 f값은 %f이다.\033[0m\n", scalarSquareAvg());
    }

    void sortByPCA()
    {
        std::wprintf(L"PCA를 이용하여 회전정렬을 시작합니다.");
        if (latticeConstant == 0)
        {
            std::wprintf(L"[Error] 격자상수가 정의되지 않은 상태에서 PCA를 실행했다.\n");
            return;
        }

        if (1)//(hasRotation == false )
        {


            std::vector<Point> testPoints;
            auto isInSphere = [](Point inputP, double rad, double cx, double cy, double cz)->bool
                {
                    double dx = inputP.x - cx;
                    double dy = inputP.y - cy;
                    double dz = inputP.z - cz;
                    double distanceSquared = dx * dx + dy * dy + dz * dz;
                    return distanceSquared <= rad * rad;
                };
            
            for (int i = 0; i < myPoints.size(); i++)
            {
                if (isInSphere(myPoints[i], latticeConstant / 2.0, 0, 0, 0))
                {
                    testPoints.push_back(myPoints[i]);
                    std::wprintf(L"구 내부의 점 (%f,%f,%f)를 컨테이너에 넣었다.\n",myPoints[i].x, myPoints[i].y, myPoints[i].z);
                }
            }
            std::wprintf(L"구 내부의 점들은 총 %d개이다.\n", testPoints.size());


            Eigen::MatrixXd matX;
            matX.resize(testPoints.size(), 3);
            for (int row = 0; row < testPoints.size(); row++)
            {
                matX(row, 0) = testPoints[row].x;
                matX(row, 1) = testPoints[row].y;
                matX(row, 2) = testPoints[row].z;
            }

            Eigen::MatrixXd matXT = matX.transpose();
            Eigen::MatrixXd matCov = (1.0 / (double)testPoints.size()) * (matXT * matX);

            Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(matCov);
            Eigen::VectorXd eigenvalues = solver.eigenvalues();
            Eigen::MatrixXd eigenvectors = solver.eigenvectors();

            std::cout << "Eigenvalues " << ":\n" << eigenvalues << "\n\n";
            for (int i = 0; i < eigenvectors.cols(); ++i)
            {
                std::cout << "Eigenvector " << i + 1 << ":\n" << eigenvectors.col(i) << "\n\n";
            }

            double maxVal = eigenvalues[0];
            int maxIndex = 0;

            for (int i = 1; i < eigenvalues.size(); ++i)
            {
                double currentVal = eigenvalues[i];
                if (currentVal > maxVal)
                {
                    maxVal = currentVal;
                    maxIndex = i;
                }
            }

            Eigen::Matrix3d tgtMat;
            tgtMat = eigenvectors;

            Eigen::Matrix3d originMat;
            originMat << 0.859311, 0.485435, 0.161053,
                0.417805, -0.484633, -0.768485,
                0.294998, -0.727656, 0.619268;

            Eigen::Matrix3d rotationMat = originMat * tgtMat.inverse();

            hasRotation = true;
            rotMat = rotationMat;
        }
        else
        {
        }

        latticeDuplicate(myPoints,latticeConstant);
        rotation(myPoints, rotMat);
        eraseExternalLattice(myPoints, latticeConstant);
        printRotationMatrix(rotMat);
        scalarCalc();
        //std::wprintf(L"\033[0;33m이 함수의 평균 f값은 %f이다.\033[0m\n", scalarSquareAvg());
    }

    std::vector<std::array<std::complex<double>, 4>> convertToDensityFuncAndFFT(const std::vector<Point>& inputPoints, double inputLatticeConstant, int inputGridSize)
    {
        double del = inputLatticeConstant / (inputGridSize - 1);
        int gridSize = inputGridSize * inputGridSize * inputGridSize;

        Eigen::Tensor<double, 3> density(inputGridSize, inputGridSize, inputGridSize);
        density.setZero();
        density = createDensityFunctionTensor(inputPoints, inputLatticeConstant, inputGridSize);

        Eigen::Tensor<double, 3> windowFunc = createBlackmanWindow(inputGridSize);
        density = density * windowFunc;
        std::wprintf(L"창함수를 곱하였다.\n");


        fftw_complex* input, * output;
        fftw_plan p;
        input = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * gridSize);
        output = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * gridSize);
        if (!input || !output)
        {
            std::wprintf(L"[FFT] 메모리 할당 실패\n");
            if (input) fftw_free(input);
            if (output) fftw_free(output);
            return {};
        }

        int idx = 0;
        for (int x = 0; x < inputGridSize; ++x)
        {
            for (int y = 0; y < inputGridSize; ++y)
            {
                for (int z = 0; z < inputGridSize; ++z)
                {
                    input[idx][0] = density(x, y, z);//실수
                    input[idx][1] = 0.0;//허수
                    ++idx;
                }
            }
        }

        std::wprintf(L"푸리에변환 입력값 구성이 끝났다.\n");


        p = fftw_plan_dft_3d(inputGridSize, inputGridSize, inputGridSize, input, output, FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(p);

        std::wprintf(L"푸리에변환 계산이 끝났다.\n");


        double delta_x = del;
        double delta_y = del;
        double delta_z = del;

        std::vector<std::array<std::complex<double>, 4>> fourierResult;
        for (int x = 0; x < inputGridSize; ++x)
        {
            for (int y = 0; y < inputGridSize; ++y)
            {
                for (int z = 0; z < inputGridSize; ++z)
                {
                    int i = z * inputGridSize * inputGridSize + y * inputGridSize + x;

                    int kx = (x <= inputGridSize / 2) ? x : x - inputGridSize;
                    int ky = (y <= inputGridSize / 2) ? y : y - inputGridSize;
                    int kz = (z <= inputGridSize / 2) ? z : z - inputGridSize;

                    double fx = kx / (inputGridSize * delta_x);
                    double fy = ky / (inputGridSize * delta_y);
                    double fz = kz / (inputGridSize * delta_z);

                    std::complex<double> complexVal(output[i][0], output[i][1]);
                    fourierResult.push_back({ std::complex<double>(kx, 0), std::complex<double>(ky, 0), std::complex<double>(kz, 0), complexVal });
                }
            }
        }

        wprintf(L"푸리에변환한 결과값을 저장했다.\n");


        fftw_destroy_plan(p);
        fftw_free(input);
        fftw_free(output);
        return fourierResult;
    }

    Eigen::Tensor<double, 3> createDensityFunctionTensor(const std::vector<Point>& inputPoints, double inputLatticeConstant, int inputGridSize)
    {
        double*** density = create3DArray(inputGridSize, inputGridSize, inputGridSize);

        std::vector<std::array<double, 3>> rtnPoints;
        for (int i = 0; i < inputPoints.size(); i++) rtnPoints.push_back({ inputPoints[i].x,inputPoints[i].y,inputPoints[i].z });
        createDensityFunction(rtnPoints, inputLatticeConstant, density, inputGridSize);

        Eigen::Tensor<double, 3> densityTensor(inputGridSize, inputGridSize, inputGridSize);
        for (int x = 0; x < inputGridSize; x++)
        {
            for (int y = 0; y < inputGridSize; y++)
            {
                for (int z = 0; z < inputGridSize; z++)
                {
                    densityTensor(x, y, z) = density[x][y][z];
                }
            }
        }
        free3DArray(density, inputGridSize, inputGridSize);
        return densityTensor;
    }

    std::array<double,6> calcCurvature(const std::vector<Point>& inputPoints, double inputLatticeConstant, int inputGridSize)
    {
        double del = inputLatticeConstant / (inputGridSize - 1);
        double delSquared = del * del;
        int gridSize = inputGridSize * inputGridSize * inputGridSize;
        Eigen::Tensor<double, 3> density(inputGridSize, inputGridSize, inputGridSize);
        density = createDensityFunctionTensor(inputPoints, inputLatticeConstant, inputGridSize);
        Eigen::Tensor<double, 3> laplacian(inputGridSize-2, inputGridSize-2, inputGridSize-2);
        laplacian.setZero();
        for (int x = 1; x < inputGridSize - 1; x++)
        {
            for (int y = 1; y < inputGridSize - 1; y++)
            {
                for (int z = 1; z < inputGridSize - 1; z++)
                {
                    laplacian(x - 1, y - 1, z - 1) += (density(x + 1, y, z) - 2 * density(x, y, z) + density(x - 1, y, z));
                    laplacian(x - 1, y - 1, z - 1) += (density(x, y + 1, z) - 2 * density(x, y, z) + density(x, y - 1, z));
                    laplacian(x - 1, y - 1, z - 1) += (density(x, y, z + 1) - 2 * density(x, y, z) + density(x, y, z - 1));
                    laplacian(x - 1, y - 1, z - 1) /= delSquared;
                }
            }
        }

        double variance = 0.0;
        double totalVal = 0.0;
        for (int x = 0; x < inputGridSize - 2; x++)
        {
            for (int y = 0; y < inputGridSize - 2; y++)
            {
                for (int z = 0; z < inputGridSize - 2; z++)
                {
                    totalVal += laplacian(x, y, z);
                }
            }
        }
        double mean = totalVal / (double)laplacian.size();
        for (int x = 0; x < inputGridSize - 2; x++)
        {
            for (int y = 0; y < inputGridSize - 2; y++)
            {
                for (int z = 0; z < inputGridSize - 2; z++)
                {
                    variance += std::pow(laplacian(x, y, z) - mean, 2.0);
                }
            }
        }
        variance /= (double)laplacian.size();
        double stddev = std::sqrt(variance);
        std::wprintf(L"[1/5] 분산은 %f이다.\n", variance);
        std::wprintf(L"[2/5] 표준편차는 %f이다.\n", stddev);

        double kurtosis = 0.0;
        for (int x = 0; x < inputGridSize - 2; x++)
        {
            for (int y = 0; y < inputGridSize - 2; y++)
            {
                for (int z = 0; z < inputGridSize - 2; z++)
                {
                    kurtosis += std::pow((laplacian(x, y, z) - mean)/ stddev, 4.0);
                }
            }
        }
        kurtosis /= (double)laplacian.size();
        kurtosis -= 3.0;
        std::wprintf(L"[3/5] 첨도는 %f이다.\n", kurtosis);

        double skewness = 0.0;
        for (int x = 0; x < inputGridSize - 2; x++)
        {
            for (int y = 0; y < inputGridSize - 2; y++)
            {
                for (int z = 0; z < inputGridSize - 2; z++)
                {
                    skewness += std::pow((laplacian(x, y, z) - mean) / stddev, 3.0);
                }
            }
        }
        skewness /= (double)laplacian.size();
        std::wprintf(L"[4/5] 왜도는 %f이다.\n", skewness);



        int number = 0;
        double cutoff = 4.2;
        for (int x = 0; x < inputGridSize - 2; x++)
        {
            for (int y = 0; y < inputGridSize - 2; y++)
            {
                for (int z = 0; z < inputGridSize - 2; z++)
                {
                    if (laplacian(x, y, z) > cutoff) number++;
                }
            }
        }
        std::wprintf(L"[5/5] 컷오프 %f를 넘은 원자의 숫자는 %d개이다.\n", cutoff, number);


        return { mean,variance,stddev,kurtosis,skewness,(double)number };
    }

    std::array<double, 6> getCurvData(double*** density, const int inputGridSize, const double inputBosSize)
    {
        double del = inputBosSize / (inputGridSize - 1);
        double delSquared = del * del;
        Eigen::Tensor<double, 3> laplacian(inputGridSize - 2, inputGridSize - 2, inputGridSize - 2);
        laplacian.setZero();
        for (int x = 1; x < inputGridSize - 1; x++)
        {
            for (int y = 1; y < inputGridSize - 1; y++)
            {
                for (int z = 1; z < inputGridSize - 1; z++)
                {
                    laplacian(x - 1, y - 1, z - 1) += (density[x + 1][y][z] - 2 * density[x][y][z] + density[x - 1][y][z]);
                    laplacian(x - 1, y - 1, z - 1) += (density[x][y + 1][z] - 2 * density[x][y][z] + density[x][y - 1][z]);
                    laplacian(x - 1, y - 1, z - 1) += (density[x][y][z + 1] - 2 * density[x][y][z] + density[x][y][z - 1]);
                    laplacian(x - 1, y - 1, z - 1) /= delSquared;
                }
            }
        }

        double variance = 0.0;
        double totalVal = 0.0;
        for (int x = 0; x < inputGridSize - 2; x++)
        {
            for (int y = 0; y < inputGridSize - 2; y++)
            {
                for (int z = 0; z < inputGridSize - 2; z++)
                {
                    totalVal += laplacian(x, y, z);
                }
            }
        }
        double mean = totalVal / (double)laplacian.size();
        for (int x = 0; x < inputGridSize - 2; x++)
        {
            for (int y = 0; y < inputGridSize - 2; y++)
            {
                for (int z = 0; z < inputGridSize - 2; z++)
                {
                    variance += std::pow(laplacian(x, y, z) - mean, 2.0);
                }
            }
        }
        variance /= (double)laplacian.size();
        double stddev = std::sqrt(variance);
        //std::wprintf(L"[1/5] 분산은 %f이다.\n", variance);
        //std::wprintf(L"[2/5] 표준편차는 %f이다.\n", stddev);

        double kurtosis = 0.0;
        for (int x = 0; x < inputGridSize - 2; x++)
        {
            for (int y = 0; y < inputGridSize - 2; y++)
            {
                for (int z = 0; z < inputGridSize - 2; z++)
                {
                    kurtosis += std::pow((laplacian(x, y, z) - mean) / stddev, 4.0);
                }
            }
        }
        kurtosis /= (double)laplacian.size();
        kurtosis -= 3.0;
        //std::wprintf(L"[3/5] 첨도는 %f이다.\n", kurtosis);

        double skewness = 0.0;
        for (int x = 0; x < inputGridSize - 2; x++)
        {
            for (int y = 0; y < inputGridSize - 2; y++)
            {
                for (int z = 0; z < inputGridSize - 2; z++)
                {
                    skewness += std::pow((laplacian(x, y, z) - mean) / stddev, 3.0);
                }
            }
        }
        skewness /= (double)laplacian.size();
        //std::wprintf(L"[4/5] 왜도는 %f이다.\n", skewness);



        int number = 0;
        double cutoff = 4.2;
        for (int x = 0; x < inputGridSize - 2; x++)
        {
            for (int y = 0; y < inputGridSize - 2; y++)
            {
                for (int z = 0; z < inputGridSize - 2; z++)
                {
                    if (laplacian(x, y, z) > cutoff) number++;
                }
            }
        }
        //std::wprintf(L"[5/5] 컷오프 %f를 넘은 원자의 숫자는 %d개이다.\n", cutoff, number);


        return { mean,variance,stddev,kurtosis,skewness,(double)number };
    }


    std::array<double, 6> getDenseData(double*** density, const int inputGridSize, const double inputBosSize)
    {
        double del = inputBosSize / (inputGridSize - 1);
        double delSquared = del * del;

        double variance = 0.0;
        double totalVal = 0.0;
        for (int x = 0; x < inputGridSize; x++)
        {
            for (int y = 0; y < inputGridSize; y++)
            {
                for (int z = 0; z < inputGridSize; z++)
                {
                    totalVal += density[x][y][z];
                }
            }
        }
        double mean = totalVal / (inputGridSize* inputGridSize* inputGridSize);
        for (int x = 0; x < inputGridSize; x++)
        {
            for (int y = 0; y < inputGridSize; y++)
            {
                for (int z = 0; z < inputGridSize; z++)
                {
                    variance += std::pow(density[x][y][z] - mean, 2.0);
                }
            }
        }
        variance /= (double)(inputGridSize * inputGridSize * inputGridSize);
        double stddev = std::sqrt(variance);
        std::wprintf(L"[1/5] 분산은 %f이다.\n", variance);
        std::wprintf(L"[2/5] 표준편차는 %f이다.\n", stddev);

        double kurtosis = 0.0;
        for (int x = 0; x < inputGridSize; x++)
        {
            for (int y = 0; y < inputGridSize; y++)
            {
                for (int z = 0; z < inputGridSize; z++)
                {
                    kurtosis += std::pow((density[x][y][z] - mean) / stddev, 4.0);
                }
            }
        }
        kurtosis /= (double)(inputGridSize * inputGridSize * inputGridSize);
        kurtosis -= 3.0;
        std::wprintf(L"[3/5] 첨도는 %f이다.\n", kurtosis);

        double skewness = 0.0;
        for (int x = 0; x < inputGridSize; x++)
        {
            for (int y = 0; y < inputGridSize; y++)
            {
                for (int z = 0; z < inputGridSize; z++)
                {
                    skewness += std::pow((density[x][y][z] - mean) / stddev, 3.0);
                }
            }
        }
        skewness /= (double)(inputGridSize * inputGridSize * inputGridSize);
        std::wprintf(L"[4/5] 왜도는 %f이다.\n", skewness);



        int number = 0;
        double cutoff = 2.5;
        for (int x = 0; x < inputGridSize; x++)
        {
            for (int y = 0; y < inputGridSize; y++)
            {
                for (int z = 0; z < inputGridSize; z++)
                {
                    if (density[x][y][z] > cutoff) number++;
                }
            }
        }
        std::wprintf(L"[5/5] 컷오프 %f를 넘은 원자의 숫자는 %d개이다.\n", cutoff, number);


        return { mean,variance,stddev,kurtosis,skewness,(double)number };
    }

    Eigen::Tensor<std::complex<double>, 3> convertToDensityFuncAndFFT2(const std::vector<Point>& inputPoints, double inputLatticeConstant, int inputGridSize)
    {
        double del = inputLatticeConstant / (inputGridSize - 1);
        int gridSize = inputGridSize * inputGridSize * inputGridSize;

        Eigen::Tensor<double, 3> density(inputGridSize, inputGridSize, inputGridSize);
        density = createDensityFunctionTensor(inputPoints, inputLatticeConstant, inputGridSize);
        Eigen::Tensor<double, 3> windowFunc = createBlackmanWindow(inputGridSize);
        density = density * windowFunc;
        std::wprintf(L"창함수를 곱하였다.\n");


        fftw_complex* input, * output;
        fftw_plan p;
        input = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * gridSize);
        output = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * gridSize);
        if (!input || !output)
        {
            std::wprintf(L"[FFT] 메모리 할당 실패\n");
            if (input) fftw_free(input);
            if (output) fftw_free(output);
            return {};
        }

        int idx = 0;
        for (int x = 0; x < inputGridSize; ++x)
        {
            for (int y = 0; y < inputGridSize; ++y)
            {
                for (int z = 0; z < inputGridSize; ++z)
                {
                    input[idx][0] = density(x, y, z);//실수
                    input[idx][1] = 0.0;//허수
                    ++idx;
                }
            }
        }

        std::wprintf(L"푸리에변환 입력값 구성이 끝났다.\n");


        p = fftw_plan_dft_3d(inputGridSize, inputGridSize, inputGridSize, input, output, FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(p);

        std::wprintf(L"푸리에변환 계산이 끝났다.\n");


        double delta_x = del;
        double delta_y = del;
        double delta_z = del;

        Eigen::Tensor<std::complex<double>, 3> fourierResult(inputGridSize, inputGridSize, inputGridSize);
        for (int x = 0; x < inputGridSize; ++x)
        {
            for (int y = 0; y < inputGridSize; ++y)
            {
                for (int z = 0; z < inputGridSize; ++z)
                {
                    int i = z * inputGridSize * inputGridSize + y * inputGridSize + x;
                    int kx = (x <= inputGridSize / 2) ? x : x - inputGridSize;
                    int ky = (y <= inputGridSize / 2) ? y : y - inputGridSize;
                    int kz = (z <= inputGridSize / 2) ? z : z - inputGridSize;

                    std::complex<double> complexVal(output[i][0], output[i][1]);
                    int convertX = kx + inputGridSize / 2 - 1;
                    int convertY = ky + inputGridSize / 2 - 1;
                    int convertZ = kz + inputGridSize / 2 - 1;
                    fourierResult(convertX, convertY, convertZ) = complexVal;
                }
            }
        }

        wprintf(L"푸리에변환한 결과값을 저장했다.\n");
        fftw_destroy_plan(p);
        fftw_free(input);
        fftw_free(output);
        return fourierResult;
    }

    void saveFourierRef()
    {
        if (latticeConstant == 0)
        {
            std::wprintf(L"[Error] 격자상수가 정의되지 않았다.\n");
            return;
        }

        __int64 startRotateTime = getNanoTimer();

        const int delDegree = 15;
        ThreadPool threadPool(std::thread::hardware_concurrency());
        std::mutex fourierAngleSaveMutex;
        std::mutex fourierTransSaveMutex;

        for (int xAngle = 0; xAngle < 360; xAngle += delDegree)
        {
            for (int yAngle = 0; yAngle < 360; yAngle += delDegree)
            {
                for (int zAngle = 0; zAngle < 360; zAngle += delDegree)
                {
                    
                    threadPool.addTask([this, xAngle, yAngle, zAngle,&fourierAngleSaveMutex]()
                        {
                            Eigen::Matrix3d inputRot = angleToMatrix(xAngle, yAngle, zAngle);
                            std::vector<Point> targetPoints = myPoints; // 백업
                            latticeRotation(targetPoints, latticeConstant, inputRot); // 결정구조 회전 후 나간 입자 제거 및 빈 공간은 다시 채움(주기조건)
                            std::vector<std::array<std::complex<double>, 4>> resultFFT = convertToDensityFuncAndFFT(targetPoints, latticeConstant, DENSITY_GRID);
                            {
                                std::unique_lock<std::mutex> lock(fourierAngleSaveMutex);  // 수정
                                fourierAngleSave.push_back({ {(double)xAngle, (double)yAngle, (double)zAngle}, resultFFT });
                            }
                        });
                }
            }
        }
        threadPool.waitForThreads();

        std::wprintf(L"Amplitude spectrum 추가 완료\n");
        {
            __int64 elapsedSeconds = (getNanoTimer() - startRotateTime) / 1e9;
            int hours = elapsedSeconds / 3600;
            int minutes = (elapsedSeconds % 3600) / 60;
            int seconds = elapsedSeconds % 60;
            std::wprintf(L"회전 저장에 걸린 시간 : %02d시간 %02d분 %02d초\n", hours, minutes, seconds);
        }

        __int64 startTransTime = getNanoTimer();

        double del = latticeConstant / 20;

        for (double dx = -latticeConstant / 2.0; dx < latticeConstant / 2.0; dx += del)
        {
            for (double dy = -latticeConstant / 2.0; dy < latticeConstant / 2.0; dy += del)
            {
                for (double dz = -latticeConstant / 2.0; dz < latticeConstant / 2.0; dz += del)
                {
                    threadPool.addTask([this, dx, dy, dz, &fourierTransSaveMutex]()
                        {
                            std::vector<Point> targetPoints = myPoints; // 백업
                            latticeTranslation(targetPoints, latticeConstant, { dx, dy, dz }); // 결정구조 평행이동 후 나간 입자 제거 및 빈 공간 다시 채움
                            std::vector<std::array<std::complex<double>, 4>> resultFFT = convertToDensityFuncAndFFT(targetPoints, latticeConstant, DENSITY_GRID);
                            {
                                std::unique_lock<std::mutex> lock(fourierTransSaveMutex);  // 수정
                                fourierTransSave.push_back({ {dx, dy, dz}, resultFFT });
                            }
                        });
                }
            }
        }

        threadPool.waitForThreads();



        std::wprintf(L"평행이동 아카이브 추가 완료\n");
        {
            __int64 elapsedSeconds = (getNanoTimer() - startTransTime) / 1e9;
            int hours = elapsedSeconds / 3600;
            int minutes = (elapsedSeconds % 3600) / 60;
            int seconds = elapsedSeconds % 60;
            std::wprintf(L"평행이동 아카이빙에 걸린 시간 : %02d시간 %02d분 %02d초\n", hours, minutes, seconds);
        }

        std::wprintf(L"새로운 Fourier reference가 성공적으로 추가되었다.\n");
        hasFourierRef = true;
    }

    void saveFourierRef2()
    {
        for (int i = 0; i < 2; i++)
        {
            const int RESOLUTION = 128;
            //double dx = 0;
            int xAngle = 0;
            if (i == 1)
            {
                xAngle = 140;
                //dx = 3;
            }
            std::wprintf(L"xAngle = %d\n", xAngle);
            Eigen::Matrix3d inputRot = angleToMatrix(xAngle, 0, 0);
            std::vector<Point> targetPoints = myPoints; // 백업
            latticeRotation(targetPoints, latticeConstant, inputRot); // 결정구조 회전 후 나간 입자 제거 및 빈 공간은 다시 채움(주기조건)
            //latticeTranslation(targetPoints, latticeConstant, { dx, 0, 0 });
            Eigen::Tensor<std::complex<double>, 3> resultFFT = convertToDensityFuncAndFFT2(targetPoints, latticeConstant, RESOLUTION);

            std::map<double, std::array<int, 3>> localPeakList;
            for (int x = 1; x < RESOLUTION - 1; x++)
            {
                for (int y = 1; y < RESOLUTION - 1; y++)
                {
                    for (int z = 1; z < RESOLUTION - 1; z++)
                    {
                        bool isLocalPeak = true;
                        double currentVal = std::abs(resultFFT(x, y, z));

                        for (int dx = -1; dx <= 1; dx++)
                        {
                            for (int dy = -1; dy <= 1; dy++)
                            {
                                for (int dz = -1; dz <= 1; dz++)
                                {
                                    if (dx == 0 && dy == 0 && dz == 0) continue;
                                    if (currentVal < std::abs(resultFFT(x + dx, y + dy, z + dz)))
                                    {
                                        isLocalPeak = false;
                                        break;
                                    }
                                }
                                if (!isLocalPeak) break;
                            }
                            if (!isLocalPeak) break;
                        }

                        if (isLocalPeak) localPeakList[currentVal] = { x - RESOLUTION / 2 + 1, y - RESOLUTION / 2 + 1, z - RESOLUTION / 2 + 1 };
                    }
                }
            }

            std::wprintf(L"Local peaks for xAngle = %d:\n", xAngle);
            int repeat = 0;
            for (auto it = localPeakList.rbegin(); it != localPeakList.rend(); ++it)
            {
                repeat++;
                const auto& peak = *it;
                if (repeat > 5) break;
                std::wprintf(L"%d : Peak at (%d, %d, %d) with magnitude %f\n", repeat, peak.second[0], peak.second[1], peak.second[2], peak.first);
            }
        }
        std::wprintf(L"새로운 Fourier reference가 성공적으로 추가되었다.\n");
        hasFourierRef = true;
    }

    double calcMSELossAmpFFT(const std::vector<std::array<std::complex<double>, 4>>& firstFFT, const std::vector<std::array<std::complex<double>, 4>>& secondFFT)
    {
        if (!hasFourierRef)
        {
            std::wprintf(L"[Error] 레퍼런스 FFT가 정의되지 않았다.\n");
            return -1.0;
        }

        double loss = 0.0;
        for (int i = 0; i < firstFFT.size(); ++i)
        {
            double ampRef = std::abs(secondFFT[i][3]);
            double ampTgt = std::abs(firstFFT[i][3]);
            loss += std::pow(ampRef - ampTgt, 2);
        }
        return loss / (double)firstFFT.size();
    }

    double calcMSELossPhaseFFT(const std::vector<std::array<std::complex<double>, 4>>& firstFFT, const std::vector<std::array<std::complex<double>, 4>>& secondFFT)
    {
        if (!hasFourierRef)
        {
            std::wprintf(L"[Error] 레퍼런스 FFT가 정의되지 않았다.\n");
            return -1.0;
        }

        double loss = 0.0;
        for (int i = 0; i < firstFFT.size(); ++i)
        {
            double phaseRef = std::arg(secondFFT[i][3]);
            double phaseTgt = std::arg(firstFFT[i][3]);
            loss += std::pow(phaseRef - phaseTgt, 2);
        }
        return loss / (double)firstFFT.size();
    }

    Eigen::Matrix3d getRotationByFFT()
    {
        __int64 startTimeStamp = getNanoTimer();
        if (hasFourierRef == false)
        {
            std::wprintf(L"[Error] 참조할 푸리에 변환 값이 존재하지 않는다.\n");
            return {};
        }

        double minLoss = std::numeric_limits<double>::max();
        Point minAngle = { 0, 0 };
        std::mutex mtx;

        std::vector<std::array<std::complex<double>, 4>> currentFFT = convertToDensityFuncAndFFT(myPoints, latticeConstant, DENSITY_GRID);
        int numThreads = std::thread::hardware_concurrency();
        int chunkSize = fourierAngleSave.size() / numThreads;
        std::vector<std::thread> threads;

        for (int t = 0; t < numThreads; ++t)
        {
            int start = t * chunkSize;
            int end = (t == numThreads - 1) ? fourierAngleSave.size() : start + chunkSize;
            threads.emplace_back([&currentFFT, &minLoss, &minAngle, &mtx, start, end,this]() {
                for (int i = start; i < end; ++i)
                {
                    std::vector<std::array<std::complex<double>, 4>> refFFT = fourierAngleSave[i].second;
                    double loss = calcMSELossAmpFFT(currentFFT, refFFT);
                    std::lock_guard<std::mutex> lock(mtx);
                    if (loss < minLoss)
                    {
                        minLoss = loss;
                        minAngle = fourierAngleSave[i].first;
                    }
                }
                });
        }

        for (auto& th : threads)
        {
            th.join();
        }

        std::wprintf(L"======================================================================================\n");
        std::wprintf(L"진폭 스펙트럼 분석이 완료되었다.\n");
        __int64 elapsedSeconds = (getNanoTimer() - startTimeStamp) / 1e9;
        int hours = elapsedSeconds / 3600;
        int minutes = (elapsedSeconds % 3600) / 60;
        int seconds = elapsedSeconds % 60;
        std::wprintf(L"걸린 시간 : %02d시간 %02d분 %02d초\n", hours, minutes, seconds);
        std::wprintf(L"최소 손실(loss = %f)을 가지는 회전행렬은 다음과 같다.\n", minLoss);
        double xRad = minAngle.x * DEGREE_TO_RADIAN; //x축 회전
        double yRad = minAngle.y * DEGREE_TO_RADIAN; //y축 회전
        double zRad = minAngle.z * DEGREE_TO_RADIAN; //z축 회전

        Eigen::Matrix3d rotX, rotY, rotZ, minRot;
        rotX << 1, 0, 0,
            0, cos(xRad), -sin(xRad),
            0, sin(xRad), cos(xRad);

        rotY << cos(yRad), 0, sin(yRad),
            0, 1, 0,
            -sin(yRad), 0, cos(yRad);

        rotZ << cos(zRad), -sin(zRad), 0,
            sin(zRad), cos(zRad), 0,
            0, 0, 1;

        minRot = rotZ * rotY * rotX;
        
        printRotationMatrix(minRot);
        return minRot;
    }

    Eigen::Vector3d getTranslationByFFT()
    {
        __int64 startTimeStamp = getNanoTimer();

        if (hasFourierRef == false)
        {
            std::wprintf(L"[Error] 참조할 푸리에 변환 값이 존재하지 않는다.\n");
            return {};
        }

        double minLoss = std::numeric_limits<double>::max();
        Eigen::Vector3d minVec;
        minVec << 0, 0, 0;
        std::mutex mtx;

        std::vector<std::array<std::complex<double>, 4>> currentFFT = convertToDensityFuncAndFFT(myPoints, latticeConstant, DENSITY_GRID);
        int numThreads = std::thread::hardware_concurrency();
        int chunkSize = fourierTransSave.size() / numThreads;
        std::vector<std::thread> threads;

        for (int t = 0; t < numThreads; ++t)
        {
            int start = t * chunkSize;
            int end = (t == numThreads - 1) ? fourierTransSave.size() : start + chunkSize;
            threads.emplace_back([&currentFFT, &minLoss, &minVec, &mtx, start, end,this]() {
                for (int i = start; i < end; ++i)
                {
                    std::vector<std::array<std::complex<double>, 4>> refFFT = fourierTransSave[i].second;
                    double loss = calcMSELossAmpFFT(currentFFT, refFFT);
                    std::lock_guard<std::mutex> lock(mtx);
                    if (loss < minLoss)
                    {
                        minLoss = loss;
                        minVec = { fourierTransSave[i].first.x, fourierTransSave[i].first.y, fourierTransSave[i].first.z };
                    }
                }
                });
        }

        for (auto& th : threads)
        {
            th.join();
        }

        std::wprintf(L"======================================================================================\n");
        std::wprintf(L"위상 스펙트럼 분석이 완료되었다.\n");
        __int64 elapsedSeconds = (getNanoTimer() - startTimeStamp) / 1e9;
        int hours = elapsedSeconds / 3600;
        int minutes = (elapsedSeconds % 3600) / 60;
        int seconds = elapsedSeconds % 60;
        std::wprintf(L"걸린 시간 : %02d시간 %02d분 %02d초\n", hours, minutes, seconds);
        std::wprintf(L"최소 손실(loss = %f)을 가지는 평행이동 벡터는 다음과 같다.\n", minLoss);
        std::cout << minVec << std::endl;
        return minVec;
    }

    Eigen::Tensor<double, 3> createHammingWindow(int gridSize)
    {
        Eigen::Tensor<double, 3> hammingWindow(gridSize, gridSize, gridSize);
        for (int i = 0; i < gridSize; ++i)
        {
            for (int j = 0; j < gridSize; ++j)
            {
                for (int k = 0; k < gridSize; ++k)
                {
                    double wx = 0.54 - 0.46 * std::cos(2 * M_PI * i / (gridSize - 1));
                    double wy = 0.54 - 0.46 * std::cos(2 * M_PI * j / (gridSize - 1));
                    double wz = 0.54 - 0.46 * std::cos(2 * M_PI * k / (gridSize - 1));
                    hammingWindow(i, j, k) = wx * wy * wz;
                }
            }
        }
        return hammingWindow;
    }

    Eigen::Tensor<double, 3> createBlackmanWindow(int gridSize)
    {
        Eigen::Tensor<double, 3> blackmanWindow(gridSize, gridSize, gridSize);
        for (int i = 0; i < gridSize; ++i)
        {
            for (int j = 0; j < gridSize; ++j)
            {
                for (int k = 0; k < gridSize; ++k)
                {
                    double wx = 0.42 - 0.5 * std::cos(2 * M_PI * i / (gridSize - 1)) + 0.08 * std::cos(4 * M_PI * i / (gridSize - 1));
                    double wy = 0.42 - 0.5 * std::cos(2 * M_PI * j / (gridSize - 1)) + 0.08 * std::cos(4 * M_PI * j / (gridSize - 1));
                    double wz = 0.42 - 0.5 * std::cos(2 * M_PI * k / (gridSize - 1)) + 0.08 * std::cos(4 * M_PI * k / (gridSize - 1));
                    blackmanWindow(i, j, k) = wx * wy * wz;
                }
            }
        }
        return blackmanWindow;
    }

    void invariablize()
    {
        if (hasFourierRef == false)
        {
            std::wprintf(L"[Error] 참조할 푸리에 변환 값이 존재하지 않는다.\n");
            return;
        }

        if (latticeConstant == 0)
        {
            std::wprintf(L"[Error] 격자 상수가 정의되어 있지 않다.\n");
            return;
        }

        latticeRotation(myPoints, latticeConstant, getRotationByFFT().inverse());
        latticeTranslation(myPoints, latticeConstant, -getTranslationByFFT());
    }
};


export std::vector<Triangle> getTrianglesFromScalar(const Eigen::Tensor<double, 3>& scalarField, double isoLevel, double tolerance)
{
    const int dimX = scalarField.dimension(0);
    const int dimY = scalarField.dimension(1);
    const int dimZ = scalarField.dimension(2);

    Eigen::MatrixXd GV(dimX * dimY * dimZ, 3);
    Eigen::VectorXd GS(dimX * dimY * dimZ);

    for (int z = 0; z < dimZ; z++) {
        for (int y = 0; y < dimY; y++) {
            for (int x = 0; x < dimX; x++) {
                int idx = x + y * dimX + z * dimX * dimY;
                GV.row(idx) << x, y, z;
                GS(idx) = scalarField(x, y, z);
            }
        }
    }

    for (int i = 0; i < GS.size(); i++) {
        if (std::abs(GS(i) - isoLevel) > tolerance) {
            GS(i) = (GS(i) > isoLevel) ? isoLevel + tolerance * 2 : isoLevel - tolerance * 2;
        }
    }

    Eigen::MatrixXd V;
    Eigen::MatrixXi F;
    igl::marching_cubes(GS, GV, dimX, dimY, dimZ, isoLevel, V, F);

    std::vector<Triangle> triangles;
    triangles.reserve(F.rows());

    for (int i = 0; i < F.rows(); i++) {
        Triangle tri;
        Eigen::Vector3d v1 = V.row(F(i, 0));
        Eigen::Vector3d v2 = V.row(F(i, 1));
        Eigen::Vector3d v3 = V.row(F(i, 2));

        tri.p1 = Point(v1.x(), v1.y(), v1.z());
        tri.p2 = Point(v2.x(), v2.y(), v2.z());
        tri.p3 = Point(v3.x(), v3.y(), v3.z());

        triangles.push_back(tri);
    }

    return triangles;
}

export std::vector<Triangle> getTrianglesFromScalar(const Eigen::Tensor<double, 3>& scalarField,double isoLevel) 
{
    const int dimX = scalarField.dimension(0);
    const int dimY = scalarField.dimension(1);
    const int dimZ = scalarField.dimension(2);

    Eigen::MatrixXd GV(dimX * dimY * dimZ, 3);
    Eigen::VectorXd GS(dimX * dimY * dimZ);

    for (int z = 0; z < dimZ; z++) {
        for (int y = 0; y < dimY; y++) {
            for (int x = 0; x < dimX; x++) {
                int idx = x + y * dimX + z * dimX * dimY;
                GV.row(idx) << x, y, z;
                GS(idx) = scalarField(x, y, z);
            }
        }
    }


    Eigen::MatrixXd V;
    Eigen::MatrixXi F;
    igl::marching_cubes(GS, GV, dimX, dimY, dimZ, isoLevel, V, F);

    std::vector<Triangle> triangles;
    triangles.reserve(F.rows());
    for (int i = 0; i < F.rows(); i++) {
        Triangle tri;
        Eigen::Vector3d v1 = V.row(F(i, 0));
        Eigen::Vector3d v2 = V.row(F(i, 1));
        Eigen::Vector3d v3 = V.row(F(i, 2));

        tri.p1 = Point(v1.x(), v1.y(), v1.z());
        tri.p2 = Point(v2.x(), v2.y(), v2.z());
        tri.p3 = Point(v3.x(), v3.y(), v3.z());

        triangles.push_back(tri);
    }

    return triangles;
}

export double getAreaFromTriangles(const std::vector<Triangle>& triangles)
{
    double totalArea = 0.0;
    for (const auto& triangle : triangles) totalArea += triangle.getArea();
    return totalArea;
}

struct PointHasher 
{
    std::size_t operator()(const Point& p) const 
    {
        return std::hash<double>()(p.x) ^ std::hash<double>()(p.y) ^ std::hash<double>()(p.z);
    }
};

export std::vector<double> getMeanCurvatures(const std::vector<Triangle>& inputTriangles) 
{
    std::unordered_map<Point, int, PointHasher> pointToIndex;
    std::vector<Point> indexToPoint;
    int index = 0;
    for (const auto& tri : inputTriangles) {
        if (pointToIndex.find(tri.p1) == pointToIndex.end()) {
            pointToIndex[tri.p1] = index++;
            indexToPoint.push_back(tri.p1);
        }
        if (pointToIndex.find(tri.p2) == pointToIndex.end()) {
            pointToIndex[tri.p2] = index++;
            indexToPoint.push_back(tri.p2);
        }
        if (pointToIndex.find(tri.p3) == pointToIndex.end()) {
            pointToIndex[tri.p3] = index++;
            indexToPoint.push_back(tri.p3);
        }
    }

    int numVertices = indexToPoint.size();
    std::vector<Eigen::Vector3d> meanCurvatureVectors(numVertices, Eigen::Vector3d::Zero());
    std::vector<double> vertexAreas(numVertices, 0.0);
    std::vector<std::vector<int>> vertexTriangles(numVertices);

    for (int i = 0; i < inputTriangles.size(); ++i) {
        const auto& tri = inputTriangles[i];
        int idx1 = pointToIndex[tri.p1];
        int idx2 = pointToIndex[tri.p2];
        int idx3 = pointToIndex[tri.p3];

        vertexTriangles[idx1].push_back(i);
        vertexTriangles[idx2].push_back(i);
        vertexTriangles[idx3].push_back(i);
    }

    std::vector<std::vector<std::pair<int, int>>> vertexEdges(numVertices);

    std::map<std::pair<int, int>, std::vector<int>> edgeToTriangles;
    for (int i = 0; i < inputTriangles.size(); ++i) 
    {
        const auto& tri = inputTriangles[i];
        int idx[3] = { pointToIndex[tri.p1], pointToIndex[tri.p2], pointToIndex[tri.p3] };

        for (int j = 0; j < 3; ++j) {
            int idx1 = idx[j];
            int idx2 = idx[(j + 1) % 3];
            if (idx1 > idx2) std::swap(idx1, idx2);
            edgeToTriangles[{idx1, idx2}].push_back(i);
        }
    }

    for (const auto& edgePair : edgeToTriangles) 
    {
        int idx1 = edgePair.first.first;
        int idx2 = edgePair.first.second;
        const std::vector<int>& adjacentTriangles = edgePair.second;

        Eigen::Vector3d vi = indexToPoint[idx1].toVector();
        Eigen::Vector3d vj = indexToPoint[idx2].toVector();

        double cotAlpha = 0.0, cotBeta = 0.0;

        if (adjacentTriangles.size() == 2) {
            const Triangle& tri1 = inputTriangles[adjacentTriangles[0]];
            const Triangle& tri2 = inputTriangles[adjacentTriangles[1]];

            int idx3_1 = -1, idx3_2 = -1;
            for (int idx : { pointToIndex[tri1.p1], pointToIndex[tri1.p2], pointToIndex[tri1.p3] }) 
            {
                if (idx != idx1 && idx != idx2) 
                {
                    idx3_1 = idx;
                    break;
                }
            }
            for (int idx : { pointToIndex[tri2.p1], pointToIndex[tri2.p2], pointToIndex[tri2.p3] }) 
            {
                if (idx != idx1 && idx != idx2) 
                {
                    idx3_2 = idx;
                    break;
                }
            }

            Eigen::Vector3d vk = indexToPoint[idx3_1].toVector();
            Eigen::Vector3d vl = indexToPoint[idx3_2].toVector();

            Eigen::Vector3d e_ij = vi - vj;
            Eigen::Vector3d e_ik = vi - vk;
            Eigen::Vector3d e_jk = vj - vk;
            Eigen::Vector3d e_il = vi - vl;
            Eigen::Vector3d e_jl = vj - vl;

            double angleOpposite_Alpha = std::acos(e_jk.dot(-e_ik) / (e_jk.norm() * e_ik.norm()));
            double angleOpposite_Beta = std::acos(e_jl.dot(-e_il) / (e_jl.norm() * e_il.norm()));

            cotAlpha = 1.0 / std::tan(angleOpposite_Alpha);
            cotBeta = 1.0 / std::tan(angleOpposite_Beta);
        }
        else if (adjacentTriangles.size() == 1) 
        {
            const Triangle& tri = inputTriangles[adjacentTriangles[0]];
            int idx3 = -1;
            for (int idx : { pointToIndex[tri.p1], pointToIndex[tri.p2], pointToIndex[tri.p3] }) 
            {
                if (idx != idx1 && idx != idx2) 
                {
                    idx3 = idx;
                    break;
                }
            }

            Eigen::Vector3d vk = indexToPoint[idx3].toVector();

            Eigen::Vector3d e_ij = vi - vj;
            Eigen::Vector3d e_ik = vi - vk;
            Eigen::Vector3d e_jk = vj - vk;

            double angleOpposite = std::acos(e_jk.dot(-e_ik) / (e_jk.norm() * e_ik.norm()));
            cotAlpha = 1.0 / std::tan(angleOpposite);
            cotBeta = 0.0;
        }
        else continue;

        double weight = cotAlpha + cotBeta;
        Eigen::Vector3d contribution = weight * (vi - vj);

        meanCurvatureVectors[idx1] += contribution;
        meanCurvatureVectors[idx2] -= contribution;


        double area = 0.0;
        for (int triIdx : adjacentTriangles) area += inputTriangles[triIdx].getArea();
        area /= 3.0;

        vertexAreas[idx1] += area / 2.0;
        vertexAreas[idx2] += area / 2.0;
    }

    std::vector<double> meanCurvatures(numVertices);
    for (int i = 0; i < numVertices; ++i) 
    {
        double Ai = vertexAreas[i];
        if (Ai > 0) 
        {
            Eigen::Vector3d Hi = (1.0 / (2.0 * Ai)) * meanCurvatureVectors[i];
            meanCurvatures[i] = Hi.norm();
        }
        else meanCurvatures[i] = 0.0;
    }

    return meanCurvatures;
}

export void calculateAndAnalyzeCurvature(const std::vector<Triangle>& triangles) 
{
    std::vector<double> curvatures = getMeanCurvatures(triangles);

    double mean = std::accumulate(curvatures.begin(), curvatures.end(), 0.0) / curvatures.size();
    double variance = 0.0;
    for (double c : curvatures) {
        variance += (c - mean) * (c - mean);
    }
    variance /= curvatures.size();
    double stdDev = std::sqrt(variance);

    std::cout << "Mean Curvature: " << mean << "\n";
    std::cout << "Standard Deviation: " << stdDev << "\n";
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Hash functions
struct PointHash {
    std::size_t operator()(const Point& p) const {
        return std::hash<double>()(p.x) ^ std::hash<double>()(p.y) ^ std::hash<double>()(p.z);
    }
};


struct EdgeHash {
    std::size_t operator()(const std::pair<Point, Point>& edge) const {
        std::size_t h1 = PointHash()(edge.first);
        std::size_t h2 = PointHash()(edge.second);
        return h1 ^ (h2 << 1);
    }
};

// Normalize edges
std::pair<Point, Point> normalizeEdge(const Point& p1, const Point& p2) {
    return (p1 < p2) ? std::make_pair(p1, p2) : std::make_pair(p2, p1);
}

export Eigen::Tensor<double, 3>  arrayToTensor(double*** density, int resol)
{
    Eigen::Tensor<double, 3> densityTensor(resol, resol, resol);
    for (int i = 0; i < resol; ++i)
    {
        for (int j = 0; j < resol; ++j)
        {
            for (int k = 0; k < resol; ++k)
            {
                densityTensor(i, j, k) = density[i][j][k];
            }
        }
    }
    return densityTensor;
}

export std::pair<double, double> getLargestSurfaces(const std::vector<Triangle>& triangles) {
    // Build adjacency list inside the function
    std::unordered_map<std::pair<Point, Point>, std::vector<int>, EdgeHash> edgeMap;

    for (size_t i = 0; i < triangles.size(); ++i) {
        const auto& t = triangles[i];
        std::vector<std::pair<Point, Point>> edges = {
            normalizeEdge(t.p1, t.p2),
            normalizeEdge(t.p2, t.p3),
            normalizeEdge(t.p3, t.p1)
        };

        for (const auto& edge : edges) {
            edgeMap[edge].push_back(i);
        }
    }

    std::vector<std::vector<int>> adjacencyList(triangles.size());
    for (const auto& edgeMapEntry : edgeMap) {
        const auto& edge = edgeMapEntry.first;
        const auto& indices = edgeMapEntry.second;

        for (size_t i = 0; i < indices.size(); ++i) {
            for (size_t j = i + 1; j < indices.size(); ++j) {
                adjacencyList[indices[i]].push_back(indices[j]);
                adjacencyList[indices[j]].push_back(indices[i]);
            }
        }
    }

    // Calculate largest surfaces using BFS
    std::vector<bool> visited(triangles.size(), false);
    std::vector<double> clusterAreas;

    for (size_t i = 0; i < triangles.size(); ++i) {
        if (visited[i]) continue;

        std::queue<int> q;
        q.push(i);
        visited[i] = true;

        double clusterArea = 0.0;
        while (!q.empty()) {
            int current = q.front();
            q.pop();

            clusterArea += triangles[current].getArea();

            for (int neighbor : adjacencyList[current]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }

        clusterAreas.push_back(clusterArea);
    }

    // Find the two largest areas
    double largest = 0.0, secondLargest = 0.0;
    for (double area : clusterAreas) {
        if (area > largest) {
            secondLargest = largest;
            largest = area;
        }
        else if (area > secondLargest) {
            secondLargest = area;
        }
    }

    return { largest, secondLargest };
}