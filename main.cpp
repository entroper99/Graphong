#define _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS

#include <SDL.h>
#include <SDL_ttf.h>
#include <GL/glew.h>
#include <Eigen/Dense>
#include <codecvt>

import std;
import globalVar;
import constVar;
import drawer;
import Func;
import read;
import rainbow;
import cubicSpline;


int main(int argc, char** argv)
{
    bool debugMode = false;

    std::locale::global(std::locale("korean"));
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Graphong v0.201", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
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
    std::wprintf(L"Graphong v0.201\n");
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
                    funcSet[0]->singleTriangulation();
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
                std::string file;
                std::string fileInput;
                std::wprintf(L"������ ��θ� �Է����ּ���.\n");
                std::wprintf(L"data �������� ������ �н��ϴ�. ���� ��� Graphong/data/test.txt��� �͹̳ο� test.txt�� �Է����ּ���.\n");
                std::wprintf(L"������ ��� Graphong/data/folder/test.txt��� �͹̳ο� folder/test.txt�� �Է����ּ���.\n");
                std::wprintf(L"--------------------------------------------------------------------------------------------------\n");
                int i = 1;
                for (const auto& entry : std::filesystem::directory_iterator("data/"))
                {
                    if (std::filesystem::is_regular_file(entry.status()))
                    {
                        std::wcout << entry.path().filename().wstring();
                        std::wprintf(L"        ");
                        if (i % 5 == 0) std::wprintf(L"\n");
                        i++;
                    }
                }
                std::wprintf(L"\n----------------------------------������ �̸��� �Է����ּ����------------------------------------\n");
                std::cin >> fileInput;
                file = "data/" + fileInput;

                std::ifstream in(file);
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


                    int rCol, gCol, bCol;
                    std::wprintf(L"�� �������� R���� ���� �ұ�?(~255)\n");
                    std::cin >> rCol;
                    std::wprintf(L"�� �������� G���� ���� �ұ�?(~255)\n");
                    std::cin >> gCol;
                    std::wprintf(L"�� �������� B���� ���� �ұ�?(~255)\n");
                    std::cin >> bCol;

                    readXY(file, startLine, endLine, xCol, yCol, { (Uint8)rCol,(Uint8)gCol,(Uint8)bCol });
                }
                else std::wprintf(L"������ �дµ� �����Ͽ����ϴ�.\n");
            }
            else if (input == 2)
            {
                std::string file;
                std::string fileInput;
                std::wprintf(L"������ ��θ� �Է����ּ���.\n");
                std::wprintf(L"data �������� ������ �н��ϴ�. ���� ��� Graphong/data/test.txt��� �͹̳ο� test.txt�� �Է����ּ���.\n");
                std::wprintf(L"������ ��� Graphong/data/folder/test.txt��� �͹̳ο� folder/test.txt�� �Է����ּ���.\n");
                std::wprintf(L"--------------------------------------------------------------------------------------------------\n");
                int i = 1;
                for (const auto& entry : std::filesystem::directory_iterator("data/"))
                {
                    if (std::filesystem::is_regular_file(entry.status()))
                    {
                        std::wcout << entry.path().filename().wstring();
                        std::wprintf(L"        ");
                        if (i % 5 == 0) std::wprintf(L"\n");
                        i++;
                    }
                }
                std::wprintf(L"\n----------------------------------������ �̸��� �Է����ּ����------------------------------------\n");
                std::cin >> fileInput;
                file = "data/" + fileInput;

                std::ifstream in(file);
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

                    int rCol, gCol, bCol;
                    std::wprintf(L"�� �������� R���� ���� �ұ�?(~255)\n");
                    std::cin >> rCol;
                    std::wprintf(L"�� �������� G���� ���� �ұ�?(~255)\n");
                    std::cin >> gCol;
                    std::wprintf(L"�� �������� B���� ���� �ұ�?(~255)\n");
                    std::cin >> bCol;

                    readXYZ(file, startLine, endLine, xCol, yCol, zCol, { (Uint8)rCol,(Uint8)gCol,(Uint8)bCol });
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
                    Func* targetFunc = new Func();
                    funcSet.push_back(targetFunc);
                    targetFunc->myPoints.push_back({ inputX,inputY,inputZ });
                    std::wprintf(L"������ {%f,%f,%f}�� �Լ� %d�� �Է��ߴ�.\n", inputX, inputY, inputZ, funcSet.size() - 1);
                }
                else
                {
                    funcSet[targetIndex]->myPoints.push_back({ inputX,inputY,inputZ });
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
                std::cin >> dataIndex;
                funcSet[dataIndex]->triangulation();
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
                for (int i = 0; i < funcSet.size(); i++)
                {
                    std::wcout << "\033[38;2;" << static_cast<int>(funcSet[i]->myColor.r) << ";" << static_cast<int>(funcSet[i]->myColor.g) << ";" << static_cast<int>(funcSet[i]->myColor.b) << "m";
                    std::wprintf(L"[ �� : ");
                    std::printf(funcSet[i]->funcName.c_str());
                    std::wprintf(L"] \033[0m �ε��� : %d, �����ͼ� : %d ��, ���������� : %d ��, �÷��ڵ� : %d,%d,%d \n", i, funcSet[i]->myPoints.size(), funcSet[i]->myInterPoints.size(), funcSet[i]->myColor.r, funcSet[i]->myColor.g, funcSet[i]->myColor.b);
                }
            }
            else if (input == 99)
            {
                std::wprintf(L"----------------------------------------------------\n");
                std::wprintf(L"�׽�Ʈ�� ��� \n");
                std::wprintf(L"1.[Plumed] COLVAR : draw time-# Graph \n");
                std::wprintf(L"2.[Plumed] COLVAR : draw time-biasPot Graph \n");
                std::wprintf(L"3.[Plumed] FES : draw FES result \n");
                std::wprintf(L"4.[Plumed] FES : draw FES by time \n");
                std::wprintf(L"5.[Plumed] Sigma - Probability Density \n");
                std::wprintf(L"6.[Plumed] Sigma - Overlap \n");
                std::wprintf(L"------------------��Ʒ��� �� �Է�-----------------\n");
                int testInput = 0;
                std::cin >> testInput;
                if (testInput == 1)
                {
                    std::string file;
                    std::wprintf(L"COLVAR�� ��θ� �Է����ּ���.\n");
                    std::cin >> file;

                    std::ifstream in(file);
                    if (in.is_open())
                    {
                        int rCol, gCol, bCol;
                        std::wprintf(L"�� �������� R���� ���� �ұ�?(~255)\n");
                        std::cin >> rCol;
                        std::wprintf(L"�� �������� G���� ���� �ұ�?(~255)\n");
                        std::cin >> gCol;
                        std::wprintf(L"�� �������� B���� ���� �ұ�?(~255)\n");
                        std::cin >> bCol;

                        readXY(file, 1, -1, 0, 2, { (Uint8)rCol,(Uint8)gCol,(Uint8)bCol });

                        graphName = L"time-number graph";

                        xAxisName = "Time";
                        yAxisName = "Number_of_atoms";

                        xScaleUnit = L"ps";

                        xScale = 0.002;
                        yScale = 0.02;
                    }
                    else std::wprintf(L"COLVAR�� �дµ� �����Ͽ����ϴ�.\n");
                }
                else if (testInput == 2)
                {
                    std::string file;
                    std::wprintf(L"COLVAR�� ��θ� �Է����ּ���.\n");
                    std::cin >> file;

                    std::ifstream in(file);
                    if (in.is_open())
                    {
                        int rCol, gCol, bCol;
                        std::wprintf(L"�� �������� R���� ���� �ұ�?(~255)\n");
                        std::cin >> rCol;
                        std::wprintf(L"�� �������� G���� ���� �ұ�?(~255)\n");
                        std::cin >> gCol;
                        std::wprintf(L"�� �������� B���� ���� �ұ�?(~255)\n");
                        std::cin >> bCol;

                        readXY(file, 1, -1, 0, 3, { (Uint8)rCol,(Uint8)gCol,(Uint8)bCol });

                        graphName = L"time-biasPot graph";

                        xAxisName = "Time";
                        yAxisName = "BiasPotential";

                        xScaleUnit = L"ps";

                        xScale = 0.001;
                        yScale = 0.002;
                    }
                    else std::wprintf(L"COLVAR�� �дµ� �����Ͽ����ϴ�.\n");
                }
                else if (testInput == 3)
                {
                    std::string file;
                    std::wprintf(L"������ ��θ� �Է����ּ���.\n");
                    std::cin >> file;

                    std::ifstream in(file);
                    if (in.is_open())
                    {
                        int rCol, gCol, bCol;
                        std::wprintf(L"�� �������� R���� ���� �ұ�?(~255)\n");
                        std::cin >> rCol;
                        std::wprintf(L"�� �������� G���� ���� �ұ�?(~255)\n");
                        std::cin >> gCol;
                        std::wprintf(L"�� �������� B���� ���� �ұ�?(~255)\n");
                        std::cin >> bCol;

                        readXY(file, 5, -1, 0, 1, { (Uint8)rCol,(Uint8)gCol,(Uint8)bCol });

                        graphName = L"FES Result";

                        xAxisName = "CollectiveVariable";
                        yAxisName = "FreeEnergy";

                        yScaleUnit = L"kJ/mol";

                        xScale = 0.05;
                        yScale = 0.004;
                    }
                    else std::wprintf(L"������ �дµ� �����Ͽ����ϴ�.\n");
                }
                else if (testInput == 4)
                {
                    std::string folderPath;
                    std::wprintf(L"fes_0.dat�� ����ִ� ������ ��θ� �Է����ּ���.\n");
                    std::cin >> folderPath;

                    float stride = 0;
                    std::string strideAnswer;
                    std::wprintf(L"STRIDE�� ���� �Է��Ͻðڽ��ϱ�?[y/n]\n");
                    std::cin >> strideAnswer;
                    if (strideAnswer == "y")
                    {
                        std::wprintf(L"STRIDE�� ���� �Է����ּ���.\n");
                        std::cin >> stride;
                    }

                    if (std::filesystem::exists(folderPath + "/fes_0.dat"))
                    {
                        int fileEndNumber = -1;
                        std::wprintf(L"fes_0.dat�� ���������� ã�Ҵ�.\n");

                        while (1)
                        {
                            fileEndNumber++;
                            std::string targetPath = folderPath + "/fes_" + std::to_string(fileEndNumber) + ".dat";
                            if (std::filesystem::exists(targetPath) == false)
                            {
                                fileEndNumber--;
                                std::wprintf(L"���� �� ������ 0���� %d������ FES �����Ͱ� �����Ѵ�.\n", fileEndNumber);
                                break;
                            }
                        }


                        int startIndex = -1;
                        int endIndex = -1;

                        for (int i = 0; i <= fileEndNumber; i++)
                        {
                            std::string targetPath = folderPath + "/fes_" + std::to_string(i) + ".dat";
                            if (std::filesystem::exists(targetPath))
                            {
                                readXY(targetPath, 5, -1, 0, 1, rainbow(((float)i) / ((float)fileEndNumber)));

                                if (stride != 0)
                                {
                                    funcSet[funcSet.size() - 1]->funcName = "FES " + std::to_string(((float)i + 1.0) * stride / 1000.0) + " ns";
                                }

                                if (i == 0) startIndex = funcSet.size() - 1;
                                else if (i == fileEndNumber) endIndex = funcSet.size() - 1;
                            }

                        }

                        std::string answer;
                        std::wprintf(L"��� �Լ��鿡 ���� ���ö��� ������ �����ұ�?[y/n].\n");
                        std::cin >> answer;

                        if (answer == "y")
                        {
                            int newPointNum = 1;
                            std::wprintf(L"�� ǥ���� ���̿� �� �������� ������ �Է����ּ���.\n");
                            std::cin >> newPointNum;

                            for (int i = startIndex; i <= endIndex; i++)
                            {
                                cubicSpline(i, newPointNum);
                            }
                            visInterPoint = false;
                        }
                        interLine = true;

                        graphName = L"FES by time";

                        xAxisName = "CollectiveVariable";
                        yAxisName = "FreeEnergy";

                        yScaleUnit = L"kJ/mol";

                        xScale = 0.05;
                        yScale = 0.004;
                    }
                    else
                    {
                        std::wprintf(L"���� �бⰡ �����Ͽ����ϴ�. �ش� ��ο� fes_0.dat ������ �����ϴ�..\n");
                    }
                }
                else if (testInput == 5)
                {
                    //funcSet.clear();



                    std::string fileName;
                    std::vector<std::string> folderList;

                    std::wprintf(L"������ �̸��� �Է����ּ���.\n");
                    std::cin >> fileName;

                    const int phaseNumber = 2;

                    for (int i = 0; i < phaseNumber; i++)
                    {
                        std::wprintf(L"%d�� ������ ��θ� �Է����ּ���.\n", i);
                        std::string str;
                        std::cin >> str;
                        folderList.push_back(str);
                    }

                    for (int i = 0; i < folderList.size(); i++)
                    {
                        SDL_Color tgtCol = col::white;
                        switch (i % 7)
                        {
                        case 0:
                            tgtCol = col::yellow;
                            break;
                        case 1:
                            tgtCol = col::skyBlue;
                            break;
                        case 2:
                            tgtCol = col::monaLisa;
                            break;
                        case 3:
                            tgtCol = col::blueberry;
                            break;
                        case 4:
                            tgtCol = col::bondiBlue;
                            break;
                        case 5:
                            tgtCol = col::yellowGreen;
                            break;
                        case 6:
                            tgtCol = col::orange;
                            break;
                        }

                        int targetFuncIndex = readXY(folderList[i] + "/" + fileName, 6, -1, 0, 1, tgtCol);
                    }

                    xScale = 4;
                    yScale = 0.5;

                    graphName = std::wstring(fileName.begin(), fileName.end());

                    xAxisName = "k(��)";
                    yAxisName = "ProbabilityDensity";
                }
                else if (testInput == 6)
                {
                    xScale = 360;
                    yScale = 1000;
                    std::string path;
                    std::wprintf(L"overlap.txt�� ��θ� �Է����ּ���.\n");
                    std::cin >> path;
                    int targetIndex = readXY(path, 0, -1, 0, 1, { 255,255,0 });
                    cubicSpline(targetIndex, 6);
                    interLine = true;
                    visDataPoint = true;
                    visInterPoint = false;

                    float min = 999999.0;
                    int minIndex = -1;
                    for (int i = 0; i < funcSet[targetIndex]->myInterPoints.size(); i++)
                    {
                        if (funcSet[targetIndex]->myInterPoints[i].y < min)
                        {
                            min = funcSet[targetIndex]->myInterPoints[i].y;
                            minIndex = i;
                        }
                    }
                    std::wprintf(L"\033[0;33m");
                    std::wprintf(L"overlap�� �������� (%f,%f)�̴�. ���� ������ �� = %f\n", funcSet[targetIndex]->myInterPoints[minIndex].x, funcSet[targetIndex]->myInterPoints[minIndex].y, funcSet[targetIndex]->myInterPoints[minIndex].x);
                    std::wprintf(L"\033[0m");

                    xAxisName = "SIGMA";
                    yAxisName = "Overlap";
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

        float axisLength = 1000.0f;

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
        if (camFixZ || camFixMinusZ) zeroX = 0;

        if (visDataPoint)
        {
            for (int dataIndex = 0; dataIndex < funcSet.size(); dataIndex++)
            {
                for (int i = 0; i < funcSet[dataIndex]->myPoints.size(); i++)
                {
                    glPointSize(pointSize);
                    glBegin(GL_POINTS);
                    glColor3f(((float)funcSet[dataIndex]->myColor.r) / 256.0, ((float)funcSet[dataIndex]->myColor.g) / 256.0, ((float)funcSet[dataIndex]->myColor.b) / 256.0);
                    glVertex3f(zeroX * xScale * (funcSet[dataIndex]->myPoints[i].x), zeroY * yScale * (funcSet[dataIndex]->myPoints[i].y), zeroZ * zScale * (funcSet[dataIndex]->myPoints[i].z));
                    glEnd();
                }
            }
        }

        if (visInterPoint)
        {

            for (int dataIndex = 0; dataIndex < funcSet.size(); dataIndex++)
            {
                for (int i = 0; i < funcSet[dataIndex]->myInterPoints.size(); i++)
                {
                    glPointSize(pointSize);
                    glBegin(GL_POINTS);
                    glColor3f(((float)funcSet[dataIndex]->myColor.r) / 256.0 / 3.0, ((float)funcSet[dataIndex]->myColor.g) / 256.0 / 3.0, ((float)funcSet[dataIndex]->myColor.b) / 256.0 / 3.0);
                    glVertex3f(zeroX * xScale * (funcSet[dataIndex]->myInterPoints[i].x), zeroY * yScale * (funcSet[dataIndex]->myInterPoints[i].y), zeroZ * zScale * (funcSet[dataIndex]->myInterPoints[i].z));
                    glEnd();
                }
            }
        }

        if (interLine)
        {
            for (int dataIndex = 0; dataIndex < funcSet.size(); dataIndex++)
            {
                if (funcSet[dataIndex]->myInterPoints.size() >= 2)
                {
                    for (int i = 0; i < funcSet[dataIndex]->myInterPoints.size() - 1; i++)
                    {
                        // ���� �׸���
                        glBegin(GL_LINES);
                        glColor3f(((float)funcSet[dataIndex]->myColor.r) / 256.0, ((float)funcSet[dataIndex]->myColor.g) / 256.0, ((float)funcSet[dataIndex]->myColor.b) / 256.0);
                        glVertex3f(zeroX * xScale * (funcSet[dataIndex]->myInterPoints[i].x), zeroY * yScale * (funcSet[dataIndex]->myInterPoints[i].y), zeroZ * zScale * (funcSet[dataIndex]->myInterPoints[i].z));
                        glVertex3f(zeroX * xScale * (funcSet[dataIndex]->myInterPoints[i + 1].x), zeroY * yScale * (funcSet[dataIndex]->myInterPoints[i + 1].y), zeroZ * zScale * (funcSet[dataIndex]->myInterPoints[i + 1].z));
                        glEnd();
                    }
                }
            }
        }

        for (int dataIndex = 0; dataIndex < funcSet.size(); dataIndex++)
        {
            for (int i = 0; i < funcSet[dataIndex]->triangles.size(); i++)
            {
                glBegin(GL_LINES);
                glColor3f(((float)funcSet[dataIndex]->myColor.r) / 256.0, ((float)funcSet[dataIndex]->myColor.g) / 256.0, ((float)funcSet[dataIndex]->myColor.b) / 256.0);
                glVertex3f(funcSet[dataIndex]->triangles[i].p1.x * zeroX* xScale, funcSet[dataIndex]->triangles[i].p1.y * zeroY* yScale, funcSet[dataIndex]->triangles[i].p1.z * zeroZ* zScale);
                glVertex3f(funcSet[dataIndex]->triangles[i].p2.x * zeroX* xScale, funcSet[dataIndex]->triangles[i].p2.y * zeroY* yScale, funcSet[dataIndex]->triangles[i].p2.z * zeroZ* zScale);
                glEnd();

                glBegin(GL_LINES);
                glColor3f(((float)funcSet[dataIndex]->myColor.r) / 256.0, ((float)funcSet[dataIndex]->myColor.g) / 256.0, ((float)funcSet[dataIndex]->myColor.b) / 256.0);
                glVertex3f(funcSet[dataIndex]->triangles[i].p1.x * zeroX* xScale, funcSet[dataIndex]->triangles[i].p1.y * zeroY* yScale, funcSet[dataIndex]->triangles[i].p1.z * zeroZ* zScale);
                glVertex3f(funcSet[dataIndex]->triangles[i].p3.x * zeroX* xScale, funcSet[dataIndex]->triangles[i].p3.y * zeroY* yScale, funcSet[dataIndex]->triangles[i].p3.z * zeroZ* zScale);
                glEnd();

                glBegin(GL_LINES);
                glColor3f(((float)funcSet[dataIndex]->myColor.r) / 256.0, ((float)funcSet[dataIndex]->myColor.g) / 256.0, ((float)funcSet[dataIndex]->myColor.b) / 256.0);
                glVertex3f(funcSet[dataIndex]->triangles[i].p2.x * zeroX* xScale, funcSet[dataIndex]->triangles[i].p2.y * zeroY* yScale, funcSet[dataIndex]->triangles[i].p2.z * zeroZ* zScale);
                glVertex3f(funcSet[dataIndex]->triangles[i].p3.x * zeroX* xScale, funcSet[dataIndex]->triangles[i].p3.y * zeroY* yScale, funcSet[dataIndex]->triangles[i].p3.z * zeroZ* zScale);
                glEnd();


                //glBegin(GL_TRIANGLES);
                //glColor3f(((float)funcSet[dataIndex]->myColor.r) / 256.0 / 4.0, ((float)funcSet[dataIndex]->myColor.g) / 256.0 / 4.0, ((float)funcSet[dataIndex]->myColor.b) / 256.0 / 4.0);
                //glVertex3f(funcSet[dataIndex]->triangles[i].p1.x* xScale, funcSet[dataIndex]->triangles[i].p1.y* yScale, funcSet[dataIndex]->triangles[i].p1.z* zScale);
                //glVertex3f(funcSet[dataIndex]->triangles[i].p2.x* xScale, funcSet[dataIndex]->triangles[i].p2.y* yScale, funcSet[dataIndex]->triangles[i].p2.z* zScale);
                //glVertex3f(funcSet[dataIndex]->triangles[i].p3.x* xScale, funcSet[dataIndex]->triangles[i].p3.y* yScale, funcSet[dataIndex]->triangles[i].p3.z* zScale);

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
