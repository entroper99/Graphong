#define _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS

#include <SDL.h>
#include <SDL_ttf.h>
#include <GL/glew.h>
#include <Eigen/Dense>
#include <codecvt>
#include <windows.h>
#include <fstream>
#include <string>
#include <iostream>
#include <fftw3.h>

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
import stdevCurvature;

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

int main(int argc, char** argv)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    bool debugMode = false;

    std::locale::global(std::locale("korean"));
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Graphong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_SetRelativeMouseMode(SDL_TRUE); //���콺 �����
    glewExperimental = GL_TRUE; //opneGL ���� ��� Ȱ��ȭ
    glewInit();
    glEnable(GL_DEPTH_TEST);
    TTF_Init();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    fftw_init_threads();
    fftw_make_planner_thread_safe();

    std::wprintf(L"\033[0;37m");
    std::wprintf(L"**********************************************************\n");
    std::wprintf(L"\033[0;33m");
    std::wprintf(L"Graphong v0.300\n");
    std::wprintf(L"\033[0;37m");
    std::wprintf(L"WASD : �̵�\n");
    std::wprintf(L"QE : ������\n");
    std::wprintf(L"X,Y,Z + ���콺�� : ������ ����\n");
    std::wprintf(L"\033[0;31m���� : X��\n");
    std::wprintf(L"\033[0;32m��� : Y��\n");
    std::wprintf(L"\033[0;34mû�� : Z��\033[0m\n");
    std::wprintf(L"Enter Ű�� ���� Ŀ�ǵ� �Է�\n");
    std::wprintf(L"**********************************************************\n");
    std::wprintf(L"\033[0m");

    TTF_Font* font = TTF_OpenFont("NanumGothic.ttf", 16);
    if (font == nullptr) std::wprintf(L"16�������� ��Ʈ �ε忡 �����Ͽ���.\n");

    TTF_Font* smallFont = TTF_OpenFont("NanumGothic.ttf", 10);
    if (smallFont == nullptr) std::wprintf(L"10�������� ��Ʈ �ε忡 �����Ͽ���.\n");

    bool quit = false;
    SDL_Event event;
    while (!quit)
    {
        const Uint8* state = SDL_GetKeyboardState(NULL);

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) quit = true;
            else if (event.type == SDL_MOUSEMOTION)//���콺 �̵�
            {
                camYaw += event.motion.xrel * mouseSensitivity;
                camPitch += event.motion.yrel * mouseSensitivity;
                if (camPitch > 89.0f) camPitch = 89.0f;
                if (camPitch < -89.0f) camPitch = -89.0f;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (debugMode)
                {
                    ((Func*)funcSet[0])->singleTriangulation();
                }
            }
            else if (event.type == SDL_MOUSEWHEEL)
            {
                if (state[SDL_SCANCODE_X]) {
                    if (event.wheel.y > 0) {
                        xScale *= 1.1;
                    }
                    else if (event.wheel.y < 0) {
                        xScale *= 0.9;
                    }
                }
                if (state[SDL_SCANCODE_Y]) {
                    if (event.wheel.y > 0) {
                        yScale *= 1.1;
                    }
                    else if (event.wheel.y < 0) {
                        yScale *= 0.9;
                    }
                }
                if (state[SDL_SCANCODE_Z]) {
                    if (event.wheel.y > 0) {
                        zScale *= 1.1;
                    }
                    else if (event.wheel.y < 0) {
                        zScale *= 0.9;
                    }
                }
            }
        }
        //Ű���� �Է�

        if (state[SDL_SCANCODE_RETURN])
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
            if (visDataPoint)  std::wprintf(L"14.�������� ȭ�� ǥ�� [ \033[0;32mON\033[0m / OFF ]\n");
            else std::wprintf(L"14.�������� ȭ�� ǥ��  [ ON / \033[1;31mOFF\033[0m ]\n");
            if (visInterPoint)  std::wprintf(L"15.������ ȭ�� ǥ�� [ \033[0;32mON\033[0m / OFF ]\n");
            else std::wprintf(L"15.������ ȭ�� ǥ��  [ ON / \033[1;31mOFF\033[0m ]\n");
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
            std::wprintf(L"\033[0;33m44. �������� ��� �м�(������ ���)\033[0m\n");
            std::wprintf(L"\033[0;33m45. �������� ��� ������׷� ���\033[0m\n");
            std::wprintf(L"\033[0;33m45. �������� ���۷��� ������׷� ���\033[0m\n");
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
                double length = BOX_SIZE / 2.0;
                double scaleFactor = 2.0 * M_PI / length;
                refGyroid->scalarFunc = [=](double x, double y, double z)->double
                    {
                        return (std::cos(scaleFactor * x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor * z)) + std::cos(scaleFactor * y) * std::sin(scaleFactor * z) * std::sin(2 * (scaleFactor * x)) + std::cos(scaleFactor * z) * std::sin(scaleFactor * x) * std::sin(2 * (scaleFactor * y)));
                    };
                refGyroid->latticeConstant = BOX_SIZE;// / 2.0;
                refGyroid->scalarInfimum = -1.0;// * std::sqrt(2);
                refGyroid->scalarSupremum = 1.0;// *std::sqrt(2);

                for (double x = -BOX_SIZE/2.0; x <= BOX_SIZE / 2.0; x += 0.78)
                {
                    for (double y = -BOX_SIZE / 2.0; y <= BOX_SIZE / 2.0; y += 0.78)
                    {
                        for (double z = -BOX_SIZE / 2.0; z <= BOX_SIZE / 2.0; z += 0.78)
                        {
                            if (refGyroid->scalarFunc(x,y,z) >= 0.8) refGyroid->myPoints.push_back({ x, y, z });
                        }
                    }
                }
                std::wprintf(L"�����ϴ� ���̷��̵� ���� ���ڴ� %d���̴�.\n", refGyroid->myPoints.size());
                refGyroid->scalarCalc();
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
                    std::wprintf(L"%.10f,%.10f,%.10f\n", i, pt.x, pt.y, pt.z);
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
                std::wifstream in(file);
                if (in.is_open())
                {
                    readTrjFile(file, 9, -1, 2, 3, 4, 1, 2);
                    Func* tgtFunc = ((Func*)funcSet[funcSet.size() - 1]);
                    tgtFunc->period = BOX_SIZE / 2.0;

                    //tgtFunc->myPoints.clear();
                    //orthogonal box = (-0.111315 -0.111315 -0.111315) to (10.7379 10.7379 10.7379)
                    //���� �Ѻ��� ������ 10.7379 - (-0.111315) = 10.849215
                    double length = BOX_SIZE / 2.0;
                    double scaleFactor = 2.0 * M_PI / length;
                    tgtFunc->scalarFunc = [=](double x, double y, double z)->double
                        {
                            return (std::cos(scaleFactor * x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor * z)) + std::cos(scaleFactor * y) * std::sin(scaleFactor * z) * std::sin(2 * (scaleFactor * x)) + std::cos(scaleFactor * z) * std::sin(scaleFactor * x) * std::sin(2 * (scaleFactor * y)));
                        };

                    tgtFunc->scalarInfimum = -1.0;
                    tgtFunc->scalarSupremum = 1.0;
                    tgtFunc->scalarCalc();
                    tgtFunc->latticeConstant = BOX_SIZE;// / 2.0;
                    tgtFunc->translation(-BOX_SIZE / 2.0, -BOX_SIZE / 2.0, -BOX_SIZE / 2.0);
                    std::wprintf(L"�� �Լ��� ��� f���� %f�̴�.\n", tgtFunc->scalarSquareAvg());
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
                        tgtFunc->myColor = rainbow((double)i*0.7 / (double)fourierTransSaveX.size());
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

                        Func* timeGraphFunc = new Func(funcFlag::scalarField);
                        timeGraphFunc->funcType = funcFlag::dim2;
                        timeGraphFunc->funcName = L"���";
                        std::wprintf(L"ù��° F_avg�� ���� ���� �ұ�?\n");
                        timeGraphFunc->myColor = rainbow(0);

                        Func* timeGraphFunc2 = new Func(funcFlag::scalarField);
                        timeGraphFunc2->funcType = funcFlag::dim2;
                        timeGraphFunc2->funcName = L"�л�";
                        std::wprintf(L"�ι�° F_avg�� ���� ���� �ұ�?\n");
                        timeGraphFunc2->myColor = rainbow(0.1);

                        Func* timeGraphFunc3 = new Func(funcFlag::scalarField);
                        timeGraphFunc3->funcType = funcFlag::dim2;
                        timeGraphFunc3->funcName = L"����";
                        std::wprintf(L"����° F_avg�� ���� ���� �ұ�?\n");
                        timeGraphFunc3->myColor = rainbow(0.2);

                        Func* timeGraphFunc4 = new Func(funcFlag::scalarField);
                        timeGraphFunc4->funcType = funcFlag::dim2;
                        timeGraphFunc4->funcName = L"÷��";
                        std::wprintf(L"�׹�° F_avg�� ���� ���� �ұ�?\n");
                        timeGraphFunc4->myColor = rainbow(0.35);

                        Func* timeGraphFunc5 = new Func(funcFlag::scalarField);
                        timeGraphFunc5->funcType = funcFlag::dim2;
                        timeGraphFunc5->funcName = L"�ֵ�";
                        std::wprintf(L"�ټ���° F_avg�� ���� ���� �ұ�?\n");
                        timeGraphFunc5->myColor = rainbow(0.5);

                        Func* timeGraphFunc6 = new Func(funcFlag::scalarField);
                        timeGraphFunc6->funcType = funcFlag::dim2;
                        timeGraphFunc6->funcName = L"�ƿ���";
                        std::wprintf(L"������° F_avg�� ���� ���� �ұ�?\n");
                        timeGraphFunc6->myColor = rainbow(0.7);

                        Func* originGraphFunc = new Func(funcFlag::scalarField);
                        originGraphFunc->funcType = funcFlag::dim2;
                        originGraphFunc->funcName = L"���� F��";
                        std::wprintf(L"���� F_avg�� ���� ���� �ұ�?\n");
                        originGraphFunc->myColor = { 0xff,0xff,0xff };



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

                            std::array<double, 6> result = tgtGyroid->calcCurvature(tgtGyroid->myPoints, tgtGyroid->latticeConstant, 128);
                            timeGraphFunc->myPoints.push_back({ (double)i,result[0],0 });
                            timeGraphFunc2->myPoints.push_back({ (double)i,result[1],0 });
                            timeGraphFunc3->myPoints.push_back({ (double)i,result[2],0 });
                            timeGraphFunc4->myPoints.push_back({ (double)i,result[3],0 });
                            timeGraphFunc5->myPoints.push_back({ (double)i,result[4],0 });
                            timeGraphFunc6->myPoints.push_back({ (double)i,result[5],0 });
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
            else if (input == 44)//������ ���
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


                        Func* timeGraphFuncStdev = new Func(funcFlag::scalarField);
                        timeGraphFuncStdev->funcType = funcFlag::dim2;
                        timeGraphFuncStdev->funcName = L"ǥ������";
                        timeGraphFuncStdev->myColor = rainbow(0.2);

                        Func* originGraphFunc = new Func(funcFlag::scalarField);
                        originGraphFunc->funcType = funcFlag::dim2;
                        originGraphFunc->funcName = L"���� F��";
                        originGraphFunc->myColor = { 0xff,0xff,0xff };

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
                            std::vector<std::array<double, 3>> tgtPoints;
                            for (int i = 0; i < tgtGyroid->myPoints.size(); i++) tgtPoints.push_back({ tgtGyroid->myPoints[i].x,tgtGyroid->myPoints[i].y,tgtGyroid->myPoints[i].z });
                            double result = calcStdevCurvature(tgtPoints, tgtGyroid->latticeConstant);
                            timeGraphFuncStdev->myPoints.push_back({ (double)i,result,0 });
                            std::wprintf(L"TIME %d : STDEV : %f\n", i,result);
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


                        Func* unorderFunc = new Func(funcFlag::scalarField);
                        unorderFunc->funcType = funcFlag::dim2;
                        unorderFunc->funcName = L"������";
                        unorderFunc->myColor = col::white;

                        Func* orderFunc = new Func(funcFlag::scalarField);
                        orderFunc->funcType = funcFlag::dim2;
                        orderFunc->funcName = L"���̷��̵�";
                        orderFunc->myColor = rainbow(0.2);

                        Func* orderFunc2 = new Func(funcFlag::scalarField);
                        orderFunc2->funcType = funcFlag::dim2;
                        orderFunc2->funcName = L"���̷��̵�2";
                        orderFunc2->myColor = rainbow(0.3);

                        Func* orderFunc3 = new Func(funcFlag::scalarField);
                        orderFunc3->funcType = funcFlag::dim2;
                        orderFunc3->funcName = L"���̷��̵�3";
                        orderFunc3->myColor = rainbow(0.4);

                        Func* refGyroid = new Func(funcFlag::scalarField);
                        refGyroid->funcType = funcFlag::dim2;
                        refGyroid->funcName = L"���̷��̵�(�ؼ�)";
                        refGyroid->myColor = rainbow(0.6);


                        int i = -1;
                        while (1)
                        {
                            if (i == -1)
                            {
                                Func* refGyroid = new Func(funcFlag::scalarField);
                                refGyroid->funcType = funcFlag::dim2;
                                refGyroid->funcName = L"���̷��̵�(�ؼ�)";
                                refGyroid->myColor = rainbow(0.6);
                                double length = BOX_SIZE / 2.0;
                                double scaleFactor = 2.0 * M_PI / length;
                                refGyroid->scalarFunc = [=](double x, double y, double z)->double
                                    {
                                        return (std::cos(scaleFactor * x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor * z)) + std::cos(scaleFactor * y) * std::sin(scaleFactor * z) * std::sin(2 * (scaleFactor * x)) + std::cos(scaleFactor * z) * std::sin(scaleFactor * x) * std::sin(2 * (scaleFactor * y)));
                                    };
                                refGyroid->latticeConstant = BOX_SIZE;// / 2.0;
                                for (double x = -BOX_SIZE / 2.0; x <= BOX_SIZE / 2.0; x += 0.78)
                                {
                                    for (double y = -BOX_SIZE / 2.0; y <= BOX_SIZE / 2.0; y += 0.78)
                                    {
                                        for (double z = -BOX_SIZE / 2.0; z <= BOX_SIZE / 2.0; z += 0.78)
                                        {
                                            if (refGyroid->scalarFunc(x, y, z) >= 0.8) refGyroid->myPoints.push_back({ x, y, z });
                                        }
                                    }
                                }
                                std::wprintf(L"�����ϴ� ���̷��̵� ���� ���ڴ� %d���̴�.\n", refGyroid->myPoints.size());
                                refGyroid->scalarCalc();
                                refGyroid->translation(BOX_SIZE / 2.0, BOX_SIZE / 2.0, BOX_SIZE / 2.0);
                            }
                            else readTrjString(str, 9, -1, 2, 3, 4, 1, atomType);

                            
                            Func* tgtGyroid = ((Func*)funcSet[funcSet.size() - 1]);
                            double length = BOX_SIZE / 2.0;
                            double scaleFactor = 2.0 * M_PI / length;
                            tgtGyroid->scalarFunc = [=](double x, double y, double z)->double
                                {
                                    return (std::cos(scaleFactor * x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor * z)) + std::cos(scaleFactor * y) * std::sin(scaleFactor * z) * std::sin(2 * (scaleFactor * x)) + std::cos(scaleFactor * z) * std::sin(scaleFactor * x) * std::sin(2 * (scaleFactor * y)));
                                };
                            tgtGyroid->latticeConstant = BOX_SIZE;// / 2.0;
                            tgtGyroid->translation(-BOX_SIZE / 2.0, -BOX_SIZE / 2.0, -BOX_SIZE / 2.0);
                            tgtGyroid->scalarCalc();
                            double originF = tgtGyroid->scalarSquareAvg();


                            double xAngle = 0, yAngle = 0, zAngle = 0;
                            Eigen::Vector3d transVec = { 0,0,0 };
                            
                            if (i != -1)
                            {
                                double lat = tgtGyroid->latticeConstant;
                                transVec = { randomRangeFloat(-lat / 2.0,lat / 2.0),randomRangeFloat(-lat / 2.0,lat / 2.0),randomRangeFloat(-lat / 2.0,lat / 2.0) };
                                tgtGyroid->latticeTranslation(tgtGyroid->myPoints, tgtGyroid->latticeConstant, transVec); //���� �����̵�

                                xAngle = randomRangeFloat(0, 360.0);
                                yAngle = randomRangeFloat(0, 360.0);
                                zAngle = randomRangeFloat(0, 360.0);

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
                            }

                            if (i == 0 || i == 150 || i == 200 || i == 230 ||i==-1)
                            {
                                std::wprintf(L"TIME %d : ���� �����̵� : (%f,%f,%f), ���� ȸ�� : (%f,%f,%f)\n", i, transVec[0], transVec[1], transVec[2], xAngle, yAngle, zAngle);
                                std::vector<std::array<double, 3>> tgtPoints;
                                for (int i = 0; i < tgtGyroid->myPoints.size(); i++) tgtPoints.push_back({ tgtGyroid->myPoints[i].x,tgtGyroid->myPoints[i].y,tgtGyroid->myPoints[i].z });
                                std::vector<std::array<double, 3>> result = calcLaplacianHistogram(tgtPoints, tgtGyroid->latticeConstant);
                                //std::wprintf(L"���� w���� %f�̴�.\n", calcLaplacianWasserstein(tgtPoints, tgtGyroid->latticeConstant));
                                for (int j = 0; j < result.size(); j++)
                                {
                                    if (i == -1) std::wprintf(L"ref[%d] = { %f,%f,0 };\n", j, result[j][0], result[j][1]);
                                    //std::wprintf(L"�� (%f,%f,%f)�� �Լ��� �־���.\n", result[j][0], result[j][1], 0);
                                    if (i == 0) unorderFunc->myPoints.push_back({ result[j][0],result[j][1],0 });
                                    else if (i == 150) orderFunc->myPoints.push_back({ result[j][0],result[j][1],0 });
                                    else if (i == 200) orderFunc2->myPoints.push_back({ result[j][0],result[j][1],0 });
                                    else if (i == 230) orderFunc3->myPoints.push_back({ result[j][0],result[j][1],0 });
                                    else if ( i == -1) refGyroid->myPoints.push_back({ result[j][0],result[j][1],0 });
                                }
                            }

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
                Func* refGyroid = new Func(funcFlag::scalarField);
                refGyroid->funcType = funcFlag::dim2;
                refGyroid->funcName = L"���̷��̵�(�ؼ�)";
                refGyroid->myColor = rainbow(0.6);

                double length = BOX_SIZE / 2.0;
                double scaleFactor = 2.0 * M_PI / length;
                refGyroid->scalarFunc = [=](double x, double y, double z)->double
                    {
                        return (std::cos(scaleFactor * x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor * z)) + std::cos(scaleFactor * y) * std::sin(scaleFactor * z) * std::sin(2 * (scaleFactor * x)) + std::cos(scaleFactor * z) * std::sin(scaleFactor * x) * std::sin(2 * (scaleFactor * y)));
                    };
                refGyroid->latticeConstant = BOX_SIZE;
                for (double x = -BOX_SIZE / 4.0; x <= BOX_SIZE / 4.0; x += 0.78)
                {
                    for (double y = -BOX_SIZE / 4.0; y <= BOX_SIZE / 4.0; y += 0.78)
                    {
                        for (double z = -BOX_SIZE / 4.0; z <= BOX_SIZE / 4.0; z += 0.78)
                        {
                            if (refGyroid->scalarFunc(x, y, z) >= 0.8) refGyroid->myPoints.push_back({ x, y, z });
                        }
                    }
                }
                std::wprintf(L"�����ϴ� ���̷��̵� ���� ���ڴ� %d���̴�.\n", refGyroid->myPoints.size());
                std::vector<std::array<double, 3>> tgtPoints;
                for (int i = 0; i < refGyroid->myPoints.size(); i++) tgtPoints.push_back({ refGyroid->myPoints[i].x,refGyroid->myPoints[i].y,refGyroid->myPoints[i].z });
                std::vector<std::array<double, 3>> result = calcLaplacianHistogram(tgtPoints, BOX_SIZE / 2.0);
                refGyroid->myPoints.clear();
                for (int j = 0; j < result.size(); j++)
                {
                    std::wprintf(L"ref[%d] = { %f,%f,0 };\n", j, result[j][0], result[j][1]);
                    refGyroid->myPoints.push_back({ result[j][0], result[j][1],0 });
                }
            }
            else if (input == 47)
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

                        Func* unorderFunc = new Func(funcFlag::scalarField);
                        unorderFunc->funcType = funcFlag::dim2;
                        unorderFunc->funcName = L"������";
                        unorderFunc->myColor = col::white;

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
                            tgtGyroid->latticeConstant = BOX_SIZE;// / 2.0;
                            tgtGyroid->translation(-BOX_SIZE / 2.0, -BOX_SIZE / 2.0, -BOX_SIZE / 2.0);
                            tgtGyroid->scalarCalc();
                            double originF = tgtGyroid->scalarSquareAvg();

                            double xAngle = 0, yAngle = 0, zAngle = 0;
                            Eigen::Vector3d transVec = { 0,0,0 };

                            double lat = tgtGyroid->latticeConstant;
                            transVec = { randomRangeFloat(-lat / 2.0,lat / 2.0),randomRangeFloat(-lat / 2.0,lat / 2.0),randomRangeFloat(-lat / 2.0,lat / 2.0) };
                            tgtGyroid->latticeTranslation(tgtGyroid->myPoints, tgtGyroid->latticeConstant, transVec); //���� �����̵�

                            xAngle = randomRangeFloat(0, 360.0);
                            yAngle = randomRangeFloat(0, 360.0);
                            zAngle = randomRangeFloat(0, 360.0);

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

                            std::vector<Point> newPoints;
                            for (size_t j = 0; j < tgtGyroid->myPoints.size(); j++)
                            {
                                if (tgtGyroid->myPoints[j].x < BOX_SIZE / 4.0 && tgtGyroid->myPoints[j].x > -BOX_SIZE / 4.0)
                                {
                                    if (tgtGyroid->myPoints[j].y < BOX_SIZE / 4.0 && tgtGyroid->myPoints[j].y > -BOX_SIZE / 4.0)
                                    {
                                        if (tgtGyroid->myPoints[j].z < BOX_SIZE / 4.0 && tgtGyroid->myPoints[j].z > -BOX_SIZE / 4.0)
                                        {
                                            newPoints.push_back({ tgtGyroid->myPoints[j].x,tgtGyroid->myPoints[j].y,tgtGyroid->myPoints[j].z });
                                        }
                                    }
                                }
                            }
                            tgtGyroid->myPoints = newPoints;

                            if (i == 0)
                            {
                                std::vector<std::array<double, 3>> tgtPoints;
                                for (int i = 0; i < tgtGyroid->myPoints.size(); i++) tgtPoints.push_back({ tgtGyroid->myPoints[i].x,tgtGyroid->myPoints[i].y,tgtGyroid->myPoints[i].z });
                                std::vector<std::array<double, 3>> result = calcLaplacianHistogram(tgtPoints, BOX_SIZE/2.0);
                                //std::wprintf(L"���� w���� %f�̴�.\n", calcLaplacianWasserstein(tgtPoints, tgtGyroid->latticeConstant));
                                for (int j = 0; j < result.size(); j++)
                                {
                                    if (i == 0) unorderFunc->myPoints.push_back({ result[j][0],result[j][1],0 });
                                }
                            }

                            delete tgtGyroid;

                            size_t firstTimestepPos = str.find("ITEM: TIMESTEP");
                            size_t secondTimestepPos = str.find("ITEM: TIMESTEP", firstTimestepPos + 1);
                            if (secondTimestepPos == std::string::npos) break;
                            else str = str.substr(secondTimestepPos);


                            break;
                        }
                    }
                }
                }
            else std::wprintf(L"�߸��� ���� �ԷµǾ���.\n");
        }



        if (camFixX == true)
        {
            camYaw = 90;
            camPitch = 0;
        }
        else if (camFixMinusX == true)
        {
            camYaw = -90;
            camPitch = 0;
        }
        else if (camFixY == true)
        {
            camYaw = 0;
            camPitch = -90;
        }
        else if (camFixMinusY == true)
        {
            camYaw = 0;
            camPitch = 90;
        }
        else if (camFixZ == true)
        {
            camYaw = 180;
            camPitch = 0;

        }
        else if (camFixMinusZ == true)
        {
            camYaw = 0;
            camPitch = 0;
        }

        if (state[SDL_SCANCODE_W])
        {
            camX += camSpd * sin(camYaw * M_PI / 180.0);
            camZ -= camSpd * cos(camYaw * M_PI / 180.0);
        }
        if (state[SDL_SCANCODE_S])
        {
            camX -= camSpd * sin(camYaw * M_PI / 180.0);
            camZ += camSpd * cos(camYaw * M_PI / 180.0);
        }
        if (state[SDL_SCANCODE_A])
        {
            camX -= camSpd * cos(camYaw * M_PI / 180.0);
            camZ -= camSpd * sin(camYaw * M_PI / 180.0);
        }
        if (state[SDL_SCANCODE_D])
        {
            camX += camSpd * cos(camYaw * M_PI / 180.0);
            camZ += camSpd * sin(camYaw * M_PI / 180.0);
        }

        if (state[SDL_SCANCODE_X])
        {
        }

        if (state[SDL_SCANCODE_Q]) camY += camSpd;
        if (state[SDL_SCANCODE_E]) camY -= camSpd;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0f, (float)640 / (float)480, 0.1f, 100.0f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(camPitch, 1.0f, 0.0f, 0.0f);
        glRotatef(camYaw, 0.0f, 1.0f, 0.0f);
        glTranslatef(-camX, -camY, -camZ);

        float axisLength = 10000.0f;

        // x�� �׸���
        if (!camFixX && !camFixMinusX)
        {
            glBegin(GL_LINES);
            glColor3f(1.0, 0, 0);
            glVertex2f(-axisLength / 2.0, 0.0);
            glVertex2f(+axisLength / 2.0, 0.0);
            glEnd();

            glColor3f(1.0, 1.0, 1.0);
            for (float x = -axisLength / 2.0; x <= axisLength / 2.0; x += 1.0) {
                glBegin(GL_LINES);
                if (x == 0);
                else if (fmod(x, 10.0) == 0.0)
                {
                    glColor3f(1.0, 0, 0);
                    glVertex3f(x, -0.4, 0.0);
                    glVertex3f(x, 0.4, 0.0);
                }
                else
                {
                    glColor3f(1.0, 0, 0);
                    glVertex3f(x, -0.1, 0.0);
                    glVertex3f(x, 0.1, 0.0);
                }
                glEnd();
            }
        }

        if (!camFixY && !camFixMinusY)
        {
            // y�� �׸���
            glBegin(GL_LINES);
            glColor3f(0.0, 1.0, 0.0);
            glVertex3f(0.0, -axisLength / 2.0, 0.0);
            glVertex3f(0.0, +axisLength / 2.0, 0.0);
            glEnd();

            glColor3f(1.0, 1.0, 1.0);
            for (float y = -axisLength / 2.0; y <= axisLength / 2.0; y += 1.0) {
                glBegin(GL_LINES);
                if (y == 0);
                else if (fmod(y, 10.0) == 0.0)
                {
                    glColor3f(0.0, 1.0, 0.0);
                    glVertex3f(-0.4, y, 0.0);
                    glVertex3f(0.4, y, 0.0);
                }
                else
                {
                    glColor3f(0.0, 1.0, 0.0);
                    glVertex3f(-0.1, y, 0.0);
                    glVertex3f(0.1, y, 0.0);
                }
                glEnd();
            }
        }

        if (!camFixZ && !camFixMinusZ)
        {
            // z�� �׸���
            glBegin(GL_LINES);
            glColor3f(0.0, 0.0, 1.0);
            glVertex3f(0.0, 0.0, -axisLength / 2.0);
            glVertex3f(0.0, 0.0, +axisLength / 2.0);
            glEnd();

            glColor3f(1.0, 1.0, 1.0);
            for (float z = -axisLength / 2.0; z <= axisLength / 2.0; z += 1.0) {
                glBegin(GL_LINES);
                if (z == 0);
                else if (fmod(z, 10.0) == 0.0)
                {
                    glColor3f(0.0, 0.0, 1.0);
                    glVertex3f(0.0, -0.4, z);
                    glVertex3f(0.0, 0.4, z);
                }
                else
                {
                    glColor3f(0.0, 0.0, 1.0);
                    glVertex3f(0.0, -0.1, z);
                    glVertex3f(0.0, 0.1, z);
                }
                glEnd();
            }
        }

        float zeroX = 1, zeroY = 1, zeroZ = 1;
        if (camFixZ || camFixMinusZ) zeroZ = 0;
        if (camFixY || camFixMinusY) zeroY = 0;
        if (camFixX || camFixMinusX) zeroX = 0;

        //�������� �׸���
        if (visDataPoint)
        {
            for (int dataIndex = 0; dataIndex < funcSet.size(); dataIndex++)
            {
                Func* tgtFunc = (Func*)funcSet[dataIndex];
                for (int i = 0; i < tgtFunc->myPoints.size(); i++)
                {
                    glPointSize(pointSize);
                    glBegin(GL_POINTS);


                    if (tgtFunc->funcType == funcFlag::scalarField)
                    {
                        SDL_Color col;
                        double val = tgtFunc->scalar[tgtFunc->myPoints[i]];
                        if (val < tgtFunc->scalarInfimum)
                        {
                            col = rainbow(0);
                        }
                        else if (val > tgtFunc->scalarSupremum)
                        {
                            col = rainbow(0.7);
                        }
                        else
                        {
                            //std::wprintf(L"�÷��� ���� %f�̴�.\n", 0.7 * ((val - tgtFunc->scalarInfimum) / (tgtFunc->scalarSupremum - tgtFunc->scalarInfimum)));
                            col = rainbow(0.7 * ((val - tgtFunc->scalarInfimum) / (tgtFunc->scalarSupremum - tgtFunc->scalarInfimum)));
                        }
                        glColor3f(((float)col.r) / 256.0, ((float)col.g) / 256.0, ((float)col.b) / 256.0);
                    }
                    else
                    {
                        glColor3f(((float)tgtFunc->myColor.r) / 256.0, ((float)tgtFunc->myColor.g) / 256.0, ((float)tgtFunc->myColor.b) / 256.0);
                    }

                    glVertex3f(zeroX * xScale * (tgtFunc->myPoints[i].x), zeroY * yScale * (tgtFunc->myPoints[i].y), zeroZ * zScale * (tgtFunc->myPoints[i].z));
                    glEnd();
                }
            }
        }

        //������ �׸���
        if (visInterPoint)
        {
            for (int dataIndex = 0; dataIndex < funcSet.size(); dataIndex++)
            {
                Func* tgtFunc = (Func*)funcSet[dataIndex];
                for (int i = 0; i < tgtFunc->myInterPoints.size(); i++)
                {
                    glPointSize(pointSize);
                    glBegin(GL_POINTS);
                    glColor3f(((float)tgtFunc->myColor.r) / 256.0 / 3.0, ((float)tgtFunc->myColor.g) / 256.0 / 3.0, ((float)tgtFunc->myColor.b) / 256.0 / 3.0);
                    glVertex3f(zeroX * xScale * (tgtFunc->myInterPoints[i].x), zeroY * yScale * (tgtFunc->myInterPoints[i].y), zeroZ * zScale * (tgtFunc->myInterPoints[i].z));
                    glEnd();
                }
            }
        }


        //������ �׸��� 
        for (int dataIndex = 0; dataIndex < funcSet.size(); dataIndex++)
        {
            Func* tgtFunc = (Func*)funcSet[dataIndex];
            if (tgtFunc->interLine == true)
            {
                if (tgtFunc->myInterPoints.size() >= 2)
                {
                    for (int i = 0; i < tgtFunc->myInterPoints.size() - 1; i++)
                    {
                        // ���� �׸���
                        glBegin(GL_LINES);
                        glColor3f(((float)tgtFunc->myColor.r) / 256.0, ((float)tgtFunc->myColor.g) / 256.0, ((float)tgtFunc->myColor.b) / 256.0);
                        glVertex3f(zeroX * xScale * (tgtFunc->myInterPoints[i].x), zeroY * yScale * (tgtFunc->myInterPoints[i].y), zeroZ * zScale * (tgtFunc->myInterPoints[i].z));
                        glVertex3f(zeroX * xScale * (tgtFunc->myInterPoints[i + 1].x), zeroY * yScale * (tgtFunc->myInterPoints[i + 1].y), zeroZ * zScale * (tgtFunc->myInterPoints[i + 1].z));
                        glEnd();
                    }
                }
            }
        }

        //���ڻ�� ��� �׸���
        for (int dataIndex = 0; dataIndex < funcSet.size(); dataIndex++)
        {
            Func* tgtFunc = (Func*)funcSet[dataIndex];
            if (tgtFunc->latticeConstant != 0)
            {
                // ���� �׸���
                glBegin(GL_LINES);
                glColor3f(0.0 / 256.0, 255.0 / 256.0, 255.0 / 256.0);

                if (camFixMinusZ || camFixZ)
                {
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, 0.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, 0.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, 0.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, 0.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, 0.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, 0.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, 0.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, 0.0);
                }
                else if (camFixMinusX || camFixX)
                {
                    glVertex3f(0.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(0.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(0.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(0.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(0.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(0.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(0.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(0.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                }
                else if (camFixMinusY || camFixY)
                {
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, 0.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, 0.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, 0.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, 0.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, 0.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, 0.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, 0.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, 0.0, -tgtFunc->latticeConstant / 2.0);
                }
                else
                {
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                    glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
                }
                glEnd();
            }
        }

        //�ﰢ���� �׸���
        for (int dataIndex = 0; dataIndex < funcSet.size(); dataIndex++)
        {
            Func* tgtFunc = (Func*)funcSet[dataIndex];
            for (int i = 0; i < tgtFunc->triangles.size(); i++)
            {
                glBegin(GL_LINES);
                glColor3f(((float)tgtFunc->myColor.r) / 256.0, ((float)tgtFunc->myColor.g) / 256.0, ((float)tgtFunc->myColor.b) / 256.0);
                glVertex3f(tgtFunc->triangles[i].p1.x * zeroX * xScale, tgtFunc->triangles[i].p1.y * zeroY * yScale, tgtFunc->triangles[i].p1.z * zeroZ * zScale);
                glVertex3f(tgtFunc->triangles[i].p2.x * zeroX * xScale, tgtFunc->triangles[i].p2.y * zeroY * yScale, tgtFunc->triangles[i].p2.z * zeroZ * zScale);
                glEnd();

                glBegin(GL_LINES);
                glColor3f(((float)tgtFunc->myColor.r) / 256.0, ((float)tgtFunc->myColor.g) / 256.0, ((float)tgtFunc->myColor.b) / 256.0);
                glVertex3f(tgtFunc->triangles[i].p1.x * zeroX * xScale, tgtFunc->triangles[i].p1.y * zeroY * yScale, tgtFunc->triangles[i].p1.z * zeroZ * zScale);
                glVertex3f(tgtFunc->triangles[i].p3.x * zeroX * xScale, tgtFunc->triangles[i].p3.y * zeroY * yScale, tgtFunc->triangles[i].p3.z * zeroZ * zScale);
                glEnd();

                glBegin(GL_LINES);
                glColor3f(((float)tgtFunc->myColor.r) / 256.0, ((float)tgtFunc->myColor.g) / 256.0, ((float)tgtFunc->myColor.b) / 256.0);
                glVertex3f(tgtFunc->triangles[i].p2.x * zeroX * xScale, tgtFunc->triangles[i].p2.y * zeroY * yScale, tgtFunc->triangles[i].p2.z * zeroZ * zScale);
                glVertex3f(tgtFunc->triangles[i].p3.x * zeroX * xScale, tgtFunc->triangles[i].p3.y * zeroY * yScale, tgtFunc->triangles[i].p3.z * zeroZ * zScale);
                glEnd();


                //glBegin(GL_TRIANGLES);
                //glColor3f(((float)tgtFunc->myColor.r) / 256.0 / 4.0, ((float)tgtFunc->myColor.g) / 256.0 / 4.0, ((float)tgtFunc->myColor.b) / 256.0 / 4.0);
                //glVertex3f(tgtFunc->triangles[i].p1.x* xScale, tgtFunc->triangles[i].p1.y* yScale, tgtFunc->triangles[i].p1.z* zScale);
                //glVertex3f(tgtFunc->triangles[i].p2.x* xScale, tgtFunc->triangles[i].p2.y* yScale, tgtFunc->triangles[i].p2.z* zScale);
                //glVertex3f(tgtFunc->triangles[i].p3.x* xScale, tgtFunc->triangles[i].p3.y* yScale, tgtFunc->triangles[i].p3.z* zScale);

                glEnd();

            }
        }


        glColor3f(1.0, 1.0, 1.0);

        std::wstring scaleName;

        if (xScaleUnit.size() > 0) scaleName = L" (" + xScaleUnit + L")";
        else scaleName.clear();
        drawTextHUD(L"X-scale : " + std::to_wstring(xScale) + scaleName, font, { 255,255,255 }, 10, 10);

        if (yScaleUnit.size() > 0) scaleName = L" (" + yScaleUnit + L")";
        else scaleName.clear();
        drawTextHUD(L"Y-scale : " + std::to_wstring(yScale) + scaleName, font, { 255,255,255 }, 10, 10 + 18);

        if (zScaleUnit.size() > 0) scaleName = L" (" + zScaleUnit + L")";
        else scaleName.clear();
        drawTextHUD(L"Z-scale : " + std::to_wstring(zScale) + scaleName, font, { 255,255,255 }, 10, 10 + 18 * 2);

        std::string axisName;
        if (xAxisName.size() > 0) axisName = " (" + xAxisName + ")";
        else axisName.clear();
        drawTextHUD("Cam-X : " + std::to_string(camX) + axisName, font, { 255,255,255 }, 10, 70 + 10);
        if (yAxisName.size() > 0) axisName = " (" + yAxisName + ")";
        else axisName.clear();
        drawTextHUD("Cam-Y : " + std::to_string(camY) + axisName, font, { 255,255,255 }, 10, 70 + 10 + 18);
        if (zAxisName.size() > 0) axisName = " (" + zAxisName + ")";
        else axisName.clear();
        drawTextHUD("Cam-Z : " + std::to_string(camZ) + axisName, font, { 255,255,255 }, 10, 70 + 10 + 18 * 2);



        if (camFixMinusZ || camFixZ || camFixMinusX || camFixX || camFixMinusY || camFixY)
        {
            drawTextHUD("X-Coord : " + std::to_string(camX / xScale), smallFont, { 255,255,255 }, 320 + 10, 240 + 10);
            drawTextHUD("Y-Coord : " + std::to_string(camY / yScale), smallFont, { 255,255,255 }, 320 + 10, 240 + 20);
            drawTextHUD("Z-Coord : " + std::to_string(camZ / zScale), smallFont, { 255,255,255 }, 320 + 10, 240 + 30);
        }



        drawTextHUD(graphName, font, { 255,255,255 }, 10, 480 - 30);

        //ũ�ν� ���
        if (crosshair == true)
        {
            int cX = 320;
            int cY = 240;
            int size = 10;

            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, 640, 480, 0, -1, 1);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            glColor3f(1.0f, 1.0f, 1.0f);  // ���

            //��
            glPointSize(3.0f);
            glBegin(GL_POINTS);
            glVertex2i(cX, cY);
            glEnd();


            //���� 4��
            glLineWidth(2.0f);
            glBegin(GL_LINES);
            glVertex2i(cX, cY - size);
            glVertex2i(cX, cY - 5);
            glVertex2i(cX, cY + 5);
            glVertex2i(cX, cY + size);
            glVertex2i(cX - size, cY);
            glVertex2i(cX - 5, cY);
            glVertex2i(cX + 5, cY);
            glVertex2i(cX + size, cY);
            glEnd();

            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
        }

        SDL_GL_SwapWindow(window);
    }

    TTF_Quit();
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}


