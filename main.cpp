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
    ofn.lpstrTitle = L"데이터가 있는 파일을 선택해주세요.";
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
    if (GetOpenFileName(&ofn)) return std::wstring(filename);
    else return L"";
}

void prtFuncName()
{
    for (int i = 0; i < funcSet.size(); i++)
    {
        std::wcout << "\033[38;2;" << static_cast<int>(((Func*)funcSet[i])->myColor.r) << ";" << static_cast<int>(((Func*)funcSet[i])->myColor.g) << ";" << static_cast<int>(((Func*)funcSet[i])->myColor.b) << "m";
        std::wprintf(L"[ %d번 함수 : ", i);
        std::wprintf(((Func*)funcSet[i])->funcName.c_str());
        std::wprintf(L"] \033[0m 데이터수 : %d 개, 보간데이터 : %d 개, 컬러코드 : %d,%d,%d \n", i, ((Func*)funcSet[i])->myPoints.size(), ((Func*)funcSet[i])->myInterPoints.size(), ((Func*)funcSet[i])->myColor.r, ((Func*)funcSet[i])->myColor.g, ((Func*)funcSet[i])->myColor.b);
    }
}


int main(int argc, char** argv)
{
    bool debugMode = false;

    std::locale::global(std::locale("korean"));
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Graphong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_SetRelativeMouseMode(SDL_TRUE); //마우스 숨기기
    glewExperimental = GL_TRUE; //opneGL 실험 기능 활성화
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
    std::wprintf(L"WASD : 이동\n");
    std::wprintf(L"QE : 고도조절\n");
    std::wprintf(L"X,Y,Z + 마우스휠 : 스케일 조절\n");
    std::wprintf(L"\033[0;31m적색 : X축\n");
    std::wprintf(L"\033[0;32m녹색 : Y축\n");
    std::wprintf(L"\033[0;34m청색 : Z축\033[0m\n");
    std::wprintf(L"Enter 키를 눌러 커맨드 입력\n");
    std::wprintf(L"**********************************************************\n");
    std::wprintf(L"\033[0m");

    TTF_Font* font = TTF_OpenFont("NanumGothic.ttf", 16);
    if (font == nullptr) std::wprintf(L"16사이즈의 폰트 로드에 실패하였다.\n");

    TTF_Font* smallFont = TTF_OpenFont("NanumGothic.ttf", 10);
    if (smallFont == nullptr) std::wprintf(L"10사이즈의 폰트 로드에 실패하였다.\n");

    bool quit = false;
    SDL_Event event;
    while (!quit)
    {
        const Uint8* state = SDL_GetKeyboardState(NULL);

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) quit = true;
            else if (event.type == SDL_MOUSEMOTION)//마우스 이동
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
        //키보드 입력

        if (state[SDL_SCANCODE_RETURN])
        {
            std::wprintf(L"----------------------------------------------------------\n");
            std::wprintf(L"1.f(x)=y 데이터 읽기\n");
            std::wprintf(L"2.f(x,z)=y 데이터 읽기\n");
            std::wprintf(L"3.데이터 수동 입력\n");
            std::wprintf(L"4.데이터 초기화\n");
            std::wprintf(L"5.Axis 스케일 조정\n");
            std::wprintf(L"6.Axis 이름 설정\n");
            if (camFixMinusZ || camFixZ || camFixMinusX || camFixX || camFixMinusY || camFixY) std::wprintf(L"7.Axis 카메라 고정 해제\n");
            else std::wprintf(L"7.2차원 카메라 고정\n");
            std::wprintf(L"\033[0;33m8.Delaunay 삼각분할\033[0m\n");

            std::wprintf(L"9. 함수 Translation (평행이동)\n");
            std::wprintf(L"10. 함수 Rotation (회전)\n");

            std::wprintf(L"\033[0;33m11.[2차원] Cubic 스플라인 보간 실행\033[0m\n");
            std::wprintf(L"\033[0;33m12.[2차원] 기존 보간값에 대해 선형 보간 실행\033[0m\n");
            if (visDataPoint)  std::wprintf(L"14.데이터점 화면 표시 [ \033[0;32mON\033[0m / OFF ]\n");
            else std::wprintf(L"14.데이터점 화면 표시  [ ON / \033[1;31mOFF\033[0m ]\n");
            if (visInterPoint)  std::wprintf(L"15.보간점 화면 표시 [ \033[0;32mON\033[0m / OFF ]\n");
            else std::wprintf(L"15.보간점 화면 표시  [ ON / \033[1;31mOFF\033[0m ]\n");
            std::wprintf(L"17.스케일 단위 설정\n");
            std::wprintf(L"18.카메라 속도 설정\n");
            std::wprintf(L"20.데이터 점 크기 조절\n");
            std::wprintf(L"21.그래프 이름 변경\n");
            std::wprintf(L"22.함수 목록 출력\n");
            std::wprintf(L"23.자이로이드 구조 생성\n");
            std::wprintf(L"24.함수값 출력\n");
            std::wprintf(L"25.중심점에 대해 함수 표준화\n");
            std::wprintf(L"26.PDE Solver\n");
            std::wprintf(L"27.LAMMPS Trajectory 파일 읽기(타임스텝 0)\n");
            std::wprintf(L"28.LAMMPS Trajectory 파일 읽기(모든 타임스텝)\n");
            //std::wprintf(L"\033[37m");
            //std::wprintf(L"101.[Plumed] COLVAR : draw time-# Graph \n");
            //std::wprintf(L"102.[Plumed] COLVAR : draw time-biasPot Graph \n");
            //std::wprintf(L"103.[Plumed] FES : draw FES result \n");
            //std::wprintf(L"104.[Plumed] FES : draw FES by time \n");
            //std::wprintf(L"105.[Plumed] Sigma - Probability Density \n");
            //std::wprintf(L"106.[Plumed] Sigma - Overlap \n");
            //std::wprintf(L"\033[0m");
            std::wprintf(L"------------------▼아래에 값 입력-----------------\n");

            int input = 0;
            std::cin >> input;
            if (input == 1)
            {
                std::wstring file = L"";
                std::wprintf(L"데이터가 있는 파일을 선택해주세요.\n");
                file = openFileDialog();
                std::wprintf(L"파일 %ls 를 대상으로 설정하였다.\n",file.c_str());
                std::wifstream in(file);
                if (in.is_open())
                {
                    int startLine = 0;
                    std::wprintf(L"읽기 시작할 행을 입력해주세요.(0부터 시작)\n");
                    std::cin >> startLine;

                    int endLine = 0;
                    std::wprintf(L"마지막으로 읽을 행을 입력해주세요.(0부터 시작, -1일 경우 끝까지)\n");
                    std::cin >> endLine;

                    int xCol = 0;
                    std::wprintf(L"x값이 있는 열을 입력해주세요.(0부터 시작)\n");
                    std::cin >> xCol;

                    int yCol = 0;
                    std::wprintf(L"y값이 있는 열을 입력해주세요.(0부터 시작)\n");
                    std::cin >> yCol;

                    SDL_Color col = inputCol();
                    readXY(file, startLine, endLine, xCol, yCol, col);
                }
                else std::wprintf(L"파일을 읽는데 실패하였습니다.\n");
            }
            else if (input == 2)
            {
                std::wstring file = L"";
                std::wprintf(L"데이터가 있는 파일을 선택해주세요.\n");
                file = openFileDialog();
                std::wprintf(L"파일 %ls 를 대상으로 설정하였다.\n", file.c_str());
                std::wifstream in(file);
                if (in.is_open())
                {
                    int startLine = 0;
                    std::wprintf(L"읽기 시작할 행을 입력해주세요.(0부터 시작)\n");
                    std::cin >> startLine;

                    int endLine = 0;
                    std::wprintf(L"마지막으로 읽을 행을 입력해주세요.(0부터 시작, -1일 경우 끝까지)\n");
                    std::cin >> endLine;

                    int xCol = 0;
                    std::wprintf(L"(Input) x값이 있는 열을 입력해주세요.(0부터 시작)\n");
                    std::cin >> xCol;

                    int zCol = 0;
                    std::wprintf(L"(Input) z값이 있는 열을 입력해주세요.(0부터 시작)\n");
                    std::cin >> zCol;

                    int yCol = 0;
                    std::wprintf(L"(Output) y값이 있는 열을 입력해주세요.(0부터 시작)\n");
                    std::cin >> yCol;

                    SDL_Color col = inputCol();
                    readXYZ(file, startLine, endLine, xCol, yCol, zCol, col);
                }
                else std::wprintf(L"파일을 읽는데 실패하였습니다.\n");
            }
            else if (input == 3)
            {
                int targetIndex = 0;
                float inputX, inputY, inputZ;
                std::wprintf(L"어느 함수에 점을 추가할까? (0부터 %d까지,-1이면 새로운 함수 추가)\n", funcSet.size() - 1);
                std::cin >> targetIndex;

                std::wprintf(L"x의 값을 입력해주세요.\n");
                std::cin >> inputX;
                std::wprintf(L"y의 값을 입력해주세요.\n");
                std::cin >> inputY;
                std::wprintf(L"z의 값을 입력해주세요.\n");
                std::cin >> inputZ;

                if (targetIndex == -1)
                {
                    Func* targetFunc = new Func(funcFlag::dim3);
                    targetFunc->myPoints.push_back({ inputX,inputY,inputZ });
                    std::wprintf(L"데이터 {%f,%f,%f}를 함수 %d에 입력했다.\n", inputX, inputY, inputZ, funcSet.size() - 1);
                }
                else
                {
                    ((Func*)funcSet[targetIndex])->myPoints.push_back({ inputX,inputY,inputZ });
                    std::wprintf(L"데이터 {%f,%f,%f}를 함수 %d에 입력했다.\n", inputX, inputY, inputZ, targetIndex);
                }
            }
            else if (input == 4)
            {
                std::wprintf(L"모든 함수를 초기화하였다.\n");
                for (int i = funcSet.size() - 1; i >= 0; i--) delete funcSet[i];
                funcSet.clear();
            }
            else if (input == 5)
            {
                std::string axisInput;
                std::wprintf(L"조정할 축을 입력해주세요(x,y,z)\n");
                std::cin >> axisInput;
                float scaleInput;
                if (axisInput == "x")
                {
                    std::wprintf(L"x축의 스케일 값을 입력해주세요.(현재 : %f)\n", xScale);
                    std::cin >> scaleInput;
                    xScale = scaleInput;
                }
                else if (axisInput == "y")
                {
                    std::wprintf(L"y축의 스케일 값을 입력해주세요.(현재 : %f)\n", yScale);
                    std::cin >> scaleInput;
                    yScale = scaleInput;
                }
                else if (axisInput == "z")
                {
                    std::wprintf(L"z축의 스케일 값을 입력해주세요.(현재 : %f)\n", zScale);
                    std::cin >> scaleInput;
                    zScale = scaleInput;
                }
                std::wprintf(L"스케일 입력을 완료했다.\n");
            }
            else if (input == 6)
            {
                std::wprintf(L"x축의 이름을 입력해주세요.\n");
                std::cin >> xAxisName;
                if (xAxisName == "NULL") xAxisName.clear();
                std::wprintf(L"y축의 이름을 입력해주세요.\n");
                std::cin >> yAxisName;
                if (yAxisName == "NULL") yAxisName.clear();
                std::wprintf(L"z축의 이름을 입력해주세요.\n");
                std::cin >> zAxisName;
                if (zAxisName == "NULL") zAxisName.clear();
            }
            else if (input == 7)
            {
                if (!(camFixMinusZ || camFixZ || camFixMinusX || camFixX || camFixMinusY || camFixY))
                {
                    std::wprintf(L"카메라를 어떤 축을 바라보는 방향으로 고정할까요? (e.g. +x, -z)\n");
                    std::string inputStr;
                    std::cin >> inputStr;
                    if (inputStr == "+x")
                    {
                        std::wprintf(L"카메라를 +x 방향으로 고정했다.\n");
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
                        std::wprintf(L"카메라를 -x 방향으로 고정했다.\n");
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
                        std::wprintf(L"카메라를 +y 방향으로 고정했다.\n");
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
                        std::wprintf(L"카메라를 -y 방향으로 고정했다.\n");
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
                        std::wprintf(L"카메라를 +z 방향으로 고정했다.\n");
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
                        std::wprintf(L"카메라를 -z 방향으로 고정했다.\n");
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
                        std::wprintf(L"카메라 고정을 해제했다.\n");
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
                    std::wprintf(L"카메라 고정을 해제했다.\n");
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
                std::wprintf(L"몇번째 데이터에 삼각분할을 실행할까? (0 ~ %d).\n", funcSet.size() - 1);
                prtFuncName();
                std::cin >> dataIndex;
                ((Func*)funcSet[dataIndex])->triangulation();
            }

            else if (input == 9)
            {
                int dataIndex = 0;
                std::wprintf(L"몇번째 데이터에 translation을 진행할까? (0 ~ %d).\n", funcSet.size() - 1);
                prtFuncName();
                std::cin >> dataIndex;

                double compX = 0, compY = 0, compZ = 0;
                std::wprintf(L"Translation Vector의 x 성분을 입력해주세요.\n");
                std::wprintf(L"Vector : { ■, □, □ }\n");
                std::cin >> compX;
                std::wprintf(L"Translation Vector의 y 성분을 입력해주세요.\n");
                std::wprintf(L"Vector : { %f, ■, □ }\n", compX);
                std::cin >> compY;
                std::wprintf(L"Translation Vector의 z 성분을 입력해주세요.\n");
                std::wprintf(L"Vector : { %f, %f, ■ }\n", compX, compY);
                std::cin >> compZ;
                std::wprintf(L"다음 벡터로 tranlsation을 진행합니다.\n");
                std::wprintf(L"Vector : { %f, %f, %f }\n", compX, compY, compZ);


                ((Func*)funcSet[dataIndex])->translation(compX,compY,compZ);
                ((Func*)funcSet[dataIndex])->scalarCalc();
                std::wprintf(L"이 함수의 평균 f값은 %f이다.\n", ((Func*)funcSet[dataIndex])->scalarAvg());
            }

            else if (input == 10)
            {
                int dataIndex = 0;
                std::wprintf(L"몇번째 데이터에 rotation을 진행할까? (0 ~ %d).\n", funcSet.size() - 1);
                prtFuncName();
                std::cin >> dataIndex;

                double a11 = 1.0, a12 = 0.0, a13 = 0.0;
                double a21 = 0.0, a22 = 1.0, a23 = 0.0;
                double a31 = 0.0, a32 = 0.0, a33 = 1.0;

                std::wprintf(L"Rotation Matrix의 a11 성분을 입력해주세요.\n", funcSet.size() - 1);
                std::wprintf(L"{ ■, □, □ }\n");
                std::wprintf(L"{ □, □, □ }\n");
                std::wprintf(L"{ □, □, □ }\n");
                std::cin >> a11;

                std::wprintf(L"Rotation Matrix의 a12 성분을 입력해주세요.\n", funcSet.size() - 1);
                std::wprintf(L"{ %f, ■, □ }\n", a11);
                std::wprintf(L"{ □, □, □ }\n");
                std::wprintf(L"{ □, □, □ }\n");
                std::cin >> a12;

                std::wprintf(L"Rotation Matrix의 a13 성분을 입력해주세요.\n");
                std::wprintf(L"{ %f, %f, ■ }\n", a11, a12);
                std::wprintf(L"{ □, □, □ }\n");
                std::wprintf(L"{ □, □, □ }\n");
                std::cin >> a13;

                std::wprintf(L"Rotation Matrix의 a21 성분을 입력해주세요.\n");
                std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
                std::wprintf(L"{ ■, □, □ }\n");
                std::wprintf(L"{ □, □, □ }\n");
                std::cin >> a21;

                std::wprintf(L"Rotation Matrix의 a22 성분을 입력해주세요.\n");
                std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
                std::wprintf(L"{ %f, ■, □ }\n", a21);
                std::wprintf(L"{ □, □, □ }\n");
                std::cin >> a22;

                std::wprintf(L"Rotation Matrix의 a23 성분을 입력해주세요.\n");
                std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
                std::wprintf(L"{ %f, %f, ■ }\n", a21, a22);
                std::wprintf(L"{ □, □, □ }\n");
                std::cin >> a23;

                std::wprintf(L"Rotation Matrix의 a31 성분을 입력해주세요.\n");
                std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
                std::wprintf(L"{ %f, %f, %f }\n", a21, a22, a23);
                std::wprintf(L"{ ■, □, □ }\n");
                std::cin >> a31;

                std::wprintf(L"Rotation Matrix의 a32 성분을 입력해주세요.\n");
                std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
                std::wprintf(L"{ %f, %f, %f }\n", a21, a22, a23);
                std::wprintf(L"{ %f, ■, □ }\n", a31);
                std::cin >> a32;

                std::wprintf(L"Rotation Matrix의 a33 성분을 입력해주세요.\n");
                std::wprintf(L"{ %f, %f, %f }\n", a11, a12, a13);
                std::wprintf(L"{ %f, %f, %f }\n", a21, a22, a23);
                std::wprintf(L"{ %f, %f, ■ }\n", a31, a32);
                std::cin >> a33;

                Eigen::Matrix3d rotationMatrix;

                //rotationMatrix << 1.0, 0.0, 0.0,
                //    0.0, 0.7071, -0.7071,
                //    0.0, 0.7071, 0.7071;

                rotationMatrix << a11, a12, a13,
                    a21, a22, a23,
                    a31, a32, a33;



                std::wprintf(L"다음 행렬로 Rotation을 진행합니다.\n");
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
                    std::cout << "회전각 : " << theta * 180.0 / M_PI << std::endl;
                    std::cout << "회전축: (" << axis.x() << ", " << axis.y() << ", " << axis.z() << ")" << std::endl;
                }



                ((Func*)funcSet[dataIndex])->rotation(rotationMatrix);
                ((Func*)funcSet[dataIndex])->scalarCalc();
                std::wprintf(L"이 함수의 평균 f값은 %f이다.\n", ((Func*)funcSet[dataIndex])->scalarAvg());
            }

            else if (input == 11) //cubic 스플라인 보간 실행
            {
                int dataIndex = 0;
                std::wprintf(L"몇번째 데이터를 보간할까? (0 ~ %d).\n", funcSet.size() - 1);
                std::cin >> dataIndex;

                int newPointNum = 1;
                std::wprintf(L"두 표본점 사이에 들어갈 보간점의 개수를 입력해주세요.\n");
                std::cin >> newPointNum;

                cubicSpline(dataIndex, newPointNum);
            }
            else if (input == 14)
            {
                if (visDataPoint == false)
                {
                    std::wprintf(L"데이터값을 회면에서 다시 표시했다.\n");
                    visDataPoint = true;
                }
                else
                {
                    std::wprintf(L"데이터값을 회면에서 숨겼다.\n");
                    visDataPoint = false;
                }
            }
            else if (input == 15)
            {
                if (visInterPoint == false)
                {
                    std::wprintf(L"보간값을 회면에서 다시 표시했다.\n");
                    visInterPoint = true;
                }
                else
                {
                    std::wprintf(L"보간값을 회면에서 숨겼다.\n");
                    visInterPoint = false;
                }
            }
            else if (input == 12)
            {
                if (interLine == false)
                {
                    std::wprintf(L"보간값들을 직선으로 연결하였다.\n");
                    interLine = true;
                }
                else
                {
                    std::wprintf(L"보간값들의 직선을 화면에서 숨겼다.\n");
                    interLine = false;
                }
            }
            else if (input == 17)
            {
                std::wprintf(L"x축 스케일의 단위를 입력해주세요.\n");
                std::wcin >> xScaleUnit;
                if (xAxisName == "NULL") xAxisName.clear();

                std::wprintf(L"y축 스케일의 단위를 입력해주세요.\n");
                std::wcin >> yScaleUnit;
                if (yAxisName == "NULL") yAxisName.clear();

                std::wprintf(L"z축 스케일의 단위를 입력해주세요.\n");
                std::wcin >> zScaleUnit;
                if (zAxisName == "NULL") zAxisName.clear();
            }
            else if (input == 18)
            {
                std::wprintf(L"카메라의 속도를 입력해주세요 (현재 : %f).\n", camSpd);
                std::cin >> camSpd;
            }
            else if (input == 20)
            {
                std::wprintf(L"점의 크기를 몇으로 조정할까? (현재 : %f)\n", pointSize);
                std::cin >> pointSize;
                std::wprintf(L"점의 크기를 성공적으로 바꾸었다.\n");
            }
            else if (input == 21)
            {
                std::wprintf(L"이 함수셋의 이름을 뭐라고 할까?\n");
                int dummy;
                std::cin >> dummy;
                std::getline(std::wcin, graphName);
                std::wprintf(L"함수의 이름을 성공적으로 변경했다!\n");

            }
            else if (input == 22)
            {
                prtFuncName();
            }
            else if (input == 23) //자이로이드 생성
            {
                Func* targetFunc = new Func(funcFlag::scalarField);
                int numPoints = 40; // 공간에 존재하는 점의 수, 많을수록 정확해짐
                double spacing = (2.0 * M_PI) / numPoints; //사이의 공간, 전부 더하면 부피가 됨
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
                            double cutoff = 1.0; //이 값 이상의 자이로이드만 화면에 표시됨
                            if (1)//(value >= cutoff)
                            {
                                targetFunc->myPoints.push_back({ x, y, z });
                                targetFunc->scalar[{x, y, z}] = value;
                                //std::wprintf(L"데이터 {%f,%f,%f}의 val값은 %f이다.\n", x, y, z, value);
                            }
                            //std::wprintf(L"데이터 {%f,%f,%f}를 함수 %d에 입력했다.\n", x, y, z, funcSet.size() - 1);
                        }
                    }
                }
            }
            else if (input == 24)
            {
                int dataIndex = 0;
                std::wprintf(L"몇번째 데이터의 포인트를 출력할까? (0 ~ %d).\n", funcSet.size() - 1);
                prtFuncName();
                std::cin >> dataIndex;
                for (int i = 0; i < ((Func*)funcSet[dataIndex])->myPoints.size(); i++)
                {
                    Point pt = ((Func*)funcSet[dataIndex])->myPoints[i];
                    std::wprintf(L"Index %d : {%.10f,%.10f,%.10f}\n", i, pt.x, pt.y, pt.z);
                }
            }
            else if (input == 25) //데이터 표준화
            {
                int dataIndex = 0;
                std::wprintf(L"몇번째 데이터를 표준화시킬까? (0 ~ %d).\n", funcSet.size() - 1);
                prtFuncName();
                std::cin >> dataIndex;
                ((Func*)funcSet[dataIndex])->normalize(GYROID_PERIOD);
            }
            else if (input == 26)
            {
                std::wprintf(L"Fick's 2nd Law : ∂C/∂t = (∂/∂x)(D(C)∂C/∂x)\n");

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

                //double delX = doubleInput(L"[1] 거리 간격 Δx 입력\n");
                //double delT = doubleInput(L"[2] 시간 간격 Δt 입력\n");
                //int xNum = intInput(L"[4-1] 거리 시행 횟수 입력\n");
                //int tNum = intInput(L"[4-2] 시간 시행 횟수 입력\n");
                //double initConc = doubleInput(L"[3-1] 시간 초기조건 입력 C(x,t=0)\n");
                //double startDistBC = doubleInput(L"[3-2] 거리 0 경계조건 입력 C(x=0,t)\n");
                //double infDistBC = doubleInput(L"[3-3] 거리 ∞ 경계조건 입력 C(x=∞,t)\n");

                std::function<double(double)> diffFunc; //농도에 따라 변화하는 확산계수 D(C)


                std::wprintf(L"확산계수 D(C) 식의 형태를 입력해주세요.\n");
                std::wprintf(L"1. Constant\n");
                std::wprintf(L"2. Polynomial\n");
                std::wprintf(L"3. Exponential\n");
                std::wprintf(L"4. Logarithm\n");
                int diffType;
                std::cin >> diffType;


                if (diffType == 1)
                {
                    std::wprintf(L"D(C) = const\n");
                    std::wprintf(L"확산계수(const) 입력\n");
                    double diffVal;
                    std::cin >> diffVal;

                    diffFunc = [=](double inputConc) -> double
                        {
                            return diffVal;
                        };
                }
                else if (diffType == 2)
                {
                    std::wprintf(L"D(C) = D0(1+γC)\n");
                    std::wprintf(L"확산계수의 초기값 D0 입력\n");
                    double diff0;
                    std::cin >> diff0;
                    std::wprintf(L"농도에 따른 증감배율 γ 입력\n");
                    double diffGamma;
                    std::cin >> diffGamma;

                    diffFunc = [=](double inputConc) -> double
                        {
                            return diff0 * (1 + diffGamma * inputConc);
                        };
                }
                else if (diffType == 3)
                {
                    std::wprintf(L"D(C) = D0*exp(γC)\n");
                    std::wprintf(L"확산계수의 초기값 D0 입력\n");
                    double diff0;
                    std::cin >> diff0;
                    std::wprintf(L"농도에 따른 증감배율 γ 입력\n");
                    double diffGamma;
                    std::cin >> diffGamma;

                    diffFunc = [=](double inputConc) -> double
                        {
                            return diff0 * std::exp(diffGamma * inputConc);
                        };
                }
                else if (diffType == 4)
                {
                    std::wprintf(L"D(C) = D0*ln(1+γC)\n");
                    std::wprintf(L"확산계수의 초기값 D0 입력\n");
                    double diff0;
                    std::cin >> diff0;
                    std::wprintf(L"농도에 따른 증감배율 γ 입력\n");
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
                startDistBC = 0; //한계전류 조건
                infDistBC = initConc;

                std::vector<std::vector<double>> conc;
                std::vector<double> initConcVec;
                //초기화
                for (int i = 0; i < xNum; i++) initConcVec.push_back(initConc);
                conc.push_back(initConcVec);


                bool doPrint = false;
                std::wstring prtAns;
                std::wprintf(L"데이터를 출력하시겠습니까? [y/n]");
                std::wcin >> prtAns;
                if (prtAns == L"y") doPrint = true;
                else doPrint = false;

                int counter = 0;
                const int printInterval = 2;
                std::wstring log = L"";
                //편미분 방정식 계산
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
                        pt.z = t * delT; //청색 시간
                        pt.x = x * delX; //적색 거리
                        pt.y = conc[t][x];
                        targetFunc->myPoints.push_back(pt);

                        
                    }
                }

                std::wstring yn;
                std::wprintf(L"(∂C/∂x)|z=0으로 전류 그래프 i(t)를 그리시겠습니까? [y/n]");
                std::wcin >> yn;

                if (yn == L"y")
                {
                    Func* targetFunc2 = new Func(funcFlag::dim2);
                    targetFunc2->myColor = inputCol();
                    std::vector<double> diffData;
                    //편미분 전류 그래프 생성
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
                std::wprintf(L"모든 계산이 완료되었다.\n");
            }
            else if (input == 27)//trajectory 읽기
            {
                std::wstring file = L"";
                std::wprintf(L"데이터가 있는 파일을 선택해주세요.\n");
                file = openFileDialog();
                std::wprintf(L"파일 %ls 를 대상으로 설정하였다.\n", file.c_str());
                std::wifstream in(file);
                if (in.is_open())
                {
                    readTrjFile(file, 9, -1, 2, 3, 4,1,2);
                    Func* tgtFunc = ((Func*)funcSet[funcSet.size() - 1]);
                    tgtFunc->period = GYROID_PERIOD;

                    //tgtFunc->myPoints.clear();
                    //orthogonal box = (-0.111315 -0.111315 -0.111315) to (10.7379 10.7379 10.7379)
                    //따라서 한변의 지름은 10.7379 - (-0.111315) = 10.849215
                    double length = GYROID_PERIOD;
                    double scaleFactor = 2.0 * M_PI / length;

                    tgtFunc->scalarFunc = [=](double x, double y, double z)->double
                        {
                            return (std::cos(scaleFactor *x) * std::sin(scaleFactor * y) * std::sin(2 * (scaleFactor *z)) + std::cos(scaleFactor *y) * std::sin(scaleFactor *z) * std::sin(2 * (scaleFactor *x)) + std::cos(scaleFactor *z) * std::sin(scaleFactor *x) * std::sin(2 * (scaleFactor *y)));
                        };

                    tgtFunc->scalarInfimum = -1.0;
                    tgtFunc->scalarSupremum = 1.0;
                    tgtFunc->scalarCalc();
                    std::wprintf(L"이 함수의 평균 f값은 %f이다.\n", tgtFunc->scalarAvg());
                }
                else std::wprintf(L"파일을 읽는데 실패하였습니다.\n");
            }
            else if (input == 28)//trajectory 읽기(모든 타임스텝)
            {
                for(int atomType = 1; atomType <=2; atomType++)
                {
                    std::wstring file = L"";
                    std::wprintf(L"데이터가 있는 파일을 선택해주세요.\n");
                    file = openFileDialog();
                    std::wprintf(L"파일 %ls 를 대상으로 설정하였다.\n", file.c_str());
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
                    else std::wprintf(L"파일을 읽는데 실패하였습니다.\n");

                    Func::hasTransform = false;
                }
            }
            else std::wprintf(L"잘못된 값이 입력되었다.\n");
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

        // x축 그리기
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
            // y축 그리기
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
            // z축 그리기
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

        //데이터점 그리기
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
                           //std::wprintf(L"컬러의 값은 %f이다.\n", 0.7 * ((val - tgtFunc->scalarInfimum) / (tgtFunc->scalarSupremum - tgtFunc->scalarInfimum)));
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

        //보간점 그리기
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

        //보간선 그리기 
        if (interLine)
        {
            for (int dataIndex = 0; dataIndex < funcSet.size(); dataIndex++)
            {
                Func* tgtFunc = (Func*)funcSet[dataIndex];
                if (tgtFunc->myInterPoints.size() >= 2)
                {
                    for (int i = 0; i < tgtFunc->myInterPoints.size() - 1; i++)
                    {
                        // 라인 그리기
                        glBegin(GL_LINES);
                        glColor3f(((float)tgtFunc->myColor.r) / 256.0, ((float)tgtFunc->myColor.g) / 256.0, ((float)tgtFunc->myColor.b) / 256.0);
                        glVertex3f(zeroX * xScale * (tgtFunc->myInterPoints[i].x), zeroY * yScale * (tgtFunc->myInterPoints[i].y), zeroZ * zScale * (tgtFunc->myInterPoints[i].z));
                        glVertex3f(zeroX * xScale * (tgtFunc->myInterPoints[i + 1].x), zeroY * yScale * (tgtFunc->myInterPoints[i + 1].y), zeroZ * zScale * (tgtFunc->myInterPoints[i + 1].z));
                        glEnd();
                    }
                }
            }
        }

        //삼각분할 그리기
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

        //크로스 헤어
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

            glColor3f(1.0f, 1.0f, 1.0f);  // 흰색

            //점
            glPointSize(3.0f);
            glBegin(GL_POINTS);
            glVertex2i(cX, cY);
            glEnd();


            //직선 4개
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


