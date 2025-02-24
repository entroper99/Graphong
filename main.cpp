#define GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_USE_GLOG_EXPORT
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
import exAddOn;
import selectOption;


int main(int argc, char** argv)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
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

    fftw_init_threads();
    fftw_make_planner_thread_safe();

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
            selectOption();
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

        if (darkMode==false) glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        else glClearColor(0.1, 0.1f, 0.1f, 1.0f);

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
            if (darkMode == true) glColor3f(1.0, 0.0, 0.0);
            else glColor3f(0.0, 0.0, 0.0);
            glVertex2f(-axisLength / 2.0, 0.0);
            glVertex2f(+axisLength / 2.0, 0.0);
            glEnd();
            for (float x = -axisLength / 2.0; x <= axisLength / 2.0; x += 1.0) {
                glBegin(GL_LINES);
                if (x == 0);
                else if (fmod(x, 10.0) == 0.0)
                {
                    if (darkMode == true) glColor3f(1.0, 0.0, 0.0);
                    else glColor3f(0.0, 0.0, 0.0);
                    glVertex3f(x, -0.4, 0.0);
                    glVertex3f(x, 0.4, 0.0);
                }
                else
                {
                    if (darkMode == true) glColor3f(1.0, 0.0, 0.0);
                    else glColor3f(0.0, 0.0, 0.0);
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
            if (darkMode == true) glColor3f(0.0, 1.0, 0.0);
            else glColor3f(0.0, 0.0, 0.0);
            glVertex3f(0.0, -axisLength / 2.0, 0.0);
            glVertex3f(0.0, +axisLength / 2.0, 0.0);
            glEnd();

            glColor3f(1.0, 1.0, 1.0);
            for (float y = -axisLength / 2.0; y <= axisLength / 2.0; y += 1.0) {
                glBegin(GL_LINES);
                if (y == 0);
                else if (fmod(y, 10.0) == 0.0)
                {
                    if (darkMode == true) glColor3f(0.0, 1.0, 0.0);
                    else glColor3f(0.0, 0.0, 0.0);
                    glVertex3f(-0.4, y, 0.0);
                    glVertex3f(0.4, y, 0.0);
                }
                else
                {
                    if (darkMode == true) glColor3f(0.0, 1.0, 0.0);
                    else glColor3f(0.0, 0.0, 0.0);
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
            if(darkMode==true) glColor3f(0.0, 0.0, 1.0);
            else  glColor3f(0.0, 0.0, 0.0);
            glVertex3f(0.0, 0.0, -axisLength / 2.0);
            glVertex3f(0.0, 0.0, +axisLength / 2.0);
            glEnd();

            glColor3f(1.0, 1.0, 1.0);
            for (float z = -axisLength / 2.0; z <= axisLength / 2.0; z += 1.0) {
                glBegin(GL_LINES);
                if (z == 0);
                else if (fmod(z, 10.0) == 0.0)
                {
                    if (darkMode == true) glColor3f(0.0, 0.0, 1.0);
                    else  glColor3f(0.0, 0.0, 0.0);
                    glVertex3f(0.0, -0.4, z);
                    glVertex3f(0.0, 0.4, z);
                }
                else
                {
                    if (darkMode == true) glColor3f(0.0, 0.0, 1.0);
                    else  glColor3f(0.0, 0.0, 0.0);
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
        for (int dataIndex = 0; dataIndex < funcSet.size(); dataIndex++)
        {
            Func* tgtFunc = (Func*)funcSet[dataIndex];
            if (tgtFunc->interLine == true)
            {
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

        //격자상수 경계 그리기
        //for (int dataIndex = 0; dataIndex < funcSet.size(); dataIndex++)
        //{
        //    Func* tgtFunc = (Func*)funcSet[dataIndex];
        //    if (tgtFunc->latticeConstant != 0)
        //    {
        //        // 라인 그리기
        //        glBegin(GL_LINES);
        //        glColor3f(0.0 / 256.0, 255.0 / 256.0, 255.0 / 256.0);

        //        if (camFixMinusZ || camFixZ)
        //        {
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, 0.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, 0.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, 0.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, 0.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, 0.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, 0.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, 0.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, 0.0);
        //        }
        //        else if (camFixMinusX || camFixX)
        //        {
        //            glVertex3f(0.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(0.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(0.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(0.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(0.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(0.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(0.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(0.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //        }
        //        else if (camFixMinusY || camFixY)
        //        {
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, 0.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, 0.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, 0.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, 0.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, 0.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, 0.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, 0.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, 0.0, -tgtFunc->latticeConstant / 2.0);
        //        }
        //        else
        //        {
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(+tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //            glVertex3f(-tgtFunc->latticeConstant / 2.0, -tgtFunc->latticeConstant / 2.0, +tgtFunc->latticeConstant / 2.0);
        //        }
        //        glEnd();
        //    }
        //}

        //삼각분할 그리기
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

                glBegin(GL_TRIANGLES);
                glColor3f(((float)tgtFunc->myColor.r) / 256.0 / 4.0, ((float)tgtFunc->myColor.g) / 256.0 / 4.0, ((float)tgtFunc->myColor.b) / 256.0 / 4.0);
                glVertex3f(tgtFunc->triangles[i].p1.x* xScale, tgtFunc->triangles[i].p1.y* yScale, tgtFunc->triangles[i].p1.z* zScale);
                glVertex3f(tgtFunc->triangles[i].p2.x* xScale, tgtFunc->triangles[i].p2.y* yScale, tgtFunc->triangles[i].p2.z* zScale);
                glVertex3f(tgtFunc->triangles[i].p3.x* xScale, tgtFunc->triangles[i].p3.y* yScale, tgtFunc->triangles[i].p3.z* zScale);

                glEnd();
            }
        }


        glColor3f(1.0, 1.0, 1.0);

        if (visUI)
        {
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
        }


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

            if(darkMode) glColor3f(1.0f, 1.0f, 1.0f);  // 흰색
            else  glColor3f(0.1f, 0.1f, 0.1f);            

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


