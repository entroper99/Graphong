#define _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS

#include <SDL.h>
#include <SDL_ttf.h>
#include <GL/glew.h>
#include <Eigen/Dense>
#include <codecvt>
#include <windows.h>

import std;
import globalVar;
import constVar;
import drawer;
import Func;
import read;
import rainbow;
import cubicSpline;
import inputCol;


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
        std::wcout << "\033[38;2;" << static_cast<int>(funcSet[i]->myColor.r) << ";" << static_cast<int>(funcSet[i]->myColor.g) << ";" << static_cast<int>(funcSet[i]->myColor.b) << "m";
        std::wprintf(L"[ %d�� �Լ� : ", i);
        std::wprintf(funcSet[i]->funcName.c_str());
        std::wprintf(L"] \033[0m �����ͼ� : %d ��, ���������� : %d ��, �÷��ڵ� : %d,%d,%d \n", i, funcSet[i]->myPoints.size(), funcSet[i]->myInterPoints.size(), funcSet[i]->myColor.r, funcSet[i]->myColor.g, funcSet[i]->myColor.b);
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
            std::wprintf(L"23.���̷��̵� ���� ����\n");
            std::wprintf(L"24.�Լ��� ���\n");
            std::wprintf(L"25.�߽����� ���� �Լ� ǥ��ȭ\n");
            std::wprintf(L"26.PDE Solver\n");
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
                //openFileDialog();
                //std::string file;
                //std::string fileInput;
                //std::wprintf(L"������ ��θ� �Է����ּ���.\n");
                //std::wprintf(L"data �������� ������ �н��ϴ�. ���� ��� Graphong/data/test.txt��� �͹̳ο� test.txt�� �Է����ּ���.\n");
                //std::wprintf(L"������ ��� Graphong/data/folder/test.txt��� �͹̳ο� folder/test.txt�� �Է����ּ���.\n");
                //std::wprintf(L"--------------------------------------------------------------------------------------------------\n");
                //int i = 1;
                //for (const auto& entry : std::filesystem::directory_iterator("data/"))
                //{
                //    if (std::filesystem::is_regular_file(entry.status()))
                //    {
                //        std::wcout << entry.path().filename().wstring();
                //        std::wprintf(L"        ");
                //        if (i % 5 == 0) std::wprintf(L"\n");
                //        i++;
                //    }
                //}
                //std::wprintf(L"\n----------------------------------������ �̸��� �Է����ּ����------------------------------------\n");
                //std::cin >> fileInput;
                //file = "data/" + fileInput;

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
                //std::wstring file;
                //std::wstring fileInput;
                //std::wprintf(L"������ ��θ� �Է����ּ���.\n");
                //std::wprintf(L"data �������� ������ �н��ϴ�. ���� ��� Graphong/data/test.txt��� �͹̳ο� test.txt�� �Է����ּ���.\n");
                //std::wprintf(L"������ ��� Graphong/data/folder/test.txt��� �͹̳ο� folder/test.txt�� �Է����ּ���.\n");
                //std::wprintf(L"--------------------------------------------------------------------------------------------------\n");
                //int i = 1;
                //for (const auto& entry : std::filesystem::directory_iterator("data/"))
                //{
                //    if (std::filesystem::is_regular_file(entry.status()))
                //    {
                //        std::wcout << entry.path().filename().wstring();
                //        std::wprintf(L"        ");
                //        if (i % 5 == 0) std::wprintf(L"\n");
                //        i++;
                //    }
                //}
                //std::wprintf(L"\n----------------------------------������ �̸��� �Է����ּ����------------------------------------\n");
                //std::wcin >> fileInput;
                //file = L"data/" + fileInput;

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
                prtFuncName();
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
                prtFuncName();
            }
            else if (input == 23) //���̷��̵� ����
            {
                Func* targetFunc = new Func();
                funcSet.push_back(targetFunc);


                int numPoints = 40; // ������ �����ϴ� ���� ��, �������� ��Ȯ����
                double spacing = (2.0 * M_PI) / numPoints; //������ ����, ���� ���ϸ� ���ǰ� ��

                double cutoff = 1.0; //�� �� �̻��� ���̷��̵常 ȭ�鿡 ǥ�õ�

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
                            if (value >= cutoff) targetFunc->myPoints.push_back({ x, y, z });
                            //std::wprintf(L"������ {%f,%f,%f}�� �Լ� %d�� �Է��ߴ�.\n", x, y, z, funcSet.size() - 1);
                        }
                    }
                }


                /*int numPoints = 40; 
                double spacing = 0.5;*/

                //int numPoints = 40; // ������ �����ϴ� ���� ��, �������� ��Ȯ����
                //double spacing = (2.0 * M_PI) / numPoints; //������ ����, ���� ���ϸ� ���ǰ� ��
                //
                //double cutoff = 1.0; //�� �� �̻��� ���̷��̵常 ȭ�鿡 ǥ�õ�

                //for (int i = numPoints; i < numPoints; ++i) 
                //{
                //    for (int j = 0; j < numPoints; ++j) 
                //    {
                //        for (int k = 0; k < numPoints; ++k) 
                //        {
                //            double x = i * spacing;
                //            double y = j * spacing;
                //            double z = k * spacing;
                //            double value = std::sqrt(8.0 / 3.0) * (std::cos(x) * std::sin(y) * std::sin(2 * z) + std::cos(y) * std::sin(z) * std::sin(2 * x) + std::cos(z) * std::sin(x) * std::sin(2 * y));
                //            if (value >= cutoff) targetFunc->myPoints.push_back({ x, y, z });
                //            //std::wprintf(L"������ {%f,%f,%f}�� �Լ� %d�� �Է��ߴ�.\n", x, y, z, funcSet.size() - 1);
                //        }
                //    }
                //}
            }
            else if (input == 24)
            {
                int dataIndex = 0;
                std::wprintf(L"���° �������� ����Ʈ�� ����ұ�? (0 ~ %d).\n", funcSet.size() - 1);
                prtFuncName();
                std::cin >> dataIndex;
                for (int i = 0; i < funcSet[dataIndex]->myPoints.size(); i++)
                {
                    Point pt = funcSet[dataIndex]->myPoints[i];
                    std::wprintf(L"Index %d : {%.10f,%.10f,%.10f}\n", i, pt.x, pt.y, pt.z);
                }
            }
            else if (input == 25) //������ ǥ��ȭ
            {
                int dataIndex = 0;
                std::wprintf(L"���° �����͸� ��Ī ������ �����? (0 ~ %d).\n", funcSet.size() - 1);
                prtFuncName();
                std::cin >> dataIndex;
                funcSet[dataIndex]->sym();
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
                delT = 0.0005;
                //delT = 0.05;
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
                std::wstring log = L"";
                //��̺� ������ ���
                {
                    for (int t = 0; t < tNum - 1; t++)
                    {
                        std::vector<double> concNew(xNum, 0.0);
                        for (int i = 1; i < xNum - 1; ++i)
                        {
                            double diff_halfBefore = diffFunc((conc[t][i] + conc[t][i + 1]) / 2.0);
                            double diff_halfAfter = diffFunc((conc[t][i] + conc[t][i - 1]) / 2.0);
                            concNew[i] = conc[t][i] + delT / (delX * delX) * (diff_halfBefore * (conc[t][i + 1] - conc[t][i]) - diff_halfAfter * (conc[t][i] - conc[t][i - 1]));
                            if (doPrint) std::wprintf(L"{x = %f,  t = %f,  C(x,t) = %f}\n", i*delX, t*delT, concNew[i]);
                        }
                        concNew[0] = startDistBC;
                        concNew[xNum - 1] = infDistBC;
                        conc.push_back(concNew);
                    }
                }

               
                Func* targetFunc = new Func();
                funcSet.push_back(targetFunc);

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
                    Func* targetFunc2 = new Func();
                    targetFunc2->myColor = inputCol();
                    funcSet.push_back(targetFunc2);
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
        if (camFixX || camFixMinusX) zeroX = 0;

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


