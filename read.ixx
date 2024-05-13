#include <SDL.h>

export module read;

import globalVar;
import constVar;
import std;
import Func;


export int readXY(std::wstring file, int startLine, int endLine, int xCol, int yCol, SDL_Color inputCol)
{
    std::ifstream in(file);
    if (in.is_open() == false)
    {
        std::wprintf(file.c_str());
        std::wprintf(L" 파일을 읽는데 실패하였다.\n");
        return -1;
    }

    Func* targetFunc = new Func();
    funcSet.push_back(targetFunc);

    targetFunc->funcName = file;
    targetFunc->myColor = inputCol;

    int enterNumber = 0;

    std::string str;
    std::string strFragment;//표 한 칸의 문자열이 저장되는 객체, 매번 초기화됨

    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    str.resize(size);
    in.seekg(0, std::ios::beg);
    in.read(&str[0], size);
    in.close(); // 파일 읽기 종료


    bool numberReadStart = false;
    int numStartIndex = -1;
    int numEndIndex = -1;
    int numberReaded = 0;
    float prevValX = 0;
    int thisLineCurrentNumber = 0;
    int numLF = 0;
    for (int i = 0; i < str.size(); i++)
    {
        if (numberReadStart == false)
        {
            if (enterNumber == startLine)
            {
                std::wprintf(L"지금부터 숫자를 읽습니다.\n");
                numberReadStart = true;
                i--;
            }
            else
            {
                if (str[i] == UNI::LF)
                {
                    enterNumber++;
                }
            }
        }
        else
        {
            if (numStartIndex == -1)
            {
                if ((str[i] >= UNI::ZERO && str[i] <= UNI::NINE) || str[i] == 0x2d)
                {
                    numStartIndex = i;
                    //std::wprintf(L"숫자 시작 유니코드를 발견하였다.\n");
                }
                else if ((str[i] >= UNI::A && str[i] <= UNI::Z) || (str[i] >= UNI::a && str[i] <= UNI::z))
                {
                    std::wprintf(L"파일 읽기를 완료하였다!\n");
                    break;
                }
            }
            else
            {
                if (((str[i] >= UNI::ZERO && str[i] <= UNI::NINE) || str[i] == UNI::MINUS_SIGN || str[i] == UNI::PERIOD));
                else
                {

                    numEndIndex = i - 1;
                    float floatValue = std::stof(str.substr(numStartIndex, numEndIndex - numStartIndex + 1));
                    if (thisLineCurrentNumber == xCol)
                    {
                        //std::wprintf(L"첫번째 x값을 입력하였다.\n");
                        prevValX = floatValue;
                    }
                    else if (thisLineCurrentNumber == yCol)
                    {
                        targetFunc->myPoints.push_back({ prevValX, floatValue,0 });
                        std::wprintf(L"%d번째 데이터 (%f,%f)를 데이터셋에 넣었다.\n", targetFunc->myPoints.size(), prevValX, floatValue);
                    }
                    numberReaded++;
                    numStartIndex = -1;
                    thisLineCurrentNumber++;
                }
            }

            if (str[i] == UNI::LF)
            {
                thisLineCurrentNumber = 0;
                if (endLine != -1 && endLine == numLF) break;
                numLF++;
            }

        }
    }
    targetFunc->myInterPoints = targetFunc->myPoints;
    std::wprintf(L"추가한 함수의 점의 수는 %d개이고, 현재 %d개의 함수가 존재한다.\n", targetFunc->myPoints.size(), funcSet.size());
    return funcSet.size()-1;
}

export int readXYZ(std::wstring file, int startLine, int endLine, int xCol, int yCol, int zCol, SDL_Color inputCol)
{
    std::ifstream in(file);
    if (in.is_open() == false) return -1;

    Func* targetFunc = new Func();
    funcSet.push_back(targetFunc);
    targetFunc->funcName = file;
    targetFunc->myColor = inputCol;

    int enterNumber = 0;
    std::string str;
    std::string strFragment;//표 한 칸의 문자열이 저장되는 객체, 매번 초기화됨

    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    str.resize(size);
    in.seekg(0, std::ios::beg);
    in.read(&str[0], size);
    in.close();

    bool numberReadStart = false;
    int numStartIndex = -1;
    int numEndIndex = -1;
    int numberReaded = 0;

    float firstVal = 0;
    float secondVal = 0;
    float thirdVal = 0;

    bool findFirst = false;
    bool findSecond = false;
    bool findThird = false;

    int thisLineCurrentNumber = 0;
    int numLF = 0;
    for (int i = 0; i < str.size(); i++)
    {
        if (numberReadStart == false)
        {
            if (enterNumber == startLine)
            {
                std::wprintf(L"지금부터 숫자를 읽습니다.\n");
                numberReadStart = true;
            }
            else
            {
                if (str[i] == UNI::LF)
                {
                    enterNumber++;
                }
            }
        }
        else
        {
            if (numStartIndex == -1)
            {
                if ((str[i] >= UNI::ZERO && str[i] <= UNI::NINE) || str[i] == 0x2d)
                {
                    numStartIndex = i;
                    //std::wprintf(L"숫자 시작 유니코드를 발견하였다.\n");
                }
                else if ((str[i] >= UNI::A && str[i] <= UNI::Z) || (str[i] >= UNI::a && str[i] <= UNI::z))
                {
                    std::wprintf(L"파일 읽기를 완료하였다!\n");
                    break;
                }
            }
            else
            {
                if (str[i] == UNI::SPACE || str[i] == UNI::LF || str[i] == UNI::TAB || str[i] == UNI::COMMA)
                {
                    numEndIndex = i - 1;
                    float floatValue = std::stof(str.substr(numStartIndex, numEndIndex - numStartIndex + 1));
                    if (thisLineCurrentNumber == xCol)
                    {
                        firstVal = floatValue;
                        findFirst = true;
                    }
                    else if (thisLineCurrentNumber == yCol)
                    {
                        secondVal = floatValue;
                        findSecond = true;
                    }
                    else if (thisLineCurrentNumber == zCol)
                    {
                        thirdVal = floatValue;
                        findThird = true;
                    }

                    if (findFirst && findSecond && findThird)
                    {
                        targetFunc->myPoints.push_back({ firstVal, secondVal, thirdVal });
                        std::wprintf(L"%d번째 데이터 (%f,%f,%f)를 데이터셋에 넣었다.\n", targetFunc->myPoints.size(), firstVal, secondVal, thirdVal);
                        findFirst = false;
                        findSecond = false;
                        findThird = false;
                    }

                    numberReaded++;
                    numStartIndex = -1;
                    thisLineCurrentNumber++;
                }
            }

            if (str[i] == UNI::LF)
            {
                thisLineCurrentNumber = 0;
                if (endLine != -1 && endLine == numLF) break;
                numLF++;
            }
        }

    }
    targetFunc->myInterPoints = targetFunc->myPoints;
    std::wprintf(L"추가한 함수의 점의 수는 %d개이고, 현재 %d개의 함수가 존재한다.\n", targetFunc->myPoints.size(), funcSet.size());
    return funcSet.size()-1;
}