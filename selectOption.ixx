#include <SDL.h>
#include <unsupported/Eigen/CXX11/Tensor>
#include <fftw3.h>
#include <pagmo/pagmo.hpp>

export module selectOption;

import std;
import globalVar;
import constVar;
import drawer;
import read;
import rainbow;
import cubicSpline;
import inputCol;
import Func;
import utilMath;
import utilFile;
import randomRange;
import exAddOn;

void prtFuncName()
{
    for (int i = 0; i < funcSet.size(); i++)
    {
        std::wcout << "\033[38;2;" << static_cast<int>(((Func*)funcSet[i])->myColor.r) << ";" << static_cast<int>(((Func*)funcSet[i])->myColor.g) << ";" << static_cast<int>(((Func*)funcSet[i])->myColor.b) << "m";
        std::wprintf(L"[ %d�� �Լ� : ", i);
        std::wprintf(((Func*)funcSet[i])->funcName.c_str());
        std::wprintf(L"] \033[0m �����ͼ� : %d ��, ���������� : %d ��, �÷��ڵ� : %d,%d,%d \n", i, ((Func*)funcSet[i])->myPoints.size(), ((Func*)funcSet[i])->myInterPoints.size(), ((Func*)funcSet[i])->myColor.r, ((Func*)funcSet[i])->myColor.g, ((Func*)funcSet[i])->myColor.b);
    }
}

template <typename T>
T Sigmoid(T x) {
    return T(1) / (T(1) + exp(-x));
}

struct EvalData
{
    std::vector<std::array<double, 3>> points;
    double boxSize;
    int counter;
};


class MaxFxProblem {
public:
    MaxFxProblem() : data() {}
    MaxFxProblem(const EvalData& data) : data(data) {}
    MaxFxProblem(const MaxFxProblem& other) : data(other.data) {}

    std::vector<double> fitness(const std::vector<double>& x) const
    {
        double dx = x[0] - data.boxSize * std::floor(x[0] / data.boxSize);
        double dy = x[1] - data.boxSize * std::floor(x[1] / data.boxSize);
        double dz = x[2] - data.boxSize * std::floor(x[2] / data.boxSize);

        Eigen::Vector3d translation(dx, dy, dz);

        double da = x[3] - 2 * M_PI * std::floor(x[3] / (2 * M_PI));
        double db = x[4] - 2 * M_PI * std::floor(x[4] / (2 * M_PI));
        double dc = x[5] - 2 * M_PI * std::floor(x[5] / (2 * M_PI));

        double totalVal = 0;

        double length = data.boxSize;//1�ֱ�(���ּ�)
        //double length = data.boxSize / 2.0;//2�ֱ�

        double scaleFactor = 2.0 * M_PI / length;

        Eigen::Matrix3d Rx;
        Rx << 1, 0, 0,
            0, std::cos(da), -std::sin(da),
            0, std::sin(da), std::cos(da);

        Eigen::Matrix3d Ry;
        Ry << std::cos(db), 0, std::sin(db),
            0, 1, 0,
            -std::sin(db), 0, std::cos(db);

        Eigen::Matrix3d Rz;
        Rz << std::cos(dc), -std::sin(dc), 0,
            std::sin(dc), std::cos(dc), 0,
            0, 0, 1;

        Eigen::Matrix3d R = Rz * Ry * Rx;

        for (size_t i = 0; i < data.points.size(); i++) {
            double xi = data.points[i][0];
            double yi = data.points[i][1];
            double zi = data.points[i][2];

            Eigen::Vector3d p(xi, yi, zi);
            Eigen::Vector3d p_translated = p + translation;
            Eigen::Vector3d p_transformed = R * p_translated;

            double sx = scaleFactor * p_transformed(0);
            double sy = scaleFactor * p_transformed(1);
            double sz = scaleFactor * p_transformed(2);

            totalVal += (std::cos(sx) * std::sin(sy) * std::sin(2 * sz) +
                std::cos(sy) * std::sin(sz) * std::sin(2 * sx) +
                std::cos(sz) * std::sin(sx) * std::sin(2 * sy));
        }

        double fx = totalVal / static_cast<double>(data.points.size());
        return { -fx };
    }


    std::size_t get_nx() const
    {
        return 6;
    }

    std::size_t get_nobj() const
    {
        return 1;
    }

    std::pair<std::vector<double>, std::vector<double>> get_bounds() const
    {
        return {
            { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }, // ����
            { data.boxSize, data.boxSize, data.boxSize, 2 * M_PI, 2 * M_PI, 2 * M_PI } // ����
        };
    }

    std::string get_name() const
    {
        return "MaxFxProblem";
    }

private:
    EvalData data;
};


