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

import std;
import globalVar;
import constVar;
import drawer;
import read;
import rainbow;
import cubicSpline;
import inputCol;
import Func;


std::wstring openFileDialog() 
{
    WCHAR filename[MAX_PATH];

    OPENFILENAME ofn;
    ZeroMemory(&filename, sizeof(filename));
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL; 
    ofn.lpstrFilter = L"All Files\0*.*\0Text Files\0*.TXT\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = L"�����Ͱ� �ִ� ������ �������ּ���.";
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
    if (GetOpenFileName(&ofn)) return std::wstring(filename);
    else return L"";
}

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

            std::wprintf(L"9. �Լ� Translation (�����̵�)\n");
            std::wprintf(L"10. �Լ� Rotation (ȸ��)\n");

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
            std::wprintf(L"25.�߽����� ���� �Լ� ǥ��ȭ\n");
            std::wprintf(L"26.PDE Solver\n");
            std::wprintf(L"27.LAMMPS Trajectory ���� �б�(Ÿ�ӽ��� 0)\n");
            std::wprintf(L"28.LAMMPS Trajectory ���� �б�(��� Ÿ�ӽ���)\n");
            //std::wprintf(L"\033[37m");
            //std::wprintf(L"101.[Plumed] COLVAR : draw time-# Graph \n");
            //std::wprintf(L"102.[Plumed] COLVAR : draw time-biasPot Graph \n");
            //std::wprintf(L"103.[Plumed] FES : draw FES result \n");
            //std::wprintf(L"104.[Plumed] FES : draw FES by time \n");
            //std::wprintf(L"105.[Plumed] Sigma - Probability Density \n");
            //std::wprintf(L"106.[Plumed] Sigma - Overlap \n");
            //std::wprintf(L"\033[0m");
            std::wprintf(L"------------------��Ʒ��� �� �Է�-----------------\n");

            int input = 0;
            std::cin >> input;
            if (input == 1)
            {
                std::wstring file = L"";
                std::wprintf(L"�����Ͱ� �ִ� ������ �������ּ���.\n");
                file = openFileDialog();
                std::wprintf(L"���� %ls �� ������� �����Ͽ���.\n",file.c_str());
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


                ((Func*)funcSet[dataIndex])->translation(compX,compY,compZ);
                ((Func*)funcSet[dataIndex])->scalarCalc();
                std::wprintf(L"�� �Լ��� ��� f���� %f�̴�.\n", ((Func*)funcSet[dataIndex])->scalarAvg());
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

                {
                    Eigen::Matrix3d checkMat = rotationMatrix;
                    double trace = checkMat.trace();
                    double theta = std::acos((trace - 1) / 2);
                    Eigen::Vector3d axis;
                    axis << checkMat(2, 1) - checkMat(1, 2),
                        checkMat(0, 2) - checkMat(2, 0),
                        checkMat(1, 0) - checkMat(0, 1);
                    axis.normalize();
                    std::cout << "ȸ���� : " << theta * 180.0 / M_PI << std::endl;
                    std::cout << "ȸ����: (" << axis.x() << ", " << axis.y() << ", " << axis.z() << ")" << std::endl;
                }



                ((Func*)funcSet[dataIndex])->rotation(rotationMatrix);
                ((Func*)funcSet[dataIndex])->scalarCalc();
                std::wprintf(L"�� �Լ��� ��� f���� %f�̴�.\n", ((Func*)funcSet[dataIndex])->scalarAvg());
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
                if (interLine == false)
                {
                    std::wprintf(L"���������� �������� �����Ͽ���.\n");
                    interLine = true;
                }
                else
                {
                    std::wprintf(L"���������� ������ ȭ�鿡�� �����.\n");
                    interLine = false;
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
                Func* targetFunc = new Func(funcFlag::scalarField);
                int numPoints = 40; // ������ �����ϴ� ���� ��, �������� ��Ȯ����
                double spacing = (2.0 * M_PI) / numPoints; //������ ����, ���� ���ϸ� ���ǰ� ��
                targetFunc->scalarFunc = [](double x, double y, double z)->double
                    {
                        return std::sqrt(8.0 / 3.0) * (std::cos(x - M_PI) * std::sin(y - M_PI) * std::sin(2 * (z - M_PI)) + std::cos(y - M_PI) * std::sin(z - M_PI) * std::sin(2 * (x - M_PI)) + std::cos(z - M_PI) * std::sin(x - M_PI) * std::sin(2 * (y - M_PI)));
                    };

                targetFunc->scalarInfimum = -2.0;// * std::sqrt(2);
                targetFunc->scalarSupremum = 2.0;// *std::sqrt(2);

                for (int i = -numPoints/2; i < numPoints/2; ++i)
                {
                    for (int j = -numPoints/2; j < numPoints/2; ++j)
                    {
                        for (int k = -numPoints/2; k < numPoints/2; ++k)
                        {
                            double x = i * spacing;
                            double y = j * spacing;
                            double z = k * spacing;
                            double value = std::sqrt(8.0 / 3.0) * (std::cos(x - M_PI) * std::sin(y - M_PI) * std::sin(2 * (z - M_PI)) + std::cos(y - M_PI) * std::sin(z - M_PI) * std::sin(2 * (x - M_PI)) + std::cos(z - M_PI) * std::sin(x - M_PI) * std::sin(2 * (y - M_PI)));
                            double cutoff = 1.0; //�� �� �̻��� ���̷��̵常 ȭ�鿡 ǥ�õ�
                            if (1)//(value >= cutoff)
                            {
                                targetFunc->myPoints.push_back({ x, y, z });
                                targetFunc->scalar[{x, y, z}] = value;
                                //std::wprintf(L"������ {%f,%f,%f}�� val���� %f�̴�.\n", x, y, z, value);
                            }
                            //std::wprintf(L"������ {%f,%f,%f}�� �Լ� %d�� �Է��ߴ�.\n", x, y, z, funcSet.size() - 1);
                        }
                    }
                }
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
                    std::wprintf(L"Index %d : {%.10f,%.10f,%.10f}\n", i, pt.x, pt.y, pt.z);
                }
            }
            else if (input == 25) //������ ǥ��ȭ
            {
                int dataIndex = 0;
                std::wprintf(L"���° �����͸� ǥ��ȭ��ų��? (0 ~ %d).\n", funcSet.size() - 1);
                prtFuncName();
                std::cin >> dataIndex;
                ((Func*)funcSet[dataIndex])->normalize(GYROID_PERIOD);
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
            else if (input == 27)//trajectory �б�
            {
                std::wstring file = L"";
                std::wprintf(L"�����Ͱ� �ִ� ������ �������ּ���.\n");
                file = openFileDialog();
                std::wprintf(L"���� %ls �� ������� �����Ͽ���.\n", file.c_str());
                std::wifstream in(file);
                if (in.is_open())
                {
                    readTrjFile(file, 9, -1, 2, 3, 4,1,2);
                    Func* tgtFunc = ((Func*)funcSet[funcSet.size() - 1]);
                    tgtFunc->period = GYROID_PERIOD;

                    //tgtFunc->myPoints.clear();
                    //orthogonal box = (-0.111315 -0.111315 -0.111315) to (10.7379 10.7379 10.7379)
                    //���� �Ѻ��� ������ 10.7379 - (-0.111315) = 10.849215
                    double length = GYROID_PERIOD;
                    double scaleFactor = 2.0 * M_PI / length;

                    tgtFunc->scalarFunc = [=](double x, double y, double z)->double
                        {
                            return (std::cos(scaleFactor *x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor *z)) + std::cos(scaleFactor *y) * std::sin(scaleFactor *z) * std::sin(2 * (scaleFactor *x)) + std::cos(scaleFactor *z) * std::sin(scaleFactor *x) * std::sin(2 * (scaleFactor *y)));
                        };

                    tgtFunc->scalarInfimum = -1.0;
                    tgtFunc->scalarSupremum = 1.0;
                    tgtFunc->scalarCalc();
                    std::wprintf(L"�� �Լ��� ��� f���� %f�̴�.\n", tgtFunc->scalarAvg());
                }
                else std::wprintf(L"������ �дµ� �����Ͽ����ϴ�.\n");
            }
            else if (input == 28)//trajectory �б�(��� Ÿ�ӽ���)
            {
                for(int atomType = 1; atomType <=2; atomType++)
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
                        timeGraphFunc->myColor = inputCol();

                        int i = 0;
                        while (1)
                        {
                            readTrjString(str, 9, -1, 2, 3, 4, 1, atomType);
                            Func* tgtGyroid = ((Func*)funcSet[funcSet.size() - 1]);
                            tgtGyroid->normalize(GYROID_PERIOD);
                            tgtGyroid->scalarCalc();
                            timeGraphFunc->myPoints.push_back({ (double)i,tgtGyroid->scalarSquareAvg(),0 });
                            delete tgtGyroid;

                            size_t firstTimestepPos = str.find("ITEM: TIMESTEP");
                            size_t secondTimestepPos = str.find("ITEM: TIMESTEP", firstTimestepPos + 1);
                            if (secondTimestepPos == std::string::npos) break;
                            else str = str.substr(secondTimestepPos);
                            i++;
                        }
                    }
                    else std::wprintf(L"������ �дµ� �����Ͽ����ϴ�.\n");

                    Func::hasTransform = false;
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
                           col = rainbow(0.7* ((val - tgtFunc->scalarInfimum) / (tgtFunc->scalarSupremum - tgtFunc->scalarInfimum)));
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
        if (interLine)
        {
            for (int dataIndex = 0; dataIndex < funcSet.size(); dataIndex++)
            {
                Func* tgtFunc = (Func*)funcSet[dataIndex];
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

        //�ﰢ���� �׸���
        for (int dataIndex = 0; dataIndex < funcSet.size(); dataIndex++)
        {
            Func* tgtFunc = (Func*)funcSet[dataIndex];
            for (int i = 0; i < tgtFunc->triangles.size(); i++)
            {
                glBegin(GL_LINES);
                glColor3f(((float)tgtFunc->myColor.r) / 256.0, ((float)tgtFunc->myColor.g) / 256.0, ((float)tgtFunc->myColor.b) / 256.0);
                glVertex3f(tgtFunc->triangles[i].p1.x * zeroX* xScale, tgtFunc->triangles[i].p1.y * zeroY* yScale, tgtFunc->triangles[i].p1.z * zeroZ* zScale);
                glVertex3f(tgtFunc->triangles[i].p2.x * zeroX* xScale, tgtFunc->triangles[i].p2.y * zeroY* yScale, tgtFunc->triangles[i].p2.z * zeroZ* zScale);
                glEnd();

                glBegin(GL_LINES);
                glColor3f(((float)tgtFunc->myColor.r) / 256.0, ((float)tgtFunc->myColor.g) / 256.0, ((float)tgtFunc->myColor.b) / 256.0);
                glVertex3f(tgtFunc->triangles[i].p1.x * zeroX* xScale, tgtFunc->triangles[i].p1.y * zeroY* yScale, tgtFunc->triangles[i].p1.z * zeroZ* zScale);
                glVertex3f(tgtFunc->triangles[i].p3.x * zeroX* xScale, tgtFunc->triangles[i].p3.y * zeroY* yScale, tgtFunc->triangles[i].p3.z * zeroZ* zScale);
                glEnd();

                glBegin(GL_LINES);
                glColor3f(((float)tgtFunc->myColor.r) / 256.0, ((float)tgtFunc->myColor.g) / 256.0, ((float)tgtFunc->myColor.b) / 256.0);
                glVertex3f(tgtFunc->triangles[i].p2.x * zeroX* xScale, tgtFunc->triangles[i].p2.y * zeroY* yScale, tgtFunc->triangles[i].p2.z * zeroZ* zScale);
                glVertex3f(tgtFunc->triangles[i].p3.x * zeroX* xScale, tgtFunc->triangles[i].p3.y * zeroY* yScale, tgtFunc->triangles[i].p3.z * zeroZ* zScale);
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


