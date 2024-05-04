#define _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS
#include <Eigen/Dense>

export module cubicSpline;

import std;
import globalVar;

export void cubicSpline(int dataIndex, int newPointNum)
{

    //int bcInput = -1;
    //std::wprintf(L"Cubic 스플라인 보간의 boundary condition을 설정해주세요.\n");
    //std::wprintf(L"1. Nartural \n");
    //std::wprintf(L"2. Clamped \n");
    //std::cin >> bcInput;

    std::wprintf(L"데이터셋 x 오름차순 재배열 완료.\n");
    std::sort(funcSet[dataIndex]->myPoints.begin(), funcSet[dataIndex]->myPoints.end(), [](const auto& a, const auto& b) { return a.x < b.x; });

    funcSet[dataIndex]->myInterPoints.clear();

    if (1/*bcInput == 1*/)
    {
        int dataSize = funcSet[dataIndex]->myPoints.size();

        Eigen::MatrixXd A = Eigen::MatrixXd::Zero(dataSize, dataSize);
        for (int i = 0; i < dataSize; i++)
        {
            if (i == 0)
            {
                A(0, 0) = 2.0;
                A(1, 0) = 1.0;
            }
            else if (i == dataSize - 1)
            {
                A(dataSize - 1, dataSize - 1) = 2.0;
                A(dataSize - 1 - 1, dataSize - 1) = 1.0;
            }
            else
            {
                A(i, i) = 4.0;
                A(i - 1, i) = 1.0;
                A(i + 1, i) = 1.0;
            }
        }
        std::cout << A << std::endl;

        Eigen::VectorXd b = Eigen::VectorXd::Zero(dataSize);
        for (int i = 0; i < dataSize; i++)
        {
            if (i == 0)
            {
                b(i) = 3.0 * (funcSet[dataIndex]->myPoints[1].y - funcSet[dataIndex]->myPoints[0].y);
            }
            else if (i == dataSize - 1)
            {
                b(i) = 3.0 * (funcSet[dataIndex]->myPoints[i].y - funcSet[dataIndex]->myPoints[i - 1].y);
            }
            else
            {
                b(i) = 3.0 * (funcSet[dataIndex]->myPoints[i + 1].y - funcSet[dataIndex]->myPoints[i - 1].y);
            }
        }
        std::cout << b << std::endl;


        Eigen::VectorXd x;
        x = A.lu().solve(b);
        std::wprintf(L"해는 다음과 같다.\n");
        std::cout << x << std::endl;

        for (int i = 0; i < dataSize - 1; i++)
        {
            float delLargeX = funcSet[dataIndex]->myPoints[i + 1].x - funcSet[dataIndex]->myPoints[i].x;
            float a = (2.0 * (funcSet[dataIndex]->myPoints[i].y - funcSet[dataIndex]->myPoints[i + 1].y)) / pow(1, 3.0) + (x[i + 1] + x[i]) / pow(1, 2.0);
            float b = (3.0 * (funcSet[dataIndex]->myPoints[i + 1].y - funcSet[dataIndex]->myPoints[i].y)) / pow(1, 2.0) - 2.0 * x[i] / pow(1, 1.0) - x[i + 1] / pow(1, 1.0);
            float c = x[i];
            float d = funcSet[dataIndex]->myPoints[i].y;

            std::wprintf(L"[%d]번째 Cubic 보간함수의 계수는 (%f,%f,%f,%f)이다.\n", i, a, b, c, d);

            float del = (funcSet[dataIndex]->myPoints[i + 1].x - funcSet[dataIndex]->myPoints[i].x) / ((float)newPointNum + 1.0);

            funcSet[dataIndex]->myInterPoints.push_back({ funcSet[dataIndex]->myPoints[i].x,funcSet[dataIndex]->myPoints[i].y,0 });
            for (int j = 0; j < newPointNum; j++)
            {
                float newX = funcSet[dataIndex]->myPoints[i].x + ((float)j + 1.0) * del;
                float delX = (newX - funcSet[dataIndex]->myPoints[i].x) / (funcSet[dataIndex]->myPoints[i + 1].x - funcSet[dataIndex]->myPoints[i].x);
                float newY = a * pow(delX, 3.0) + b * pow(delX, 2.0) + c * pow(delX, 1.0) + d;
                funcSet[dataIndex]->myInterPoints.push_back({ newX,newY,0 });
                std::wprintf(L"새로운 보간점 (%f,%f)를 추가했다!\n", newX, newY);
            }
            funcSet[dataIndex]->myInterPoints.push_back({ funcSet[dataIndex]->myPoints[i + 1].x,funcSet[dataIndex]->myPoints[i + 1].y,0 });
        }

    }
    else// if (bcInput == 2)
    {
        float firstSlope;
        float secondSlope;
        std::wprintf(L"스플라인 시작점의 기울기 상수 값을 입력해주세요.\n");
        std::cin >> firstSlope;
        std::wprintf(L"스플라인 끝점의 기울기 상수 값을 입력해주세요.\n");
        std::cin >> secondSlope;
    }
    //else std::wprintf(L"잘못된 값이 입력되었다.\n");
}