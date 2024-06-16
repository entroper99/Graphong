#define _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS
#include <SDL.h>
#include <Eigen/Dense>

export module Func;

import std;
import globalVar;
import constVar;
import Shapes;
import randomRange;

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

    inline static bool hasTransform = false;
    inline static Eigen::Vector3d transVec;
    inline static Eigen::Matrix3d rotMat;

    double period = 0.0;

    std::function<double(double, double, double)> scalarFunc = [](double x, double y, double z)->double { return 0.0; };

    Func(funcFlag inputType) : funcType(inputType)
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
            double triSize = 100.0;
            superTriangle.p1 = { avgX, 0, avgZ - triSize * distZ };
            superTriangle.p2 = { avgX - triSize * distX, 0, avgZ + triSize * distZ };
            superTriangle.p3 = { avgX + triSize * distX, 0, avgZ + triSize * distZ };
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

    void normalize(double inputPeriod)
    {
        std::wprintf(L"표준화 실행 전의 평균 f값은 %f이다.\n", scalarAvg());

        //고유 스칼라 함수 변경
        period = inputPeriod;
        double scaleFactor = 2.0 * M_PI / period;
        scalarFunc = [=](double x, double y, double z)->double
            {
                return (std::cos(scaleFactor * x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor * z)) + std::cos(scaleFactor * y) * std::sin(scaleFactor * z) * std::sin(2 * (scaleFactor * x)) + std::cos(scaleFactor * z) * std::sin(scaleFactor * x) * std::sin(2 * (scaleFactor * y)));
            };


        if (hasTransform == false)
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


            Eigen::MatrixXd matX;
            matX.resize(myPoints.size(), 3);

            for (int row = 0; row < myPoints.size(); row++)
            {
                matX(row, 0) = myPoints[row].x;
                matX(row, 1) = myPoints[row].y;
                matX(row, 2) = myPoints[row].z;
            }

            Eigen::MatrixXd matXT = matX.transpose();
            Eigen::MatrixXd matCov = (1.0 / (double)myPoints.size()) * (matXT * matX);

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
            originMat << -0.743409, -0.368947, -0.557872,
                0.242377, 0.62879, -0.738835,
                0.623375, -0.684472, -0.378024;

            Eigen::Matrix3d rotationMat = originMat * tgtMat.inverse();

            hasTransform = true;
            transVec = { -centerX,-centerY,-centerZ };
            rotMat = rotationMat;
        }

        std::cout << "\n평행이동 벡터" << ":\n" << transVec << "\n\n";
        std::cout << "\n회전행렬 " << ":\n" << rotMat << "\n\n";
        {
            Eigen::Matrix3d checkMat = rotMat;
            double trace = checkMat.trace();
            double theta = std::acos((trace - 1) / 2);
            Eigen::Vector3d axis;
            axis << checkMat(2, 1) - checkMat(1, 2),
                checkMat(0, 2) - checkMat(2, 0),
                checkMat(1, 0) - checkMat(0, 1);
            axis.normalize();
            std::cout << "회전각 : " << theta * 180.0 / M_PI << std::endl;
            std::cout << "회전축: (" << axis.x() << ", " << axis.y() << ", " << axis.z() << ")" << std::endl;
        }

        rotation(rotMat);

        scalarCalc();
        std::wprintf(L"표준화를 완료하였다.\n");
        std::wprintf(L"\033[0;33m이 함수의 표준화 후 평균 f값은 %f이다.\033[0m\n", scalarAvg());
    }


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
        //double totalVal = 0;
        //for (int i = 0; i < myPoints.size(); i++)
        //{
        //    totalVal += scalar[{myPoints[i].x, myPoints[i].y, myPoints[i].z}];
        //}

        //return totalVal / (double)myPoints.size();

        double totalVal = 0;
        for (int i = 0; i < myPoints.size(); i++)
        {
            double val = scalar[{myPoints[i].x, myPoints[i].y, myPoints[i].z}];
            totalVal += val * val;
        }

        return std::sqrt(totalVal);
    }

    double scalarL2Norm()
    {
        double totalVal = 0;
        for (int i = 0; i < myPoints.size(); i++)
        {
            double val = scalar[{myPoints[i].x, myPoints[i].y, myPoints[i].z}];
            totalVal += val * val;
        }

        return std::sqrt(totalVal);
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
};



