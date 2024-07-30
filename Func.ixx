#define _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS
#include <SDL.h>
#include <Eigen/Dense>
#include <Eigen/Core>
#include <unsupported/Eigen/CXX11/Tensor>
#include <fftw3.h>

export module Func;

import std;
import globalVar;
import constVar;
import Shapes;
import randomRange;
import utilMath;
import nanoTimer;
import ThreadPool;


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

    //�ﰢ���� ���� ����
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
            std::wprintf(L"����Ʈ���̾ޱ��� �����Ͽ���..\n");
        }

        if (unselectPts.size() == 0)
        {
            if (deleteSupertri == false)
            {
                std::wprintf(L"����Ʈ���̾ޱ� ���� �ﰢ���� �����Ͽ���.\n");
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
                std::wprintf(L"�ﰢ������ �Ϸ��Ͽ���.\n");
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

        std::wprintf(L"[���� ��ŸƮ] ���õ� ��: (%f, %f, %f)\n", tgtPt.x, tgtPt.y, tgtPt.z);

        selectPts.push_back(tgtPt);
        unselectPts.erase(std::find(unselectPts.begin(), unselectPts.end(), tgtPt));

        for (auto tri = checkTriangles.begin(); tri != checkTriangles.end();)
        {
            bool found = false;
            //std::wprintf(L"�˻� ��� �ﰢ��: (%f,%f,%f)-(%f,%f,%f)-(%f,%f,%f)\n", tri->p1.x, tri->p1.y, tri->p1.z, tri->p2.x, tri->p2.y, tri->p2.z, tri->p3.x, tri->p3.y, tri->p3.z);
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
                    //else std::wprintf(L"�̹� �����ϴ� badTriangle�̴�!\n");
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

        //std::wprintf(L"���� �ﰢ�� ����: %d\n", badTriangles.size());
        //std::wprintf(L"���� �� ����: %d\n", badPts.size());
        //std::wprintf(L"�俧�� ���� (�ߺ� ���� ��): %d\n", edges.size());

        //for (int i = 0; i < edges.size(); i++)
        //{
        //    std::wprintf(L"%d�� ���� : (%f,%f,%f)-(%f,%f,%f) \n", i, edges[i].p1.x, edges[i].p1.y, edges[i].p1.z, edges[i].p2.x, edges[i].p2.y, edges[i].p2.z);
        //}


        // �ٰ��� �ֺ� ������ ������ ��ġ�� ������ ����
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

        //std::wprintf(L"�俧�� ���� (�ߺ� ���� ��): %d\n", edges.size());
        //for (int i = 0; i < edges.size(); i++) std::wprintf(L"%d�� ���� : (%f,%f,%f)-(%f,%f,%f) \n", i, edges[i].p1.x, edges[i].p1.y, edges[i].p1.z, edges[i].p2.x, edges[i].p2.y, edges[i].p2.z);

        // ������� ����
        for (int i = 0; i < badPts.size(); i++)
        {
            edges.push_back({ tgtPt, badPts[i] });
        }

        //std::wprintf(L"���� ���� (������� ���� �߰� ��): %d\n", edges.size());
        //for (int i = 0; i < edges.size(); i++) std::wprintf(L"%d�� ���� : (%f,%f,%f)-(%f,%f,%f) \n", i, edges[i].p1.x, edges[i].p1.y, edges[i].p1.z, edges[i].p2.x, edges[i].p2.y, edges[i].p2.z);

        auto pointHash = [](const Point& p)
            {
                std::hash<double> hasher;
                return ((hasher(p.x) * 73856093) ^ (hasher(p.y) * 19349663) ^ (hasher(p.z) * 83492791));
            };

        std::unordered_map<Point, std::vector<Point>, decltype(pointHash)> edgeMap(0, pointHash);

        // ���� �� ����
        for (const auto& edge : edges) {
            edgeMap[edge.p1].push_back(edge.p2);
            edgeMap[edge.p2].push_back(edge.p1);
        }

        // ���ο� �ﰢ�� ����
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
                                            //std::wprintf(L"���ο� �ﰢ��: (%f,%f,%f)-(%f,%f,%f)-(%f,%f,%f)\n", newTri.p1.x, newTri.p1.y, newTri.p1.z, newTri.p2.x, newTri.p2.y, newTri.p2.z, newTri.p3.x, newTri.p3.y, newTri.p3.z);
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
        std::wprintf(L"���ο� �˻� �ﰢ�� ����: %d\n\n", checkTriangles.size());
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
            std::wprintf(L"���ڻ���� ���ǵǾ����� �ʴ�.");
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
        std::wprintf(L"COM�� �̿��Ͽ� �߾������� �����մϴ�.");
        if (hasTranslation == false)
        {
            //�߽��� ���
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


            //�����͸� ������ �������� ����
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

        std::cout << "\n�����̵� ����" << ":\n" << transVec << "\n\n";
        //std::wprintf(L"\033[0;33m�� �Լ��� ��� f���� %f�̴�.\033[0m\n", scalarSquareAvg());
    }

    void sortByPCA()
    {
        std::wprintf(L"PCA�� �̿��Ͽ� ȸ�������� �����մϴ�.");
        if (latticeConstant == 0)
        {
            std::wprintf(L"[Error] ���ڻ���� ���ǵ��� ���� ���¿��� PCA�� �����ߴ�.\n");
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
                    std::wprintf(L"�� ������ �� (%f,%f,%f)�� �����̳ʿ� �־���.\n",myPoints[i].x, myPoints[i].y, myPoints[i].z);
                }
            }
            std::wprintf(L"�� ������ ������ �� %d���̴�.\n", testPoints.size());


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
        //std::wprintf(L"\033[0;33m�� �Լ��� ��� f���� %f�̴�.\033[0m\n", scalarSquareAvg());
    }

    std::vector<std::array<std::complex<double>, 4>> convertToDensityFuncAndFFT(const std::vector<Point>& inputPoints, double inputLatticeConstant, int inputGridSize)
    {
        double gaussAmp = 1.0;
        double gaussSig = 1.0;
        std::vector<std::array<double, 4>> densityFunc;
        double del = inputLatticeConstant / (inputGridSize - 1);
        int gridSize = inputGridSize * inputGridSize * inputGridSize;

        Eigen::Tensor<double, 3> density(inputGridSize, inputGridSize, inputGridSize);
        density.setZero();

        int idx = 0;
        for (double tgtX = -inputLatticeConstant / 2.0; tgtX <= inputLatticeConstant / 2.0; tgtX += del)
        {
            for (double tgtY = -inputLatticeConstant / 2.0; tgtY <= inputLatticeConstant / 2.0; tgtY += del)
            {
                for (double tgtZ = -inputLatticeConstant / 2.0; tgtZ <= inputLatticeConstant / 2.0; tgtZ += del)
                {
                    double densityValue = 0;
                    for (const auto& point : inputPoints)
                    {
                        densityValue += calcGaussian(point.x - tgtX, point.y - tgtY, point.z - tgtZ, gaussSig, gaussAmp);
                    }
                    int xIdx = std::round((tgtX + inputLatticeConstant / 2.0) / del);
                    int yIdx = std::round((tgtY + inputLatticeConstant / 2.0) / del);
                    int zIdx = std::round((tgtZ + inputLatticeConstant / 2.0) / del);
                    density(xIdx, yIdx, zIdx) = densityValue;
                }
            }
        }

        Eigen::Tensor<double, 3> windowFunc = createHammingWindow(inputGridSize);
        density = density * windowFunc;

        fftw_complex* input, * output;
        fftw_plan p;
        input = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * gridSize);
        output = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * gridSize);
        if (!input || !output)
        {
            std::wprintf(L"[FFT] �޸� �Ҵ� ����\n");
            if (input) fftw_free(input);
            if (output) fftw_free(output);
            return {};
        }

        idx = 0;
        for (int x = 0; x < inputGridSize; ++x)
        {
            for (int y = 0; y < inputGridSize; ++y)
            {
                for (int z = 0; z < inputGridSize; ++z)
                {
                    input[idx][0] = density(x, y, z);//�Ǽ�
                    input[idx][1] = 0.0;//���
                    ++idx;
                }
            }
        }

        p = fftw_plan_dft_3d(inputGridSize, inputGridSize, inputGridSize, input, output, FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(p);

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

        fftw_destroy_plan(p);
        fftw_free(input);
        fftw_free(output);
        return fourierResult;
    }

    void saveFourierRef()
    {
        if (latticeConstant == 0)
        {
            std::wprintf(L"[Error] ���ڻ���� ���ǵ��� �ʾҴ�.\n");
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
                            std::vector<Point> targetPoints = myPoints; // ���
                            latticeRotation(targetPoints, latticeConstant, inputRot); // �������� ȸ�� �� ���� ���� ���� �� �� ������ �ٽ� ä��(�ֱ�����)
                            std::vector<std::array<std::complex<double>, 4>> resultFFT = convertToDensityFuncAndFFT(targetPoints, latticeConstant, DENSITY_GRID);
                            {
                                std::unique_lock<std::mutex> lock(fourierAngleSaveMutex);  // ����
                                fourierAngleSave.push_back({ {(double)xAngle, (double)yAngle, (double)zAngle}, resultFFT });
                            }
                        });
                }
            }
        }
        threadPool.waitForThreads();

        std::wprintf(L"Amplitude spectrum �߰� �Ϸ�\n");
        {
            __int64 elapsedSeconds = (getNanoTimer() - startRotateTime) / 1e9;
            int hours = elapsedSeconds / 3600;
            int minutes = (elapsedSeconds % 3600) / 60;
            int seconds = elapsedSeconds % 60;
            std::wprintf(L"ȸ�� ���忡 �ɸ� �ð� : %02d�ð� %02d�� %02d��\n", hours, minutes, seconds);
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
                            std::vector<Point> targetPoints = myPoints; // ���
                            latticeTranslation(targetPoints, latticeConstant, { dx, dy, dz }); // �������� �����̵� �� ���� ���� ���� �� �� ���� �ٽ� ä��
                            std::vector<std::array<std::complex<double>, 4>> resultFFT = convertToDensityFuncAndFFT(targetPoints, latticeConstant, DENSITY_GRID);
                            {
                                std::unique_lock<std::mutex> lock(fourierTransSaveMutex);  // ����
                                fourierTransSave.push_back({ {dx, dy, dz}, resultFFT });
                            }
                        });
                }
            }
        }

        threadPool.waitForThreads();

        std::wprintf(L"�����̵� ��ī�̺� �߰� �Ϸ�\n");
        {
            __int64 elapsedSeconds = (getNanoTimer() - startTransTime) / 1e9;
            int hours = elapsedSeconds / 3600;
            int minutes = (elapsedSeconds % 3600) / 60;
            int seconds = elapsedSeconds % 60;
            std::wprintf(L"�����̵� ��ī�̺��� �ɸ� �ð� : %02d�ð� %02d�� %02d��\n", hours, minutes, seconds);
        }

        std::wprintf(L"���ο� Fourier reference�� ���������� �߰��Ǿ���.\n");
        hasFourierRef = true;
    }


    double calcMSELossAmpFFT(const std::vector<std::array<std::complex<double>, 4>>& firstFFT, const std::vector<std::array<std::complex<double>, 4>>& secondFFT)
    {
        if (!hasFourierRef)
        {
            std::wprintf(L"[Error] ���۷��� FFT�� ���ǵ��� �ʾҴ�.\n");
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
            std::wprintf(L"[Error] ���۷��� FFT�� ���ǵ��� �ʾҴ�.\n");
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
            std::wprintf(L"[Error] ������ Ǫ���� ��ȯ ���� �������� �ʴ´�.\n");
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
        std::wprintf(L"���� ����Ʈ�� �м��� �Ϸ�Ǿ���.\n");
        __int64 elapsedSeconds = (getNanoTimer() - startTimeStamp) / 1e9;
        int hours = elapsedSeconds / 3600;
        int minutes = (elapsedSeconds % 3600) / 60;
        int seconds = elapsedSeconds % 60;
        std::wprintf(L"�ɸ� �ð� : %02d�ð� %02d�� %02d��\n", hours, minutes, seconds);
        std::wprintf(L"�ּ� �ս�(loss = %f)�� ������ ȸ������� ������ ����.\n", minLoss);
        double xRad = minAngle.x * DEGREE_TO_RADIAN; //x�� ȸ��
        double yRad = minAngle.y * DEGREE_TO_RADIAN; //y�� ȸ��
        double zRad = minAngle.z * DEGREE_TO_RADIAN; //z�� ȸ��

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
            std::wprintf(L"[Error] ������ Ǫ���� ��ȯ ���� �������� �ʴ´�.\n");
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
        std::wprintf(L"���� ����Ʈ�� �м��� �Ϸ�Ǿ���.\n");
        __int64 elapsedSeconds = (getNanoTimer() - startTimeStamp) / 1e9;
        int hours = elapsedSeconds / 3600;
        int minutes = (elapsedSeconds % 3600) / 60;
        int seconds = elapsedSeconds % 60;
        std::wprintf(L"�ɸ� �ð� : %02d�ð� %02d�� %02d��\n", hours, minutes, seconds);
        std::wprintf(L"�ּ� �ս�(loss = %f)�� ������ �����̵� ���ʹ� ������ ����.\n", minLoss);
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

    void invariablize()
    {
        if (hasFourierRef == false)
        {
            std::wprintf(L"[Error] ������ Ǫ���� ��ȯ ���� �������� �ʴ´�.\n");
            return;
        }

        if (latticeConstant == 0)
        {
            std::wprintf(L"[Error] ���� ����� ���ǵǾ� ���� �ʴ�.\n");
            return;
        }

        latticeRotation(myPoints, latticeConstant, getRotationByFFT().inverse());
        latticeTranslation(myPoints, latticeConstant, -getTranslationByFFT());
    }

};