export void selectOption()
{
    std::wprintf(L"----------------------------------------------------------\n");
    std::wprintf(L"1.f(x)=y ������ �б�\n");
    std::wprintf(L"2.f(x,z)=y ������ �б�\n");
    std::wprintf(L"3.������ ���� �Է�\n");
    std::wprintf(L"4.������ �ʱ�ȭ\n");
    std::wprintf(L"5.Axis ������ ����\n");
    std::wprintf(L"6.Axis �̸� ����\n");
    if (camFixMinusZ || camFixZ || camFixMinusX || camFixX || camFixMinusY || camFixY) std::wprintf(L"7.Axis ī�޶� ���� ����\n");
    else std::wprintf(L"7.2���� ī�޶� ����\n");
    std::wprintf(L"\033[0;33m8.Delaunay �ﰢ����\033[0m\n");
    std::wprintf(L"9. �Լ� �����̵�\n");
    std::wprintf(L"10. �Լ� ȸ��\n");
    std::wprintf(L"\033[0;33m11.[2����] Cubic ���ö��� ���� ����\033[0m\n");
    std::wprintf(L"\033[0;33m12.[2����] ���� �������� ���� ���� ���� ����\033[0m\n");
    if (visUI)  std::wprintf(L"13.UI ǥ�� [ \033[0;32mON\033[0m / OFF ]\n");
    else std::wprintf(L"13.UI ǥ��  [ ON / \033[1;31mOFF\033[0m ]\n");
    if (visDataPoint)  std::wprintf(L"14.�������� ȭ�� ǥ�� [ \033[0;32mON\033[0m / OFF ]\n");
    else std::wprintf(L"14.�������� ȭ�� ǥ��  [ ON / \033[1;31mOFF\033[0m ]\n");
    if (visInterPoint)  std::wprintf(L"15.������ ȭ�� ǥ�� [ \033[0;32mON\033[0m / OFF ]\n");
    else std::wprintf(L"15.������ ȭ�� ǥ��  [ ON / \033[1;31mOFF\033[0m ]\n");
    if (darkMode)  std::wprintf(L"16.��ũ ��� [ \033[0;32mON\033[0m / OFF ]\n");
    else std::wprintf(L"16.��ũ ���  [ ON / \033[1;31mOFF\033[0m ]\n");
    std::wprintf(L"17.������ ���� ����\n");
    std::wprintf(L"18.ī�޶� �ӵ� ����\n");
    std::wprintf(L"20.������ �� ũ�� ����\n");
    std::wprintf(L"21.�׷��� �̸� ����\n");
    std::wprintf(L"22.�Լ� ��� ���\n");
    std::wprintf(L"23.���̷��̵� ���� ����\n");
    std::wprintf(L"24.�Լ��� ���\n");
    std::wprintf(L"26.PDE Solver\n");
    std::wprintf(L"27.LAMMPS Trajectory ���� �б�\n");
    std::wprintf(L"\033[0;33m29.Sort by COM \033[0m\n");
    std::wprintf(L"\033[0;33m30.Define Lattice Constant \033[0m\n");
    std::wprintf(L"\033[0;33m31.�������� ȸ�� (����Է�) \033[0m\n");
    std::wprintf(L"\033[0;33m32.�������� �����̵� \033[0m\n");
    std::wprintf(L"\033[0;33m33.Sort by PCA \033[0m\n");
    std::wprintf(L"\033[0;33m34.�������� ȸ�� (�����Է�) \033[0m\n");
    std::wprintf(L"\033[0;33m36.Ǫ������ȯ ���۷��� �Է�\033[0m\n");
    std::wprintf(L"\033[0;33m37.Ǫ������ȯ Amplitude Spectrum ȸ����� ���ϱ�\033[0m\n");
    std::wprintf(L"\033[0;33m38.Ǫ������ȯ Phase Spectrum �����̵����� ���ϱ�\033[0m\n");
    std::wprintf(L"\033[0;33m39.�Һ�ȭ �׽�Ʈ\033[0m\n");
    std::wprintf(L"\033[0;33m40 ���� ��ȭ �׽�Ʈ\033[0m\n");
    std::wprintf(L"\033[0;33m41. ȸ�� ��ȭ �׽�Ʈ\033[0m\n");
    std::wprintf(L"\033[0;33m42. 128px Ǫ������ȯ\033[0m\n");
    std::wprintf(L"\033[0;33m43. �������� ��� �м�\033[0m\n");
    std::wprintf(L"\033[0;33m44. �������� ����ȭ F�� �м�\033[0m\n");
    std::wprintf(L"\033[0;33m45. �������� ��� ������׷� ���\033[0m\n");
    std::wprintf(L"\033[0;33m45. �������� ���۷��� ������׷� ���\033[0m\n");
    std::wprintf(L"\033[0;33m46. �������� COORDINATION ���\033[0m\n");
    std::wprintf(L"\033[0;33m47. �������� �е��Լ� ��ȯ �� ���� ���\033[0m\n");
    std::wprintf(L"\033[0;33m48. ȸ�� �����̵� �׽�Ʈ : ǥ���� vs �Լ�\033[0m\n");
    std::wprintf(L"\033[0;33m49. ��е� �κ� ����\033[0m\n");
    std::wprintf(L"\033[0;33m50. LAMMPS -> Ǫ������ȯ �ļ����� ������� �׷���\033[0m\n");
    std::wprintf(L"\033[0;33m51. LAMMPS -> Ǫ������ȯ �ļ����� ��ũ �м�\033[0m\n");

    std::wprintf(L"------------------��Ʒ��� �� �Է�-----------------\n");

    int input = 0;
    std::cin >> input;
    if (input == 1)
    {
        std::wstring file = L"";
        std::wprintf(L"�����Ͱ� �ִ� ������ �������ּ���.\n");
        file = openFileDialog();
        std::wprintf(L"���� %ls �� ������� �����Ͽ���.\n", file.c_str());
        std::wifstream in(file);
        if (in.is_open())
        {
            int startLine = 0;
            std::wprintf(L"�б� ������ ���� �Է����ּ���.(0���� ����)\n");
            std::cin >> startLine;

            int endLine = 0;
            std::wprintf(L"���������� ���� ���� �Է����ּ���.(0���� ����, -1�� ��� ������)\n");
            std::cin >> endLine;

            int xCol = 0;
            std::wprintf(L"x���� �ִ� ���� �Է����ּ���.(0���� ����)\n");
            std::cin >> xCol;

            int yCol = 0;
            std::wprintf(L"y���� �ִ� ���� �Է����ּ���.(0���� ����)\n");
            std::cin >> yCol;

            SDL_Color col = inputCol();
            readXY(file, startLine, endLine, xCol, yCol, col);
        }
        else std::wprintf(L"������ �дµ� �����Ͽ����ϴ�.\n");
    }
    else if (input == 2)
    {
        std::wstring file = L"";
        std::wprintf(L"�����Ͱ� �ִ� ������ �������ּ���.\n");
        file = openFileDialog();
        std::wprintf(L"���� %ls �� ������� �����Ͽ���.\n", file.c_str());
        std::wifstream in(file);
        if (in.is_open())
        {
            int startLine = 0;
            std::wprintf(L"�б� ������ ���� �Է����ּ���.(0���� ����)\n");
            std::cin >> startLine;

            int endLine = 0;
            std::wprintf(L"���������� ���� ���� �Է����ּ���.(0���� ����, -1�� ��� ������)\n");
            std::cin >> endLine;

            int xCol = 0;
            std::wprintf(L"(Input) x���� �ִ� ���� �Է����ּ���.(0���� ����)\n");
            std::cin >> xCol;

            int zCol = 0;
            std::wprintf(L"(Input) z���� �ִ� ���� �Է����ּ���.(0���� ����)\n");
            std::cin >> zCol;

            int yCol = 0;
            std::wprintf(L"(Output) y���� �ִ� ���� �Է����ּ���.(0���� ����)\n");
            std::cin >> yCol;

            SDL_Color col = inputCol();
            readXYZ(file, startLine, endLine, xCol, yCol, zCol, col);
        }
        else std::wprintf(L"������ �дµ� �����Ͽ����ϴ�.\n");
    }
    else if (input == 3)
    {
        int targetIndex = 0;
        float inputX, inputY, inputZ;
        std::wprintf(L"��� �Լ��� ���� �߰��ұ�? (0���� %d����,-1�̸� ���ο� �Լ� �߰�)\n", funcSet.size() - 1);
        std::cin >> targetIndex;

        std::wprintf(L"x�� ���� �Է����ּ���.\n");
        std::cin >> inputX;
        std::wprintf(L"y�� ���� �Է����ּ���.\n");
        std::cin >> inputY;
        std::wprintf(L"z�� ���� �Է����ּ���.\n");
        std::cin >> inputZ;

        if (targetIndex == -1)
        {
            Func* targetFunc = new Func(funcFlag::dim3);
            targetFunc->myPoints.push_back({ inputX,inputY,inputZ });
            std::wprintf(L"������ {%f,%f,%f}�� �Լ� %d�� �Է��ߴ�.\n", inputX, inputY, inputZ, funcSet.size() - 1);
        }
        else
        {
            ((Func*)funcSet[targetIndex])->myPoints.push_back({ inputX,inputY,inputZ });
            std::wprintf(L"������ {%f,%f,%f}�� �Լ� %d�� �Է��ߴ�.\n", inputX, inputY, inputZ, targetIndex);
        }
    }
    else if (input == 4)
    {
        std::wprintf(L"��� �Լ��� �ʱ�ȭ�Ͽ���.\n");
        for (int i = funcSet.size() - 1; i >= 0; i--) delete funcSet[i];
        funcSet.clear();
    }
    else if (input == 5)
    {
        std::string axisInput;
        std::wprintf(L"������ ���� �Է����ּ���(x,y,z)\n");
        std::cin >> axisInput;
        float scaleInput;
        if (axisInput == "x")
        {
            std::wprintf(L"x���� ������ ���� �Է����ּ���.(���� : %f)\n", xScale);
            std::cin >> scaleInput;
            xScale = scaleInput;
        }
        else if (axisInput == "y")
        {
            std::wprintf(L"y���� ������ ���� �Է����ּ���.(���� : %f)\n", yScale);
            std::cin >> scaleInput;
            yScale = scaleInput;
        }
        else if (axisInput == "z")
        {
            std::wprintf(L"z���� ������ ���� �Է����ּ���.(���� : %f)\n", zScale);
            std::cin >> scaleInput;
            zScale = scaleInput;
        }
        std::wprintf(L"������ �Է��� �Ϸ��ߴ�.\n");
    }
    else if (input == 6)
    {
        std::wprintf(L"x���� �̸��� �Է����ּ���.\n");
        std::cin >> xAxisName;
        if (xAxisName == "NULL") xAxisName.clear();
        std::wprintf(L"y���� �̸��� �Է����ּ���.\n");
        std::cin >> yAxisName;
        if (yAxisName == "NULL") yAxisName.clear();
        std::wprintf(L"z���� �̸��� �Է����ּ���.\n");
        std::cin >> zAxisName;
        if (zAxisName == "NULL") zAxisName.clear();
    }
    else if (input == 7)
    {
        if (!(camFixMinusZ || camFixZ || camFixMinusX || camFixX || camFixMinusY || camFixY))
        {
            std::wprintf(L"ī�޶� � ���� �ٶ󺸴� �������� �����ұ��? (e.g. +x, -z)\n");
            std::string inputStr;
            std::cin >> inputStr;
            if (inputStr == "+x")
            {
                std::wprintf(L"ī�޶� +x �������� �����ߴ�.\n");
                camFixX = true;
                camFixMinusX = false;
                camFixY = false;
                camFixMinusY = false;
                camFixZ = false;
                camFixMinusZ = false;
                crosshair = true;

                camX = -30.0;
                camY = 0;
                camZ = 0;
            }
            else if (inputStr == "-x")
            {
                std::wprintf(L"ī�޶� -x �������� �����ߴ�.\n");
                camFixX = false;
                camFixMinusX = true;
                camFixY = false;
                camFixMinusY = false;
                camFixZ = false;
                camFixMinusZ = false;
                crosshair = true;


                camX = 30.0;
                camY = 0;
                camZ = 0;
            }
            else if (inputStr == "+y")
            {
                std::wprintf(L"ī�޶� +y �������� �����ߴ�.\n");
                camFixX = false;
                camFixMinusX = false;
                camFixY = true;
                camFixMinusY = false;
                camFixZ = false;
                camFixMinusZ = false;
                crosshair = true;

                camX = 0;
                camY = -30.0;
                camZ = 0;
            }
            else if (inputStr == "-y")
            {
                std::wprintf(L"ī�޶� -y �������� �����ߴ�.\n");
                camFixX = false;
                camFixMinusX = false;
                camFixY = false;
                camFixMinusY = true;
                camFixZ = false;
                camFixMinusZ = false;
                crosshair = true;

                camX = 0;
                camY = 30.0;
                camZ = 0;
            }
            else if (inputStr == "+z")
            {
                std::wprintf(L"ī�޶� +z �������� �����ߴ�.\n");
                camFixX = false;
                camFixMinusX = false;
                camFixY = false;
                camFixMinusY = false;
                camFixZ = true;
                camFixMinusZ = false;
                crosshair = true;

                camX = 0;
                camY = 0;
                camZ = -30.0;
            }
            else if (inputStr == "-z")
            {
                std::wprintf(L"ī�޶� -z �������� �����ߴ�.\n");
                camFixX = false;
                camFixMinusX = false;
                camFixY = false;
                camFixMinusY = false;
                camFixZ = false;
                camFixMinusZ = true;
                crosshair = true;

                camX = 0;
                camY = 0;
                camZ = 30.0;
            }
            else
            {
                std::wprintf(L"ī�޶� ������ �����ߴ�.\n");
                camFixX = false;
                camFixMinusX = false;
                camFixY = false;
                camFixMinusY = false;
                camFixZ = false;
                camFixMinusZ = false;

                crosshair = false;
            }
        }
        else
        {
            std::wprintf(L"ī�޶� ������ �����ߴ�.\n");
            camFixX = false;
            camFixMinusX = false;
            camFixY = false;
            camFixMinusY = false;
            camFixZ = false;
            camFixMinusZ = false;

            crosshair = false;
        }
    }
    else if (input == 8)
    {
        int dataIndex = 0;
        std::wprintf(L"���° �����Ϳ� �ﰢ������ �����ұ�? (0 ~ %d).\n", funcSet.size() - 1);
        prtFuncName();
        std::cin >> dataIndex;
        ((Func*)funcSet[dataIndex])->triangulation();
    }

    else if (input == 9)
    {
        int dataIndex = 0;
        std::wprintf(L"���° �����Ϳ� translation�� �����ұ�? (0 ~ %d).\n", funcSet.size() - 1);
        prtFuncName();
        std::cin >> dataIndex;

        double compX = 0, compY = 0, compZ = 0;
        std::wprintf(L"Translation Vector�� x ������ �Է����ּ���.\n");
        std::wprintf(L"Vector : { ��, ��, �� }\n");
        std::cin >> compX;
        std::wprintf(L"Translation Vector�� y ������ �Է����ּ���.\n");
        std::wprintf(L"Vector : { %f, ��, �� }\n", compX);
        std::cin >> compY;
        std::wprintf(L"Translation Vector�� z ������ �Է����ּ���.\n");
        std::wprintf(L"Vector : { %f, %f, �� }\n", compX, compY);
        std::cin >> compZ;
        std::wprintf(L"���� ���ͷ� tranlsation�� �����մϴ�.\n");
        std::wprintf(L"Vector : { %f, %f, %f }\n", compX, compY, compZ);


        ((Func*)funcSet[dataIndex])->translation(compX, compY, compZ);
        ((Func*)funcSet[dataIndex])->scalarCalc();
        std::wprintf(L"�� �Լ��� ��� f���� %f�̴�.\n", ((Func*)funcSet[dataIndex])->scalarSquareAvg());
    }

    else if (input == 10)
    {
        int dataIndex = 0;
        std::wprintf(L"���° �����Ϳ� rotation�� �����ұ�? (0 ~ %d).\n", funcSet.size() - 1);
        prtFuncName();
        std::cin >> dataIndex;

        double a11 = 1.0, a12 = 0.0, a13 = 0.0;
        double a21 = 0.0, a22 = 1.0, a23 = 0.0;
        double a31 = 0.0, a32 = 0.0, a33 = 1.0;

        std::wprintf(L"Rotation Matrix�� a11 ������ �Է����ּ���.\n", funcSet.size() - 1);
        std::wprintf(L"{ ��, ��, �� }\n");
        std::wprintf(L"{ ��, ��, �� }\n");
        std::wprintf(L"{ ��, ��, �� }\n");
        std::cin >> a11;

        std::wprintf(L"Rotation Matrix�� a12 ������ �Է����ּ���.\n", funcSet.size() - 1);
        std::wprintf(L"{ %f, ��, �� }\n", a11);
        std::wprintf(L"{ ��, ��, �� }\n");
        std::wprintf(L"{ ��, ��, �� }\n");
        std::cin >> a12;

        std::wprintf(L"Rotation Matrix�� a13 ������ �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, �� }\n", a11, a12);
        std::wprintf(L"{ ��, ��, �� }\n");
        std::wprintf(L"{ ��, ��, �� }\n");
        std::cin >> a13;

        std::wprintf(L"Rotation Matrix�� a21 ������ �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
        std::wprintf(L"{ ��, ��, �� }\n");
        std::wprintf(L"{ ��, ��, �� }\n");
        std::cin >> a21;

        std::wprintf(L"Rotation Matrix�� a22 ������ �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
        std::wprintf(L"{ %f, ��, �� }\n", a21);
        std::wprintf(L"{ ��, ��, �� }\n");
        std::cin >> a22;

        std::wprintf(L"Rotation Matrix�� a23 ������ �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
        std::wprintf(L"{ %f, %f, �� }\n", a21, a22);
        std::wprintf(L"{ ��, ��, �� }\n");
        std::cin >> a23;

        std::wprintf(L"Rotation Matrix�� a31 ������ �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
        std::wprintf(L"{ %f, %f, %f }\n", a21, a22, a23);
        std::wprintf(L"{ ��, ��, �� }\n");
        std::cin >> a31;

        std::wprintf(L"Rotation Matrix�� a32 ������ �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
        std::wprintf(L"{ %f, %f, %f }\n", a21, a22, a23);
        std::wprintf(L"{ %f, ��, �� }\n", a31);
        std::cin >> a32;

        std::wprintf(L"Rotation Matrix�� a33 ������ �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
        std::wprintf(L"{ %f, %f, %f }\n", a21, a22, a23);
        std::wprintf(L"{ %f, %f, �� }\n", a31, a32);
        std::cin >> a33;

        Eigen::Matrix3d rotationMatrix;

        //rotationMatrix << 1.0, 0.0, 0.0,
        //    0.0, 0.7071, -0.7071,
        //    0.0, 0.7071, 0.7071;

        rotationMatrix << a11, a12, a13,
            a21, a22, a23,
            a31, a32, a33;



        std::wprintf(L"���� ��ķ� Rotation�� �����մϴ�.\n");
        std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
        std::wprintf(L"{ %f, %f, %f }\n", a21, a22, a23);
        std::wprintf(L"{ %f, %f, %f }\n", a31, a32, a33);
        printRotationMatrix(rotationMatrix);

        ((Func*)funcSet[dataIndex])->rotation(rotationMatrix);
        ((Func*)funcSet[dataIndex])->scalarCalc();
        std::wprintf(L"�� �Լ��� ��� f���� %f�̴�.\n", ((Func*)funcSet[dataIndex])->scalarSquareAvg());
    }

    else if (input == 11) //cubic ���ö��� ���� ����
    {
        int dataIndex = 0;
        std::wprintf(L"���° �����͸� �����ұ�? (0 ~ %d).\n", funcSet.size() - 1);
        std::cin >> dataIndex;

        int newPointNum = 1;
        std::wprintf(L"�� ǥ���� ���̿� �� �������� ������ �Է����ּ���.\n");
        std::cin >> newPointNum;

        cubicSpline(dataIndex, newPointNum);
    }
   
    else if (input == 14)
    {
        if (visDataPoint == false)
        {
            std::wprintf(L"�����Ͱ��� ȸ�鿡�� �ٽ� ǥ���ߴ�.\n");
            visDataPoint = true;
        }
        else
        {
            std::wprintf(L"�����Ͱ��� ȸ�鿡�� �����.\n");
            visDataPoint = false;
        }
    }
    else if (input == 15)
    {
        if (visInterPoint == false)
        {
            std::wprintf(L"�������� ȸ�鿡�� �ٽ� ǥ���ߴ�.\n");
            visInterPoint = true;
        }
        else
        {
            std::wprintf(L"�������� ȸ�鿡�� �����.\n");
            visInterPoint = false;
        }
    }
    else if (input == 12)
    {
        int dataIndex = 0;
        if (funcSet.size() > 1)
        {
            std::wprintf(L"���° �����͸� �������� �����ұ�? (0 ~ %d).\n", funcSet.size() - 1);
            prtFuncName();
            std::cin >> dataIndex;
        }
        Func* tgtFunc = (Func*)funcSet[dataIndex];
        if (tgtFunc->myInterPoints.size() == 0)
        {
            tgtFunc->myInterPoints = tgtFunc->myPoints;
        }
        tgtFunc->interLine = true;
        std::wprintf(L"���������� �������� �����Ͽ���.\n");
    }
    else if (input == 13)
    {
        if (visUI == false)
        {
            std::wprintf(L"UI�� �ٽ� ǥ���ߴ�.\n");
            visUI = true;
        }
        else
        {
            std::wprintf(L"UI�� �����.\n");
            visUI = false;
        }
    }
    else if (input == 16)
    {
        if (darkMode == false)
        {
            std::wprintf(L"��ũ��带 Ȱ��ȭ�ߴ�.\n");
            darkMode = true;
        }
        else
        {
            std::wprintf(L"��ũ��带 ��Ȱ��ȭ�ߴ�.\n");
            darkMode = false;
        }
    }
    else if (input == 17)
    {
        std::wprintf(L"x�� �������� ������ �Է����ּ���.\n");
        std::wcin >> xScaleUnit;
        if (xAxisName == "NULL") xAxisName.clear();

        std::wprintf(L"y�� �������� ������ �Է����ּ���.\n");
        std::wcin >> yScaleUnit;
        if (yAxisName == "NULL") yAxisName.clear();

        std::wprintf(L"z�� �������� ������ �Է����ּ���.\n");
        std::wcin >> zScaleUnit;
        if (zAxisName == "NULL") zAxisName.clear();
    }
    else if (input == 18)
    {
        std::wprintf(L"ī�޶��� �ӵ��� �Է����ּ��� (���� : %f).\n", camSpd);
        std::cin >> camSpd;
    }
    else if (input == 20)
    {
        std::wprintf(L"���� ũ�⸦ ������ �����ұ�? (���� : %f)\n", pointSize);
        std::cin >> pointSize;
        std::wprintf(L"���� ũ�⸦ ���������� �ٲپ���.\n");
    }
    else if (input == 21)
    {
        std::wprintf(L"�� �Լ����� �̸��� ����� �ұ�?\n");
        int dummy;
        std::cin >> dummy;
        std::getline(std::wcin, graphName);
        std::wprintf(L"�Լ��� �̸��� ���������� �����ߴ�!\n");

    }
    else if (input == 22)
    {
        prtFuncName();
    }
    else if (input == 23) //���̷��̵� ����
    {
        Func* refGyroid = new Func(funcFlag::scalarField);
        double length = BOX_SIZE;
        double scaleFactor = 2.0 * M_PI / length;
        refGyroid->scalarFunc = [=](double x, double y, double z)->double
            {
                return (std::cos(scaleFactor * x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor * z)) + std::cos(scaleFactor * y) * std::sin(scaleFactor * z) * std::sin(2 * (scaleFactor * x)) + std::cos(scaleFactor * z) * std::sin(scaleFactor * x) * std::sin(2 * (scaleFactor * y)));
            };
        refGyroid->latticeConstant = BOX_SIZE;// / 2.0;
        refGyroid->scalarInfimum = -1.0;// * std::sqrt(2);
        refGyroid->scalarSupremum = 1.0;// *std::sqrt(2);

        for (double x = -BOX_SIZE / 2.0; x <= BOX_SIZE / 2.0; x += 0.2)
        {
            for (double y = -BOX_SIZE / 2.0; y <= BOX_SIZE / 2.0; y += 0.2)
            {
                for (double z = -BOX_SIZE / 2.0; z <= BOX_SIZE / 2.0; z += 0.2)
                {
                    if (refGyroid->scalarFunc(x, y, z) >= -0.2 && refGyroid->scalarFunc(x, y, z) <= 0.2) refGyroid->myPoints.push_back({ x, y, z });
                }
            }
        }
        std::wprintf(L"�����ϴ� ���̷��̵� ���� ���ڴ� %d���̴�.\n", refGyroid->myPoints.size());
        //refGyroid->scalarCalc();
    }
    else if (input == 24)
    {
        int dataIndex = 0;
        std::wprintf(L"���° �������� ����Ʈ�� ����ұ�? (0 ~ %d).\n", funcSet.size() - 1);
        prtFuncName();
        std::cin >> dataIndex;
        for (int i = 0; i < ((Func*)funcSet[dataIndex])->myPoints.size(); i++)
        {
            Point pt = ((Func*)funcSet[dataIndex])->myPoints[i];
            //std::wprintf(L"Index %d : {%.10f,%.10f,%.10f}\n", i, pt.x, pt.y, pt.z);
            std::wprintf(L"(%.5f,%.5f,%.5f),\n", i, pt.x, pt.y, pt.z);
        }
    }
    else if (input == 26)
    {
        std::wprintf(L"Fick's 2nd Law : ��C/��t = (��/��x)(D(C)��C/��x)\n");

        auto doubleInput = [](std::wstring str)->double
            {
                std::wprintf(str.c_str());
                std::wprintf(L"\n");
                double rtn;
                std::cin >> rtn;
                return rtn;
            };

        auto intInput = [](std::wstring str)->int
            {
                std::wprintf(str.c_str());
                double rtn;
                std::cin >> rtn;
                return rtn;
            };

        double delX, delT, initConc, startDistBC, infDistBC;
        int xNum, tNum;

        //double delX = doubleInput(L"[1] �Ÿ� ���� ��x �Է�\n");
        //double delT = doubleInput(L"[2] �ð� ���� ��t �Է�\n");
        //int xNum = intInput(L"[4-1] �Ÿ� ���� Ƚ�� �Է�\n");
        //int tNum = intInput(L"[4-2] �ð� ���� Ƚ�� �Է�\n");
        //double initConc = doubleInput(L"[3-1] �ð� �ʱ����� �Է� C(x,t=0)\n");
        //double startDistBC = doubleInput(L"[3-2] �Ÿ� 0 ������� �Է� C(x=0,t)\n");
        //double infDistBC = doubleInput(L"[3-3] �Ÿ� �� ������� �Է� C(x=��,t)\n");

        std::function<double(double)> diffFunc; //�󵵿� ���� ��ȭ�ϴ� Ȯ���� D(C)


        std::wprintf(L"Ȯ���� D(C) ���� ���¸� �Է����ּ���.\n");
        std::wprintf(L"1. Constant\n");
        std::wprintf(L"2. Polynomial\n");
        std::wprintf(L"3. Exponential\n");
        std::wprintf(L"4. Logarithm\n");
        int diffType;
        std::cin >> diffType;


        if (diffType == 1)
        {
            std::wprintf(L"D(C) = const\n");
            std::wprintf(L"Ȯ����(const) �Է�\n");
            double diffVal;
            std::cin >> diffVal;

            diffFunc = [=](double inputConc) -> double
                {
                    return diffVal;
                };
        }
        else if (diffType == 2)
        {
            std::wprintf(L"D(C) = D0(1+��C)\n");
            std::wprintf(L"Ȯ������ �ʱⰪ D0 �Է�\n");
            double diff0;
            std::cin >> diff0;
            std::wprintf(L"�󵵿� ���� �������� �� �Է�\n");
            double diffGamma;
            std::cin >> diffGamma;

            diffFunc = [=](double inputConc) -> double
                {
                    return diff0 * (1 + diffGamma * inputConc);
                };
        }
        else if (diffType == 3)
        {
            std::wprintf(L"D(C) = D0*exp(��C)\n");
            std::wprintf(L"Ȯ������ �ʱⰪ D0 �Է�\n");
            double diff0;
            std::cin >> diff0;
            std::wprintf(L"�󵵿� ���� �������� �� �Է�\n");
            double diffGamma;
            std::cin >> diffGamma;

            diffFunc = [=](double inputConc) -> double
                {
                    return diff0 * std::exp(diffGamma * inputConc);
                };
        }
        else if (diffType == 4)
        {
            std::wprintf(L"D(C) = D0*ln(1+��C)\n");
            std::wprintf(L"Ȯ������ �ʱⰪ D0 �Է�\n");
            double diff0;
            std::cin >> diff0;
            std::wprintf(L"�󵵿� ���� �������� �� �Է�\n");
            double diffGamma;
            std::cin >> diffGamma;

            diffFunc = [=](double inputConc) -> double
                {
                    return diff0 * std::log(1 + diffGamma * inputConc);
                };
        }

        delX = 0.1;
        //delT = 0.0005;
        delT = 0.05;
        xNum = 50;
        tNum = 400;
        initConc = 1.0;
        startDistBC = 0; //�Ѱ����� ����
        infDistBC = initConc;

        std::vector<std::vector<double>> conc;
        std::vector<double> initConcVec;
        //�ʱ�ȭ
        for (int i = 0; i < xNum; i++) initConcVec.push_back(initConc);
        conc.push_back(initConcVec);


        bool doPrint = false;
        std::wstring prtAns;
        std::wprintf(L"�����͸� ����Ͻðڽ��ϱ�? [y/n]");
        std::wcin >> prtAns;
        if (prtAns == L"y") doPrint = true;
        else doPrint = false;

        int counter = 0;
        const int printInterval = 2;
        std::wstring log = L"";
        //��̺� ������ ���
        for (int t = 0; t < tNum - 1; t++)
        {
            std::vector<double> concNew(xNum, 0.0);
            for (int i = 1; i < xNum - 1; ++i)
            {
                double diff_halfBefore = diffFunc((conc[t][i] + conc[t][i + 1]) / 2.0);
                double diff_halfAfter = diffFunc((conc[t][i] + conc[t][i - 1]) / 2.0);
                concNew[i] = conc[t][i] + delT / (delX * delX) * (diff_halfBefore * (conc[t][i + 1] - conc[t][i]) - diff_halfAfter * (conc[t][i] - conc[t][i - 1]));

                counter++;
                if (doPrint && counter >= printInterval)
                {
                    counter = 0;
                    std::wprintf(L"{x = %f,  t = %f,  C(x,t) = %f}\n", i * delX, t * delT, concNew[i]);
                }
            }
            concNew[0] = startDistBC;
            concNew[xNum - 1] = infDistBC;
            conc.push_back(concNew);
        }


        Func* targetFunc = new Func(funcFlag::dim3);

        targetFunc->myColor = inputCol();


        //targetFunc->myColor = { (Uint8)rCol,(Uint8)gCol,(Uint8)bCol };
        for (int t = 0; t < tNum; t++)
        {
            for (int x = 0; x < xNum; x++)
            {
                Point pt;
                pt.z = t * delT; //û�� �ð�
                pt.x = x * delX; //���� �Ÿ�
                pt.y = conc[t][x];
                targetFunc->myPoints.push_back(pt);


            }
        }

        std::wstring yn;
        std::wprintf(L"(��C/��x)|z=0���� ���� �׷��� i(t)�� �׸��ðڽ��ϱ�? [y/n]");
        std::wcin >> yn;

        if (yn == L"y")
        {
            Func* targetFunc2 = new Func(funcFlag::dim2);
            targetFunc2->myColor = inputCol();
            std::vector<double> diffData;
            //��̺� ���� �׷��� ����
            {
                for (int t = 0; t < tNum - 1; t++)
                {
                    double diffConc = (conc[t][2] - conc[t][0]) / (2 * delX);
                    diffData.push_back(diffConc);
                    targetFunc2->myPoints.push_back({ 0, diffConc, t * delT });
                }
            }
        }
        else
        {
        }
        std::wprintf(L"��� ����� �Ϸ�Ǿ���.\n");
    }
    else if (input == 27)//���� trajectory �б�
    {
        std::wstring file = L"";
        std::wprintf(L"�����Ͱ� �ִ� ������ �������ּ���.\n");
        file = openFileDialog();
        std::wprintf(L"���� %ls �� ������� �����Ͽ���.\n", file.c_str());
        std::ifstream in(file);
        if (in.is_open())
        {
            std::string str;
            in.seekg(0, std::ios::end);
            size_t size = in.tellg();
            str.resize(size);
            in.seekg(0, std::ios::beg);
            in.read(&str[0], size);
            in.close();

            double width = 0.0;
            std::size_t box_pos = str.find("BOX BOUNDS");
            int number_count = 0;
            if (box_pos != std::string::npos)
            {
                for (std::size_t i = box_pos; i < str.size(); ++i)
                {
                    if (isdigit(str[i]) || str[i] == '-' || str[i] == '.')
                    {
                        double temp = std::stod(str.substr(i));
                        number_count++;

                        if (number_count == 2)
                        {
                            width = temp;
                            break;
                        }

                        while (i < str.size() && (isdigit(str[i]) || str[i] == '-' || str[i] == '.' || str[i] == 'e' || str[i] == '+'))
                            i++;
                    }
                }
            }
            std::cout << "Width: " << width << std::endl;

            readTrjFile(file, 9, -1, 2, 3, 4, 1, 2);
            Func* tgtFunc = ((Func*)funcSet[funcSet.size() - 1]);
            tgtFunc->latticeConstant = width;// / 2.0;
            tgtFunc->scalarCalc();
            tgtFunc->translation(-width / 2.0, -width / 2.0, -width / 2.0);
        }
        else std::wprintf(L"������ �дµ� �����Ͽ����ϴ�.\n");
    }
    else if (input == 29)
    {
        int dataIndex = 0;
        if (funcSet.size() > 1)
        {
            std::wprintf(L"���° �����͸� �߾����Ľ�ų��? (0 ~ %d).\n", funcSet.size() - 1);
            prtFuncName();
            std::cin >> dataIndex;
        }
        ((Func*)funcSet[dataIndex])->sortByCOM();
    }
    else if (input == 30)
    {
        int dataIndex = 0;
        if (funcSet.size() > 1)
        {
            std::wprintf(L"���° �����Ϳ� ���ڻ���� �����ұ�? (0 ~ %d).\n", funcSet.size() - 1);
            prtFuncName();
            std::cin >> dataIndex;
        }
        std::wprintf(L"���ڻ���� ���� ������ �ұ�? (���⺻ 10.849215)\n");
        std::cin >> ((Func*)funcSet[dataIndex])->latticeConstant;
        std::wprintf(L"���ڻ�� ���Ǹ� �Ϸ��Ͽ���. ���� %d���� ���ڰ� �������� ���� �����Ѵ�.\n", ((Func*)funcSet[dataIndex])->countPointsWithinLattice());
    }
    else if (input == 31)
    {
        int dataIndex = 0;
        if (funcSet.size() > 1)
        {
            std::wprintf(L"���° �������� ���������� ȸ����ų��?? (0 ~ %d).\n", funcSet.size() - 1);
            prtFuncName();
            std::cin >> dataIndex;
        }

        double a11 = 1.0, a12 = 0.0, a13 = 0.0;
        double a21 = 0.0, a22 = 1.0, a23 = 0.0;
        double a31 = 0.0, a32 = 0.0, a33 = 1.0;

        std::wprintf(L"Rotation Matrix�� a11 ������ �Է����ּ���.\n", funcSet.size() - 1);
        std::wprintf(L"{ ��, ��, �� }\n");
        std::wprintf(L"{ ��, ��, �� }\n");
        std::wprintf(L"{ ��, ��, �� }\n");
        std::cin >> a11;

        std::wprintf(L"Rotation Matrix�� a12 ������ �Է����ּ���.\n", funcSet.size() - 1);
        std::wprintf(L"{ %f, ��, �� }\n", a11);
        std::wprintf(L"{ ��, ��, �� }\n");
        std::wprintf(L"{ ��, ��, �� }\n");
        std::cin >> a12;

        std::wprintf(L"Rotation Matrix�� a13 ������ �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, �� }\n", a11, a12);
        std::wprintf(L"{ ��, ��, �� }\n");
        std::wprintf(L"{ ��, ��, �� }\n");
        std::cin >> a13;

        std::wprintf(L"Rotation Matrix�� a21 ������ �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
        std::wprintf(L"{ ��, ��, �� }\n");
        std::wprintf(L"{ ��, ��, �� }\n");
        std::cin >> a21;

        std::wprintf(L"Rotation Matrix�� a22 ������ �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
        std::wprintf(L"{ %f, ��, �� }\n", a21);
        std::wprintf(L"{ ��, ��, �� }\n");
        std::cin >> a22;

        std::wprintf(L"Rotation Matrix�� a23 ������ �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
        std::wprintf(L"{ %f, %f, �� }\n", a21, a22);
        std::wprintf(L"{ ��, ��, �� }\n");
        std::cin >> a23;

        std::wprintf(L"Rotation Matrix�� a31 ������ �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
        std::wprintf(L"{ %f, %f, %f }\n", a21, a22, a23);
        std::wprintf(L"{ ��, ��, �� }\n");
        std::cin >> a31;

        std::wprintf(L"Rotation Matrix�� a32 ������ �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
        std::wprintf(L"{ %f, %f, %f }\n", a21, a22, a23);
        std::wprintf(L"{ %f, ��, �� }\n", a31);
        std::cin >> a32;

        std::wprintf(L"Rotation Matrix�� a33 ������ �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
        std::wprintf(L"{ %f, %f, %f }\n", a21, a22, a23);
        std::wprintf(L"{ %f, %f, �� }\n", a31, a32);
        std::cin >> a33;

        Eigen::Matrix3d inputMatrix;
        inputMatrix << a11, a12, a13,
            a21, a22, a23,
            a31, a32, a33;

        std::wprintf(L"�Է��� �Ϸ�Ǿ���.\n");
        std::wprintf(L"���� ��ķ� rotation�� �����մϴ�.\n");
        printRotationMatrix(inputMatrix);

        Func* tgtFunc = ((Func*)funcSet[dataIndex]);
        tgtFunc->latticeRotation(tgtFunc->myPoints, tgtFunc->latticeConstant, inputMatrix);
        tgtFunc->scalarCalc();
        std::wprintf(L"ȸ�� ���� f���� %f�̴�.\n", tgtFunc->scalarSquareAvg());
    }
    else if (input == 32)
    {
        int dataIndex = 0;
        if (funcSet.size() > 1)
        {
            std::wprintf(L"���° �������� ���������� �����̵���ų��?? (0 ~ %d).\n", funcSet.size() - 1);
            prtFuncName();
            std::cin >> dataIndex;
        }

        double compX = 0, compY = 0, compZ = 0;
        std::wprintf(L"Translation Vector�� x ������ �Է����ּ���.\n");
        std::wprintf(L"Vector : { ��, ��, �� }\n");
        std::cin >> compX;
        std::wprintf(L"Translation Vector�� y ������ �Է����ּ���.\n");
        std::wprintf(L"Vector : { %f, ��, �� }\n", compX);
        std::cin >> compY;
        std::wprintf(L"Translation Vector�� z ������ �Է����ּ���.\n");
        std::wprintf(L"Vector : { %f, %f, �� }\n", compX, compY);
        std::cin >> compZ;
        std::wprintf(L"�Է��� �Ϸ�Ǿ���.\n");
        std::wprintf(L"���� ���ͷ� tranlsation�� �����մϴ�.\n");
        std::wprintf(L"Vector : { %f, %f, %f }\n", compX, compY, compZ);

        Func* tgtFunc = ((Func*)funcSet[dataIndex]);
        tgtFunc->latticeTranslation(tgtFunc->myPoints, tgtFunc->latticeConstant, { compX, compY, compZ });
        tgtFunc->scalarCalc();
        std::wprintf(L"�����̵� ���� f���� %f�̴�.\n", tgtFunc->scalarSquareAvg());
    }
    else if (input == 33)
    {
        int dataIndex = 0;
        if (funcSet.size() > 1)
        {
            std::wprintf(L"���° �����͸� ȸ�����Ľ�ų��? (0 ~ %d).\n", funcSet.size() - 1);
            prtFuncName();
            std::cin >> dataIndex;
        }
        ((Func*)funcSet[dataIndex])->sortByPCA();
    }
    else if (input == 34)//�����Է� ȸ��
    {
        int dataIndex = 0;
        if (funcSet.size() > 1)
        {
            std::wprintf(L"���° �������� ���������� ȸ����ų��?? (0 ~ %d).\n", funcSet.size() - 1);
            prtFuncName();
            std::cin >> dataIndex;
        }

        double xAngle = 0;
        double yAngle = 0;
        double zAngle = 0;
        std::wprintf(L"Rotation Matrix�� x�� ȸ�� ��(�� ����)�� �Է����ּ���.\n");
        std::wprintf(L"{ ��, �� ,�� }\n");
        std::cin >> xAngle;

        std::wprintf(L"Rotation Matrix�� y�� ȸ�� ��(�� ����)�� �Է����ּ���.\n");
        std::wprintf(L"{ %f, ��, �� }\n", xAngle);
        std::cin >> yAngle;

        std::wprintf(L"Rotation Matrix�� z�� ȸ�� ��(�� ����)�� �Է����ּ���.\n");
        std::wprintf(L"{ %f, %f, �� }\n", xAngle, yAngle);
        std::cin >> zAngle;

        std::wprintf(L"�Է��� �Ϸ�Ǿ���.\n");
        std::wprintf(L"���� ��ķ� rotation�� �����մϴ�.\n");
        printRotationMatrix(angleToMatrix(xAngle, yAngle, zAngle));

        Func* tgtFunc = ((Func*)funcSet[dataIndex]);
        tgtFunc->latticeRotation(tgtFunc->myPoints, tgtFunc->latticeConstant, angleToMatrix(xAngle, yAngle, zAngle));
        tgtFunc->scalarCalc();
        std::wprintf(L"ȸ�� ���� f���� %f�̴�.\n", tgtFunc->scalarSquareAvg());
    }
    else if (input == 36)
    {
        int dataIndex = 0;
        if (funcSet.size() > 1)
        {
            std::wprintf(L"���° �����͸� ���۷��� ������ �����ұ�? (0 ~ %d).\n", funcSet.size() - 1);
            prtFuncName();
            std::cin >> dataIndex;
        }
        ((Func*)funcSet[dataIndex])->saveFourierRef();
    }
    else if (input == 37)
    {
        int dataIndex = 0;
        if (funcSet.size() > 1)
        {
            std::wprintf(L"���° �������� ȸ������� ���س���? (0 ~ %d).\n", funcSet.size() - 1);
            prtFuncName();
            std::cin >> dataIndex;
        }
        ((Func*)funcSet[dataIndex])->getRotationByFFT();
    }
    else if (input == 38)
    {
        int dataIndex = 0;
        if (funcSet.size() > 1)
        {
            std::wprintf(L"���° �������� �����̵� ���͸� ���س���? ȸ�������� �Ǿ��־���� (0 ~ %d).\n", funcSet.size() - 1);
            prtFuncName();
            std::cin >> dataIndex;
        }
        ((Func*)funcSet[dataIndex])->getTranslationByFFT();
    }
    else if (input == 39)
    {
        for (int atomType = 2; atomType <= 2; atomType++)//����2�� �����ϵ���
        {
            std::wstring file = L"";
            std::wprintf(L"�����Ͱ� �ִ� ������ �������ּ���.\n");
            file = openFileDialog();
            std::wprintf(L"���� %ls �� ������� �����Ͽ���.\n", file.c_str());
            std::ifstream in(file);
            if (in.is_open())
            {
                std::string str;
                in.seekg(0, std::ios::end);
                size_t size = in.tellg();
                str.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&str[0], size);
                in.close();

                Func* timeGraphFunc = new Func(funcFlag::scalarField);
                timeGraphFunc->funcType = funcFlag::dim2;
                timeGraphFunc->funcName = L"TIME-STEP F_AVG";
                std::wprintf(L"������ F_avg�� ���� ���� �ұ�?\n");
                timeGraphFunc->myColor = inputCol();

                Func* originGraphFunc = new Func(funcFlag::scalarField);
                originGraphFunc->funcType = funcFlag::dim2;
                originGraphFunc->funcName = L"ORIGIN-STEP F_AVG";
                std::wprintf(L"���� F_avg�� ���� ���� �ұ�?\n");
                originGraphFunc->myColor = inputCol();

                int i = 0;
                while (1)
                {
                    readTrjString(str, 9, -1, 2, 3, 4, 1, atomType);
                    Func* tgtGyroid = ((Func*)funcSet[funcSet.size() - 1]);
                    double length = BOX_SIZE / 2.0;
                    double scaleFactor = 2.0 * M_PI / length;
                    tgtGyroid->scalarFunc = [=](double x, double y, double z)->double
                        {
                            return (std::cos(scaleFactor * x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor * z)) + std::cos(scaleFactor * y) * std::sin(scaleFactor * z) * std::sin(2 * (scaleFactor * x)) + std::cos(scaleFactor * z) * std::sin(scaleFactor * x) * std::sin(2 * (scaleFactor * y)));
                        };
                    tgtGyroid->translation(-BOX_SIZE / 2.0, -BOX_SIZE / 2.0, -BOX_SIZE / 2.0);
                    tgtGyroid->latticeConstant = BOX_SIZE;// / 2.0;
                    tgtGyroid->scalarCalc();
                    originGraphFunc->myPoints.push_back({ (double)i,tgtGyroid->scalarSquareAvg(),0 });
                    double originF = tgtGyroid->scalarSquareAvg();

                    double lat = tgtGyroid->latticeConstant;
                    Eigen::Vector3d inputVec = { randomRangeFloat(-lat / 2.0,lat / 2.0),randomRangeFloat(-lat / 2.0,lat / 2.0),randomRangeFloat(-lat / 2.0,lat / 2.0) };
                    tgtGyroid->latticeTranslation(tgtGyroid->myPoints, tgtGyroid->latticeConstant, inputVec); //���� �����̵�

                    double xAngle = randomRangeFloat(0, 360.0);
                    double yAngle = randomRangeFloat(0, 360.0);
                    double zAngle = randomRangeFloat(0, 360.0);

                    double xRad = xAngle * DEGREE_TO_RADIAN;
                    double yRad = yAngle * DEGREE_TO_RADIAN;
                    double zRad = zAngle * DEGREE_TO_RADIAN;

                    Eigen::Matrix3d rotX, rotY, rotZ;
                    rotX << 1, 0, 0,
                        0, cos(xRad), -sin(xRad),
                        0, sin(xRad), cos(xRad);

                    rotY << cos(yRad), 0, sin(yRad),
                        0, 1, 0,
                        -sin(yRad), 0, cos(yRad);

                    rotZ << cos(zRad), -sin(zRad), 0,
                        sin(zRad), cos(zRad), 0,
                        0, 0, 1;
                    Eigen::Matrix3d inputRot = rotZ * rotY * rotX;
                    tgtGyroid->latticeRotation(tgtGyroid->myPoints, tgtGyroid->latticeConstant, inputRot); //���� ȸ��

                    std::wprintf(L"TIME %d : ���� �����̵� : (%f,%f,%f), ���� ȸ�� : (%f,%f,%f)\n", i, inputVec[0], inputVec[1], inputVec[2], xAngle, yAngle, zAngle);

                    const int RESOLUTION = 128;
                    Eigen::Tensor<std::complex<double>, 3> resultFFT = tgtGyroid->convertToDensityFuncAndFFT2(tgtGyroid->myPoints, tgtGyroid->latticeConstant, RESOLUTION);
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

                    int repeat = 0;
                    double secondPeakVal = 0;
                    for (auto it = localPeakList.rbegin(); it != localPeakList.rend(); ++it)
                    {
                        repeat++;
                        const auto& peak = *it;
                        if (repeat > 5) break;
                        if (repeat == 2)
                        {
                            std::wprintf(L"%d : Peak at (%d, %d, %d) with magnitude %f, �ش� ��ũ�� �����ߴ�.\n", repeat, peak.second[0], peak.second[1], peak.second[2], peak.first);
                            secondPeakVal = peak.first;
                        }
                        else
                        {
                            std::wprintf(L"%d : Peak at (%d, %d, %d) with magnitude %f\n", repeat, peak.second[0], peak.second[1], peak.second[2], peak.first);
                        }
                    }

                    timeGraphFunc->myPoints.push_back({ (double)i,secondPeakVal,0 });
                    delete tgtGyroid;

                    size_t firstTimestepPos = str.find("ITEM: TIMESTEP");
                    size_t secondTimestepPos = str.find("ITEM: TIMESTEP", firstTimestepPos + 1);
                    if (secondTimestepPos == std::string::npos) break;
                    else str = str.substr(secondTimestepPos);
                    i++;
                }
            }
            else std::wprintf(L"������ �дµ� �����Ͽ����ϴ�.\n");

        }
    }
    else if (input == 40)
    {
        if (hasFourierRef)
        {
            for (int i = funcSet.size() - 1; i >= 0; i--) delete funcSet[i];
            funcSet.clear();

            for (int i = 0; i < fourierTransSaveX.size(); i++)
            {
                Func* tgtFunc = new Func(funcFlag::dim2);
                tgtFunc->myColor = rainbow((double)i * 0.7 / (double)fourierTransSaveX.size());
                std::vector<std::array<std::complex<double>, 4>> fourierTargetX = fourierTransSaveX[i].second;
                for (int j = 0; j < fourierTargetX.size(); j++)
                {
                    tgtFunc->myPoints.push_back({ (double)fourierTargetX[j][0].real(),std::arg(fourierTargetX[j][3]),0 });
                }

                std::sort(tgtFunc->myPoints.begin(), tgtFunc->myPoints.end(), [](const Point& a, const Point& b) {
                    return a.x < b.x;
                    });
            }
        }
    }
    else if (input == 41)
    {
        if (hasFourierRef)
        {
            for (int i = funcSet.size() - 1; i >= 0; i--) delete funcSet[i];
            funcSet.clear();

            for (int i = 0; i < fourierAngleSaveX.size(); i++)
            {
                Func* tgtFunc = new Func(funcFlag::dim3);
                tgtFunc->myColor = rainbow((double)i * 0.7 / (double)fourierAngleSaveX.size());
                std::vector<std::array<std::complex<double>, 4>> fourierTargetX = fourierAngleSaveX[i].second;
                for (int j = 0; j < fourierTargetX.size(); j++)
                {
                    tgtFunc->myPoints.push_back({ (double)fourierTargetX[j][0].real(),(double)fourierTargetX[j][1].real(),std::abs(fourierTargetX[j][3]) });
                }

                std::sort(tgtFunc->myPoints.begin(), tgtFunc->myPoints.end(), [](const Point& a, const Point& b) {
                    return a.x < b.x;
                    });
            }
        }
    }
    else if (input == 42)
    {
        int dataIndex = 0;
        if (funcSet.size() > 1)
        {
            std::wprintf(L"���° �����͸� ���۷��� ������ �����ұ�? (0 ~ %d).\n", funcSet.size() - 1);
            prtFuncName();
            std::cin >> dataIndex;
        }
        ((Func*)funcSet[dataIndex])->saveFourierRef2();
    }
    else if (input == 43)
    {
        for (int atomType = 2; atomType <= 2; atomType++)//����2�� �����ϵ���
        {
            std::wstring file = L"";
            std::wprintf(L"�����Ͱ� �ִ� ������ �������ּ���.\n");
            file = openFileDialog();
            std::wprintf(L"���� %ls �� ������� �����Ͽ���.\n", file.c_str());
            std::ifstream in(file);
            if (in.is_open())
            {
                std::string str;
                in.seekg(0, std::ios::end);
                size_t size = in.tellg();
                str.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&str[0], size);
                in.close();


                Func* timeGraphFunc3 = new Func(funcFlag::scalarField);
                timeGraphFunc3->funcType = funcFlag::dim2;
                timeGraphFunc3->funcName = L"����";
                std::wprintf(L"����° F_avg�� ���� ���� �ұ�?\n");
                timeGraphFunc3->myColor = rainbow(0.2);


                Func* originGraphFunc = new Func(funcFlag::scalarField);
                originGraphFunc->funcType = funcFlag::dim2;
                originGraphFunc->funcName = L"���� F��";
                std::wprintf(L"���� F_avg�� ���� ���� �ұ�?\n");
                originGraphFunc->myColor = { 0xff,0xff,0xff };

                int i = 0;
                while (1)
                {
                    double width = 0.0;
                    std::size_t box_pos = str.find("BOX BOUNDS");
                    int number_count = 0;
                    if (box_pos != std::string::npos)
                    {
                        for (std::size_t i = box_pos; i < str.size(); ++i)
                        {
                            if (isdigit(str[i]) || str[i] == '-' || str[i] == '.')
                            {
                                double temp = std::stod(str.substr(i));
                                number_count++;

                                if (number_count == 2)
                                {
                                    width = temp;
                                    break;
                                }

                                while (i < str.size() && (isdigit(str[i]) || str[i] == '-' || str[i] == '.' || str[i] == 'e' || str[i] == '+'))
                                    i++;
                            }
                        }
                    }
                    std::cout << "Width: " << width << std::endl;

                    readTrjString(str, 9, -1, 2, 3, 4, 1, atomType);
                    Func* tgtGyroid = ((Func*)funcSet[funcSet.size() - 1]);
                    double length = width / 2;
                    double scaleFactor = 2.0 * M_PI / length;
                    tgtGyroid->scalarFunc = [=](double x, double y, double z)->double
                        {
                            return (std::cos(scaleFactor * x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor * z)) + std::cos(scaleFactor * y) * std::sin(scaleFactor * z) * std::sin(2 * (scaleFactor * x)) + std::cos(scaleFactor * z) * std::sin(scaleFactor * x) * std::sin(2 * (scaleFactor * y)));
                        };
                    tgtGyroid->latticeConstant = width;
                    tgtGyroid->translation(-width / 2.0, -width / 2.0, -width / 2.0);
                    tgtGyroid->scalarCalc();
                    originGraphFunc->myPoints.push_back({ (double)i,tgtGyroid->scalarSquareAvg(),0 });
                    double originF = tgtGyroid->scalarSquareAvg();

                    //Random translation
                    Eigen::Vector3d inputVec =
                    {
                        randomRangeFloat(-width / 2.0,width / 2.0),
                        randomRangeFloat(-width / 2.0,width / 2.0),
                        randomRangeFloat(-width / 2.0,width / 2.0)
                    };
                    tgtGyroid->latticeTranslation(tgtGyroid->myPoints, tgtGyroid->latticeConstant, inputVec);

                    double xAngle = randomRangeFloat(0, 360.0);
                    double yAngle = randomRangeFloat(0, 360.0);
                    double zAngle = randomRangeFloat(0, 360.0);

                    //Random rotation
                    double xRad = xAngle * DEGREE_TO_RADIAN;
                    double yRad = yAngle * DEGREE_TO_RADIAN;
                    double zRad = zAngle * DEGREE_TO_RADIAN;

                    Eigen::Matrix3d rotX, rotY, rotZ;
                    rotX << 1, 0, 0,
                        0, cos(xRad), -sin(xRad),
                        0, sin(xRad), cos(xRad);

                    rotY << cos(yRad), 0, sin(yRad),
                        0, 1, 0,
                        -sin(yRad), 0, cos(yRad);

                    rotZ << cos(zRad), -sin(zRad), 0,
                        sin(zRad), cos(zRad), 0,

                        0, 0, 1;
                    Eigen::Matrix3d inputRot = rotZ * rotY * rotX;
                    tgtGyroid->latticeRotation(tgtGyroid->myPoints, tgtGyroid->latticeConstant, inputRot);


                    //std::wprintf(L"TIME %d : ���� �����̵� : (%f,%f,%f), ���� ȸ�� : (%f,%f,%f)\n", i, inputVec[0], inputVec[1], inputVec[2], xAngle, yAngle, zAngle);
                    const int resol = 32;
                    double*** density = create3DArray(resol, resol, resol);
                    createDensityFunction(tgtGyroid->getRawPoints(), width, density, resol);
                    double stdev = densityToStdev(density, width, resol);

                    timeGraphFunc3->myPoints.push_back({ (double)i,stdev,0 });
                    delete tgtGyroid;

                    size_t firstTimestepPos = str.find("ITEM: TIMESTEP");
                    size_t secondTimestepPos = str.find("ITEM: TIMESTEP", firstTimestepPos + 1);
                    if (secondTimestepPos == std::string::npos) break;
                    else str = str.substr(secondTimestepPos);
                    i++;
                }
            }
            else std::wprintf(L"������ �дµ� �����Ͽ����ϴ�.\n");
        }
    }
    else if (input == 44)
    {
        for (int atomType = 2; atomType <= 2; atomType++)//����2�� �����ϵ���
        {
            std::wstring file = L"";
            std::wprintf(L"�����Ͱ� �ִ� ������ �������ּ���.\n");
            file = openFileDialog();
            std::wprintf(L"���� %ls �� ������� �����Ͽ���.\n", file.c_str());
            std::ifstream in(file);
            if (in.is_open())
            {
                std::string str;
                in.seekg(0, std::ios::end);
                size_t size = in.tellg();
                str.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&str[0], size);
                in.close();

                Func* optGraphFunc = new Func(funcFlag::scalarField);
                optGraphFunc->funcType = funcFlag::dim2;
                optGraphFunc->funcName = L"���";
                optGraphFunc->myColor = rainbow(0);

                Func* originGraphFunc = new Func(funcFlag::scalarField);
                originGraphFunc->funcType = funcFlag::dim2;
                originGraphFunc->funcName = L"���� F��";
                originGraphFunc->myColor = { 0xff,0xff,0xff };

                int i = 0;
                while (1)
                {
                    double width = 0.0;
                    std::size_t box_pos = str.find("BOX BOUNDS");
                    int number_count = 0;
                    if (box_pos != std::string::npos)
                    {
                        for (std::size_t i = box_pos; i < str.size(); ++i)
                        {
                            if (isdigit(str[i]) || str[i] == '-' || str[i] == '.')
                            {
                                double temp = std::stod(str.substr(i));
                                number_count++;

                                if (number_count == 2)
                                {
                                    width = temp;
                                    break;
                                }

                                while (i < str.size() && (isdigit(str[i]) || str[i] == '-' || str[i] == '.' || str[i] == 'e' || str[i] == '+'))
                                    i++;
                            }
                        }
                    }
                    std::cout << "Width: " << width << std::endl;

                    readTrjString(str, 9, -1, 2, 3, 4, 1, atomType);
                    Func* tgtGyroid = ((Func*)funcSet[funcSet.size() - 1]);
                    //double length = width/2.0;
                    double length = width;
                    double scaleFactor = 2.0 * M_PI / length;
                    tgtGyroid->scalarFunc = [=](double x, double y, double z)->double
                        {
                            return (std::cos(scaleFactor * x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor * z)) + std::cos(scaleFactor * y) * std::sin(scaleFactor * z) * std::sin(2 * (scaleFactor * x)) + std::cos(scaleFactor * z) * std::sin(scaleFactor * x) * std::sin(2 * (scaleFactor * y)));
                        };
                    tgtGyroid->translation(-width / 2.0, -width / 2.0, -width / 2.0);
                    tgtGyroid->latticeConstant = width;// / 2.0;
                    tgtGyroid->scalarCalc();
                    originGraphFunc->myPoints.push_back({ (double)i,tgtGyroid->scalarSquareAvg(),0 });
                    double originF = tgtGyroid->scalarSquareAvg();

                    double lat = tgtGyroid->latticeConstant;
                    Eigen::Vector3d inputVec = { randomRangeFloat(-lat / 2.0,lat / 2.0),randomRangeFloat(-lat / 2.0,lat / 2.0),randomRangeFloat(-lat / 2.0,lat / 2.0) };
                    tgtGyroid->latticeTranslation(tgtGyroid->myPoints, tgtGyroid->latticeConstant, inputVec); //���� �����̵�

                    double xAngle = randomRangeFloat(0, 360.0);
                    double yAngle = randomRangeFloat(0, 360.0);
                    double zAngle = randomRangeFloat(0, 360.0);

                    double xRad = xAngle * DEGREE_TO_RADIAN;
                    double yRad = yAngle * DEGREE_TO_RADIAN;
                    double zRad = zAngle * DEGREE_TO_RADIAN;

                    Eigen::Matrix3d rotX, rotY, rotZ;
                    rotX << 1, 0, 0,
                        0, cos(xRad), -sin(xRad),
                        0, sin(xRad), cos(xRad);

                    rotY << cos(yRad), 0, sin(yRad),
                        0, 1, 0,
                        -sin(yRad), 0, cos(yRad);

                    rotZ << cos(zRad), -sin(zRad), 0,
                        sin(zRad), cos(zRad), 0,

                        0, 0, 1;
                    Eigen::Matrix3d inputRot = rotZ * rotY * rotX;
                    tgtGyroid->latticeRotation(tgtGyroid->myPoints, tgtGyroid->latticeConstant, inputRot); //���� ȸ��


                    double optF;

                    if (1)
                    {
                        EvalData data;
                        data.boxSize = width;
                        data.points = tgtGyroid->getRawPoints();

                        MaxFxProblem prob{ data };
                        pagmo::problem p{ prob };
                        pagmo::algorithm algo{ pagmo::de(100) };
                        pagmo::population pop{ p, 400 };
                        pop = algo.evolve(pop);
                        auto best_fitness = pop.get_f()[pop.best_idx()][0];
                        auto best_solution = pop.get_x()[pop.best_idx()];

                        std::cout << "������ fx ��: " << -best_fitness << std::endl;
                        std::cout << "������ �Ű�����:" << std::endl;
                        std::cout << "dx: " << best_solution[0] << std::endl;
                        std::cout << "dy: " << best_solution[1] << std::endl;
                        std::cout << "dz: " << best_solution[2] << std::endl;
                        std::cout << "da: " << best_solution[3] << std::endl;
                        std::cout << "db: " << best_solution[4] << std::endl;
                        std::cout << "dc: " << best_solution[5] << std::endl;

                        optF = -best_fitness;

                    }


                    std::wprintf(L"TIME %d : ���� �����̵� : (%f,%f,%f), ���� ȸ�� : (%f,%f,%f)\n", i, inputVec[0], inputVec[1], inputVec[2], xAngle, yAngle, zAngle);

                    optGraphFunc->myPoints.push_back({ (double)i,optF,0 });
                    delete tgtGyroid;

                    size_t firstTimestepPos = str.find("ITEM: TIMESTEP");
                    size_t secondTimestepPos = str.find("ITEM: TIMESTEP", firstTimestepPos + 1);
                    if (secondTimestepPos == std::string::npos) break;
                    else str = str.substr(secondTimestepPos);
                    i++;
                }
            }
            else std::wprintf(L"������ �дµ� �����Ͽ����ϴ�.\n");
        }
    }
    else if (input == 45)
    {
        for (int atomType = 2; atomType <= 2; atomType++)//����2�� �����ϵ���
        {
            std::wstring file = L"";
            std::wprintf(L"�����Ͱ� �ִ� ������ �������ּ���.\n");
            file = openFileDialog();
            std::wprintf(L"���� %ls �� ������� �����Ͽ���.\n", file.c_str());
            std::ifstream in(file);
            if (in.is_open())
            {
                std::string str;
                in.seekg(0, std::ios::end);
                size_t size = in.tellg();
                str.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&str[0], size);
                in.close();

                Func* gyroidFunc = new Func(funcFlag::scalarField);
                gyroidFunc->funcType = funcFlag::dim2;
                gyroidFunc->funcName = L"���̷��̵�";
                gyroidFunc->myColor = rainbow(0.2);


                Func* randomFunc = new Func(funcFlag::scalarField);
                randomFunc->funcType = funcFlag::dim2;
                randomFunc->funcName = L"����";
                randomFunc->myColor = { 0xff,0xff,0xff };

                int i = 0;
                while (1)
                {
                    double width = 0.0;
                    std::size_t box_pos = str.find("BOX BOUNDS");
                    int number_count = 0;

                    if (box_pos != std::string::npos)
                    {
                        for (std::size_t i = box_pos; i < str.size(); ++i)
                        {
                            if (isdigit(str[i]) || str[i] == '-' || str[i] == '.')
                            {
                                double temp = std::stod(str.substr(i));
                                number_count++;

                                if (number_count == 2)
                                {
                                    width = temp;
                                    break;
                                }

                                while (i < str.size() && (isdigit(str[i]) || str[i] == '-' || str[i] == '.' || str[i] == 'e' || str[i] == '+'))
                                    i++;
                            }
                        }
                    }
                    std::cout << "Width: " << width << std::endl;

                    readTrjString(str, 9, -1, 2, 3, 4, 1, atomType);
                    Func* tgtGyroid = ((Func*)funcSet[funcSet.size() - 1]);
                    double length = width / 2;
                    double scaleFactor = 2.0 * M_PI / length;
                    tgtGyroid->scalarFunc = [=](double x, double y, double z)->double
                        {
                            return (std::cos(scaleFactor * x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor * z)) + std::cos(scaleFactor * y) * std::sin(scaleFactor * z) * std::sin(2 * (scaleFactor * x)) + std::cos(scaleFactor * z) * std::sin(scaleFactor * x) * std::sin(2 * (scaleFactor * y)));
                        };
                    tgtGyroid->latticeConstant = width;
                    tgtGyroid->translation(-width / 2.0, -width / 2.0, -width / 2.0);
                    tgtGyroid->scalarCalc();

                    //Random translation
                    Eigen::Vector3d inputVec =
                    {
                        randomRangeFloat(-width / 2.0,width / 2.0),
                        randomRangeFloat(-width / 2.0,width / 2.0),
                        randomRangeFloat(-width / 2.0,width / 2.0)
                    };
                    tgtGyroid->latticeTranslation(tgtGyroid->myPoints, tgtGyroid->latticeConstant, inputVec);

                    double xAngle = randomRangeFloat(0, 360.0);
                    double yAngle = randomRangeFloat(0, 360.0);
                    double zAngle = randomRangeFloat(0, 360.0);

                    //Random rotation
                    double xRad = xAngle * DEGREE_TO_RADIAN;
                    double yRad = yAngle * DEGREE_TO_RADIAN;
                    double zRad = zAngle * DEGREE_TO_RADIAN;

                    Eigen::Matrix3d rotX, rotY, rotZ;
                    rotX << 1, 0, 0,
                        0, cos(xRad), -sin(xRad),
                        0, sin(xRad), cos(xRad);

                    rotY << cos(yRad), 0, sin(yRad),
                        0, 1, 0,
                        -sin(yRad), 0, cos(yRad);

                    rotZ << cos(zRad), -sin(zRad), 0,
                        sin(zRad), cos(zRad), 0,

                        0, 0, 1;
                    Eigen::Matrix3d inputRot = rotZ * rotY * rotX;
                    tgtGyroid->latticeRotation(tgtGyroid->myPoints, tgtGyroid->latticeConstant, inputRot);



                    if (i == 0 || i == 200)
                    {
                        int resol = 64;
                        double*** density = create3DArray(resol, resol, resol);
                        createDensityFunction(tgtGyroid->getRawPoints(), width, density, resol);
                        double*** deriv2 = createLaplacian(density, resol, width);
                        double*** deriv3 = createDerivative3(density, resol, width);
                        double*** deriv4 = createDerivative4(density, resol, width);

                        std::vector<double> dataset;
                        for (int x = 0; x < resol; x++)
                        {
                            for (int y = 0; y < resol; y++)
                            {
                                for (int z = 0; z < resol; z++)
                                {
                                    dataset.push_back(deriv3[x][y][z]);
                                }
                            }
                        }

                        std::vector<std::array<double, 3>> hist = createHistogramSize(dataset, 256);

                        if (i == 0)
                        {
                            for (int j = 0; j < hist.size(); j++)
                            {
                                randomFunc->myPoints.push_back({ hist[j][0],hist[j][1],hist[j][2] });
                            }
                        }
                        else if (i == 200)
                        {
                            for (int j = 0; j < hist.size(); j++)
                            {
                                gyroidFunc->myPoints.push_back({ hist[j][0],hist[j][1],hist[j][2] });
                            }
                        }


                        free3DArray(density, resol, resol);
                        free3DArray(deriv2, resol, resol);
                        free3DArray(deriv3, resol, resol);
                        free3DArray(deriv4, resol, resol);
                    }



                    std::wprintf(L"TIME %d : ���� �����̵� : (%f,%f,%f), ���� ȸ�� : (%f,%f,%f)\n", i, inputVec[0], inputVec[1], inputVec[2], xAngle, yAngle, zAngle);
                    delete tgtGyroid;

                    size_t firstTimestepPos = str.find("ITEM: TIMESTEP");
                    size_t secondTimestepPos = str.find("ITEM: TIMESTEP", firstTimestepPos + 1);
                    if (secondTimestepPos == std::string::npos) break;
                    else str = str.substr(secondTimestepPos);
                    i++;
                }
            }
            else std::wprintf(L"������ �дµ� �����Ͽ����ϴ�.\n");
        }
    }
    else if (input == 46)
    {
        int dataIndex = 0;
        double cutoff = 1.0;
        std::wprintf(L"���° �������� COORDINATION�� ���ұ�? (0 ~ %d).\n", funcSet.size() - 1);
        std::cin >> dataIndex;
        std::wprintf(L"�ƿ��� ���� �Է����ּ���.\n");
        std::cin >> cutoff;

        std::vector<Point> pts = ((Func*)funcSet[dataIndex])->myPoints;

        delete funcSet[dataIndex];
        funcSet.pop_back();

        Func* coordFunc = new Func(funcFlag::dim2);
        coordFunc->funcName = L"COORDINATION ";
        coordFunc->funcName += std::to_wstring(cutoff);
        coordFunc->myColor = rainbow(randomRangeFloat(0.0, 0.7));

        std::array<int, 1000> coords = { 0, };
        for (int i = 0; i < pts.size(); i++)
        {
            double originDist = std::sqrt(std::pow(pts[i].x, 2) + std::pow(pts[i].y, 2) + std::pow(pts[i].z, 2));
            if (originDist < 4.0)
            {
                int number = 0;

                for (int j = 0; j < pts.size(); j++)
                {
                    if (i != j)
                    {
                        double dist = std::sqrt(std::pow(pts[i].x - pts[j].x, 2) + std::pow(pts[i].y - pts[j].y, 2) + std::pow(pts[i].z - pts[j].z, 2));
                        if (dist < cutoff) number++;
                    }
                }
                coords[number] += 1;
            }
        }

        for (int i = 0; i < coords.size(); i++)
        {
            coordFunc->myPoints.push_back({ (double)i,(double)coords[i],0 });
        }

        std::wprintf(L"COORDINATION�� ���������� ���ߴ�.\n");
    }
    else if (input == 47)
    {
        int dataIndex = 0;
        std::wprintf(L"���° �������� �е��Լ��� ���ұ�? (0 ~ %d).\n", funcSet.size() - 1);
        std::cin >> dataIndex;

        double cutoff = 1.0;
        std::wprintf(L"�ƿ��� ���� ������ �ұ�?.\n");
        std::cin >> cutoff;


        int resolution = 64;
        //std::wprintf(L"�ػ󵵸� ������ �ұ�?\n");
        //std::cin >> resolution;

        Func* lastFunc = ((Func*)funcSet[dataIndex]);

        std::vector<Point> pts = lastFunc->myPoints;
        std::vector<std::array<double, 3>> dPts;
        for (int i = 0; i < pts.size(); i++) dPts.push_back({ pts[i].x,pts[i].y,pts[i].z });

        double*** density = create3DArray(resolution, resolution, resolution);
        createDensityFunction(dPts, lastFunc->latticeConstant, density, resolution);


        lastFunc->funcType = funcFlag::dim3;
        lastFunc->myColor = inputCol();

        

        lastFunc->myPoints.clear();

        double energy = getDirichletEnergy(density, resolution, (lastFunc->latticeConstant) * (lastFunc->latticeConstant) * (lastFunc->latticeConstant));
        std::wprintf(L"�� �е��Լ��� ��Ŭ�� �������� %lf�̴�.\n", energy);

        lastFunc->triangles = getTrianglesFromScalar(arrayToTensor(density, resolution), cutoff);
        double surfaceArea = getAreaFromTriangles(lastFunc->triangles);
        calculateAndAnalyzeCurvature(lastFunc->triangles);
        std::wprintf(L"�� �е��Լ��� ǥ������ %lf�̴�.\n", surfaceArea);
        auto largestSurf = getLargestSurfaces(lastFunc->triangles);
        std::wprintf(L"���� ū ����� ���̴� %lf�̰� �ι�°�� ū ����� ���̴� %lf�̴�.\n", largestSurf.first, largestSurf.second);

    }
    else if (input == 48)
    {
        for (int atomType = 2; atomType <= 2; atomType++)//����2�� �����ϵ���
        {
            std::wstring file = L"";
            std::wprintf(L"�����Ͱ� �ִ� ������ �������ּ���.\n");
            file = openFileDialog();
            std::wprintf(L"���� %ls �� ������� �����Ͽ���.\n", file.c_str());
            std::ifstream in(file);
            if (in.is_open())
            {
                std::string str;
                in.seekg(0, std::ios::end);
                size_t size = in.tellg();
                str.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&str[0], size);
                in.close();


                Func* surfaceFunc = new Func(funcFlag::scalarField);
                surfaceFunc->funcType = funcFlag::dim2;
                surfaceFunc->funcName = L"ǥ����";
                std::wprintf(L"����° F_avg�� ���� ���� �ұ�?\n");
                surfaceFunc->myColor = rainbow(0.2);


                Func* originGraphFunc = new Func(funcFlag::scalarField);
                originGraphFunc->funcType = funcFlag::dim2;
                originGraphFunc->funcName = L"���� F��";
                std::wprintf(L"���� F_avg�� ���� ���� �ұ�?\n");
                originGraphFunc->myColor = { 0xff,0xff,0xff };

                int i = 0;
                while (1)
                {
                    double width = 0.0;
                    std::size_t box_pos = str.find("BOX BOUNDS");
                    int number_count = 0;

                    if (box_pos != std::string::npos)
                    {
                        for (std::size_t i = box_pos; i < str.size(); ++i)
                        {
                            if (isdigit(str[i]) || str[i] == '-' || str[i] == '.')
                            {
                                double temp = std::stod(str.substr(i));
                                number_count++;

                                if (number_count == 2)
                                {
                                    width = temp;
                                    break;
                                }

                                while (i < str.size() && (isdigit(str[i]) || str[i] == '-' || str[i] == '.' || str[i] == 'e' || str[i] == '+'))
                                    i++;
                            }
                        }
                    }
                    std::cout << "Width: " << width << std::endl;

                    readTrjString(str, 9, -1, 2, 3, 4, 1, atomType);
                    Func* tgtGyroid = ((Func*)funcSet[funcSet.size() - 1]);
                    double length = width;
                    double scaleFactor = 2.0 * M_PI / length;
                    tgtGyroid->scalarFunc = [=](double x, double y, double z)->double
                        {
                            return (std::cos(scaleFactor * x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor * z)) + std::cos(scaleFactor * y) * std::sin(scaleFactor * z) * std::sin(2 * (scaleFactor * x)) + std::cos(scaleFactor * z) * std::sin(scaleFactor * x) * std::sin(2 * (scaleFactor * y)));
                        };
                    tgtGyroid->latticeConstant = width;
                    tgtGyroid->translation(-width / 2.0, -width / 2.0, -width / 2.0);
                    tgtGyroid->scalarCalc();
                    originGraphFunc->myPoints.push_back({ (double)i,tgtGyroid->scalarSquareAvg(),0 });
                    double originF = tgtGyroid->scalarSquareAvg();

                    //Random translation
                    Eigen::Vector3d inputVec =
                    {
                        randomRangeFloat(-width / 2.0,width / 2.0),
                        randomRangeFloat(-width / 2.0,width / 2.0),
                        randomRangeFloat(-width / 2.0,width / 2.0)
                    };
                    //tgtGyroid->latticeTranslation(tgtGyroid->myPoints, tgtGyroid->latticeConstant, inputVec);

                    double xAngle = randomRangeFloat(0, 360.0);
                    double yAngle = randomRangeFloat(0, 360.0);
                    double zAngle = randomRangeFloat(0, 360.0);

                    //Random rotation
                    double xRad = xAngle * DEGREE_TO_RADIAN;
                    double yRad = yAngle * DEGREE_TO_RADIAN;
                    double zRad = zAngle * DEGREE_TO_RADIAN;

                    Eigen::Matrix3d rotX, rotY, rotZ;
                    rotX << 1, 0, 0,
                        0, cos(xRad), -sin(xRad),
                        0, sin(xRad), cos(xRad);

                    rotY << cos(yRad), 0, sin(yRad),
                        0, 1, 0,
                        -sin(yRad), 0, cos(yRad);

                    rotZ << cos(zRad), -sin(zRad), 0,
                        sin(zRad), cos(zRad), 0,

                        0, 0, 1;
                    Eigen::Matrix3d inputRot = rotZ * rotY * rotX;
                    //tgtGyroid->latticeRotation(tgtGyroid->myPoints, tgtGyroid->latticeConstant, inputRot);


                    std::wprintf(L"TIME %d : ���� �����̵� : (%f,%f,%f), ���� ȸ�� : (%f,%f,%f)\n", i, inputVec[0], inputVec[1], inputVec[2], xAngle, yAngle, zAngle);
                    const int resol = 32;

                    double*** density = create3DArray(resol, resol, resol);
                    createDensityFunction(tgtGyroid->getRawPoints(), width, density, resol);
                    double stdev = densityToStdev(density, width, resol);
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
                    std::vector<Triangle> triSet = getTrianglesFromScalar(densityTensor, 1.95, 0.05);
                    double surfaceArea = getAreaFromTriangles(triSet);

                    surfaceFunc->myPoints.push_back({ (double)i,surfaceArea,0 });
                    delete tgtGyroid;
                    free3DArray(density, resol, resol);

                    size_t firstTimestepPos = str.find("ITEM: TIMESTEP");
                    size_t secondTimestepPos = str.find("ITEM: TIMESTEP", firstTimestepPos + 1);
                    if (secondTimestepPos == std::string::npos) break;
                    else str = str.substr(secondTimestepPos);
                    i++;
                }
            }
            else std::wprintf(L"������ �дµ� �����Ͽ����ϴ�.\n");
        }
    }
    else if (input == 49)
    {
        int dataIndex = 0;
        std::wprintf(L"���° �������� �е��Լ��� ���ұ�? (0 ~ %d).\n", funcSet.size() - 1);
        std::cin >> dataIndex;

        double cutoff = 2.0;
        std::wprintf(L"�ƿ��� ���� ������ �ұ�?.\n");
        std::cin >> cutoff;
        int resolution = 64;

        Func* lastFunc = ((Func*)funcSet[dataIndex]);
        std::vector<Point> pts = lastFunc->myPoints;
        std::vector<std::array<double, 3>> dPts;
        for (int i = 0; i < pts.size(); i++) dPts.push_back({ pts[i].x,pts[i].y,pts[i].z });

        double period = 4.942;
        double*** density = create3DArray(resolution, resolution, resolution);
        createDensityFunction(dPts, period, density, resolution);

        lastFunc->funcType = funcFlag::dim3;
        std::wprintf(L"���� ���� �ұ�?\n");
        lastFunc->myColor = inputCol();

        lastFunc->myPoints.clear();
        int pointNumber = 0;
        for (int x = 0; x < resolution; x++)
        {
            for (int y = 0; y < resolution; y++)
            {
                for (int z = 0; z < resolution; z++)
                {
                    if (density[x][y][z] > cutoff)
                    {
                        lastFunc->myPoints.push_back({ (double)x - (double)resolution / 2.0,(double)y - (double)resolution / 2.0,(double)z - (double)resolution / 2.0 });
                        pointNumber++;
                    }
                }
            }
        }

    }
    else if (input == 50)
    {
        int dataIndex = 0;
        std::wprintf(L"���° �������� Ǫ���� �������ڸ� ���ұ�? (0 ~ %d).\n", funcSet.size() - 1);
        std::cin >> dataIndex;

        Func* lastFunc = ((Func*)funcSet[dataIndex]);

        double boxSize = lastFunc->latticeConstant;

        std::vector<Point> pts = lastFunc->myPoints;
        std::vector<std::array<double, 3>> dPts;
        for (int i = 0; i < pts.size(); i++) dPts.push_back({ pts[i].x,pts[i].y,pts[i].z });

        lastFunc->funcType = funcFlag::dim3;
        std::wprintf(L"���� ���� �ұ�?\n");
        lastFunc->myColor = inputCol();

        lastFunc->myPoints.clear();

        for (double kVal = 0; kVal <= 30; kVal += 0.03)
        {
            double y = computeShellAverageAmplitude(dPts, boxSize, boxSize, boxSize, kVal);
            lastFunc->myPoints.push_back({ kVal,y,0 });
        }
    }
    else if (input == 51)
    {
        int dataIndex = 0;
        std::wprintf(L"���° �������� Ǫ���� �������ڸ� ���ұ�? (0 ~ %d).\n", funcSet.size() - 1);
        std::cin >> dataIndex;

        Func* lastFunc = ((Func*)funcSet[dataIndex]);

        double boxSize = lastFunc->latticeConstant;
        std::vector<Point> pts = lastFunc->myPoints;
        std::vector<std::array<double, 3>> dPts;
        dPts.reserve(pts.size());
        for (int i = 0; i < pts.size(); i++)
        {
            dPts.push_back({ pts[i].x, pts[i].y, pts[i].z });
        }

        double kMin = -10.0;
        double kMax = 10.0;
        double step = 0.1;

        int Nx = static_cast<int>((kMax - kMin) / step) + 1;
        int Ny = Nx;
        int Nz = Nx;

        std::vector<std::vector<std::vector<double>>> amplitudeGrid(
            Nx, std::vector<std::vector<double>>(Ny, std::vector<double>(Nz, 0.0))
        );

        for (int i = 0; i < Nx; i++)
        {
            double kx = kMin + i * step;
            for (int j = 0; j < Ny; j++)
            {
                double ky = kMin + j * step;
                for (int k = 0; k < Nz; k++)
                {
                    double kz = kMin + k * step;
                    amplitudeGrid[i][j][k] = computeDirectionalAmplitude(dPts, kx, ky, kz, false);
                }
            }
        }

        auto isLocalMax = [&](int i, int j, int k)
            {
                double center = amplitudeGrid[i][j][k];
                for (int di = -1; di <= 1; di++)
                {
                    for (int dj = -1; dj <= 1; dj++)
                    {
                        for (int dk = -1; dk <= 1; dk++)
                        {
                            if (di == 0 && dj == 0 && dk == 0) continue;
                            double neighbor = amplitudeGrid[i + di][j + dj][k + dk];
                            if (neighbor > center) return false;
                        }
                    }
                }
                return true;
            };

        struct Peak {
            double kx, ky, kz, amplitude;
        };

        std::vector<Peak> peaks;
        peaks.reserve(Nx * Ny * Nz);

        for (int i = 1; i < Nx - 1; i++)
        {
            double kx = kMin + i * step;
            for (int j = 1; j < Ny - 1; j++)
            {
                double ky = kMin + j * step;
                for (int k = 1; k < Nz - 1; k++)
                {
                    double kz = kMin + k * step;
                    if (isLocalMax(i, j, k))
                    {
                        double amp = amplitudeGrid[i][j][k];
                        peaks.push_back({ kx, ky, kz, amp });
                    }
                }
            }
        }

        std::sort(peaks.begin(), peaks.end(),
            [](const Peak& a, const Peak& b) {
                return a.amplitude > b.amplitude;
            }
        );

        int topN = 100;
        std::wprintf(L"\n=== Top %d local maxima peaks ===\n", topN);
        for (int idx = 0; idx < topN && idx < static_cast<int>(peaks.size()); idx++)
        {
            std::wprintf(L"%d: (%.2f, %.2f, %.2f) -> amplitude = %.5f\n",
                idx, peaks[idx].kx, peaks[idx].ky, peaks[idx].kz, peaks[idx].amplitude);
        }
    }
    else std::wprintf(L"�߸��� ���� �ԷµǾ���.\n");
}