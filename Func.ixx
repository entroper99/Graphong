#define _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS
#include <SDL.h>
#include <Eigen/Dense>


export module Func;

import std;
import globalVar;
import constVar;
import Shapes;
import randomRange;

auto pointHash = [](const Point& p) -> std::size_t
    {
        std::size_t hx = std::hash<double>()(p.x);
        std::size_t hy = std::hash<double>()(p.y);
        std::size_t hz = std::hash<double>()(p.z);
        return hx ^ (hy << 1) ^ (hz << 2);
    };

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

    void sym()
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

        double rcs = 1.0; // ����ġ �Լ��� ������ �ƿ��� �Ķ����
        double rc = 10.0;  // �ƿ��� �Ÿ�

        std::vector<Point> newPoints;
        for (const auto& point : myPoints)
        {
            //�߽������κ����� ���Ÿ�
            double xji = point.x - centerX;
            double yji = point.y - centerY;
            double zji = point.z - centerZ;
            double rji = std::sqrt(xji * xji + yji * yji + zji * zji);

            // ����ġ �Լ� ���
            double s_rji;
            if (rji < rcs)  s_rji = 1.0 / rji;
            else if (rji < rc) s_rji = 1.0 / rji * (0.5 * cos(M_PI * (rji - rcs) / (rc - rcs)) + 0.5);
            else  s_rji = 0.0;

            // ��Ī ��ȯ�� ��ǥ ��� (�ϴ��� �߽������� ���Ÿ��� ����Ͽ���)
            double newX =/* centerX + */s_rji * xji;
            double newY = /*centerY + */s_rji * yji;
            double newZ = /*centerZ + */s_rji * zji;

            newPoints.push_back({ newX, newY, newZ });
        }

        myPoints = newPoints;

        std::wprintf(L"ǥ��ȭ�� �Ϸ��Ͽ���.\n");
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
        double totalVal = 0;
        for (int i = 0; i < myPoints.size(); i++)
        {
            totalVal += scalar[{myPoints[i].x, myPoints[i].y, myPoints[i].z}];
        }

        return totalVal / (double)myPoints.size();
    }
};



