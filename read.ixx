#include <SDL.h>
#include <OpenXLSX.hpp>

export module read;

import globalVar;
import constVar;
import std;
import Func;

using namespace OpenXLSX;

std::string convertXlsxToString(const std::string& filePath, char delimiter) 
{
    XLDocument doc;
    doc.open(filePath);

    auto sheetNames = doc.workbook().sheetNames();
    if (sheetNames.empty()) {
        throw std::runtime_error("The workbook does not contain any sheets.");
    }
    std::string firstSheetName = sheetNames.front();
    std::wprintf(L"xlsx ������ ù��° ��Ʈ�� �н��ϴ�.\n");
    auto wks = doc.workbook().worksheet(firstSheetName);

    std::string result;

    for (auto row = 1; row <= wks.rowCount(); ++row) 
    {
        for (auto col = 1; col <= wks.columnCount(); ++col) 
        {
            auto cell = wks.cell(row, col);
            if (cell.value().type() == XLValueType::String) result += cell.value().get<std::string>();
            else if (cell.value().type() == XLValueType::Integer) result += std::to_string(cell.value().get<int>());
            else if (cell.value().type() == XLValueType::Float) result += std::to_string(cell.value().get<double>());
            else result += "";

            if (col < wks.columnCount()) result += delimiter;
        }
        result += '\n';
    }

    doc.close();
    return result;
}

std::wstring getFileExtension(const std::wstring& filePath) 
{
    size_t pos = filePath.find_last_of(L".");
    if (pos == std::wstring::npos) return L"";
    return filePath.substr(pos);
}

std::string wstringToString(const std::wstring& wstr) 
{
    std::string str(wstr.begin(), wstr.end());
    return str;
}

export int readXY(std::wstring file, int startLine, int endLine, int xCol, int yCol, SDL_Color inputCol)
{
    std::string str;
    char delimiter = 0x1F; // ������ �ʴ� Ư������

    if (getFileExtension(file) == L".xlsx")
    {
        std::wprintf(L"xlsx ������ �����Ͽ���.\n");
        str = convertXlsxToString(wstringToString(file), delimiter);
    }
    else
    {
        std::ifstream in(wstringToString(file));
        if (in.is_open() == false)
        {
            std::wprintf(file.c_str());
            std::wprintf(L" ������ �дµ� �����Ͽ���.\n");
            return -1;
        }

        in.seekg(0, std::ios::end);
        size_t size = in.tellg();
        str.resize(size);
        in.seekg(0, std::ios::beg);
        in.read(&str[0], size);
        in.close(); // ���� �б� ����
    }

    Func* targetFunc = new Func(funcFlag::dim2);
    targetFunc->funcName = file;
    targetFunc->myColor = inputCol;

    int enterNumber = 0;
    bool numberReadStart = false;
    int numStartIndex = -1;
    int numEndIndex = -1;
    int numberReaded = 0;
    float prevValX = 0;
    int thisLineCurrentNumber = 0;
    int numLF = 0;

    for (size_t i = 0; i < str.size(); i++)
    {
        if (numberReadStart == false)
        {
            if (enterNumber == startLine)
            {
                std::wprintf(L"���ݺ��� ���ڸ� �н��ϴ�.\n");
                numberReadStart = true;
                i--;
            }
            else
            {
                if (str[i] == UNI::LF) enterNumber++;
            }
        }
        else
        {
            if (numStartIndex == -1)
            {
                if ((str[i] >= UNI::ZERO && str[i] <= UNI::NINE) || str[i] == 0x2d || str[i] == UNI::PERIOD) numStartIndex = i;
                else if ((str[i] >= UNI::A && str[i] <= UNI::Z) || (str[i] >= UNI::a && str[i] <= UNI::z))
                {
                    std::wprintf(L"���� �б⸦ �Ϸ��Ͽ���!\n");
                    break;
                }
            }
            else
            {
                if (str[i] == delimiter || str[i] == UNI::LF || str[i] == UNI::SPACE || str[i] == UNI::TAB || str[i] == UNI::COMMA)
                {
                    numEndIndex = i - 1;
                    float floatValue = std::stof(str.substr(numStartIndex, numEndIndex - numStartIndex + 1));
                    if (thisLineCurrentNumber == xCol) prevValX = floatValue;
                    else if (thisLineCurrentNumber == yCol)
                    {
                        targetFunc->myPoints.push_back({ prevValX, floatValue, 0 });
                        std::wprintf(L"%d��° ������ (%f,%f)�� �����ͼ¿� �־���.\n", targetFunc->myPoints.size(), prevValX, floatValue);
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
    std::wprintf(L"�߰��� �Լ��� ���� ���� %d���̰�, ���� %d���� �Լ��� �����Ѵ�.\n", targetFunc->myPoints.size(), funcSet.size());
    return funcSet.size() - 1;
}


export int readXYZ(std::wstring file, int startLine, int endLine, int xCol, int yCol, int zCol, SDL_Color inputCol)
{
    std::string str;
    char delimiter = 0x1F; // ������ �ʴ� Ư������

    if (getFileExtension(file) == L".xlsx")
    {
        std::wprintf(L"xlsx ������ �����Ͽ���.\n");
        str = convertXlsxToString(wstringToString(file), delimiter);
    }
    else
    {
        std::ifstream in(wstringToString(file));
        if (in.is_open() == false)
        {
            std::wprintf(file.c_str());
            std::wprintf(L" ������ �дµ� �����Ͽ���.\n");
            return -1;
        }

        in.seekg(0, std::ios::end);
        size_t size = in.tellg();
        str.resize(size);
        in.seekg(0, std::ios::beg);
        in.read(&str[0], size);
        in.close(); // ���� �б� ����
    }

    Func* targetFunc = new Func(funcFlag::dim3);
    targetFunc->funcName = file;
    targetFunc->myColor = inputCol;

    int enterNumber = 0;
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

    for (size_t i = 0; i < str.size(); i++)
    {
        if (numberReadStart == false)
        {
            if (enterNumber == startLine)
            {
                std::wprintf(L"���ݺ��� ���ڸ� �н��ϴ�.\n");
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
                if ((str[i] >= UNI::ZERO && str[i] <= UNI::NINE) || str[i] == 0x2d || str[i] == UNI::PERIOD)
                {
                    numStartIndex = i;
                }
                else if ((str[i] >= UNI::A && str[i] <= UNI::Z) || (str[i] >= UNI::a && str[i] <= UNI::z))
                {
                    std::wprintf(L"���� �б⸦ �Ϸ��Ͽ���!\n");
                    break;
                }
            }
            else
            {
                if (str[i] == delimiter || str[i] == UNI::LF || str[i] == UNI::SPACE || str[i] == UNI::TAB || str[i] == UNI::COMMA)
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
                        std::wprintf(L"%d��° ������ (%f,%f,%f)�� �����ͼ¿� �־���.\n", targetFunc->myPoints.size(), firstVal, secondVal, thirdVal);
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
    std::wprintf(L"�߰��� �Լ��� ���� ���� %d���̰�, ���� %d���� �Լ��� �����Ѵ�.\n", targetFunc->myPoints.size(), funcSet.size());
    return funcSet.size() - 1;
}


export int readTrjFile(std::wstring file, int startLine, int endLine, int xCol, int yCol, int zCol, int atomCol, int atomType)
{
    std::ifstream in(file);
    if (in.is_open() == false) return -1;

    Func* targetFunc = new Func(funcFlag::scalarField);
    targetFunc->funcName = file;

    int enterNumber = 0;
    std::string str;
    std::string strFragment;//ǥ �� ĭ�� ���ڿ��� ����Ǵ� ��ü, �Ź� �ʱ�ȭ��

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
    bool skipThis = false;

    int thisLineCurrentNumber = 0;
    int numLF = 0;
    for (int i = 0; i < str.size(); i++)
    {
        if (numberReadStart == false)
        {
            if (enterNumber == startLine)
            {
                std::wprintf(L"���ݺ��� ���ڸ� �н��ϴ�.\n");
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
                    //std::wprintf(L"���� ���� �����ڵ带 �߰��Ͽ���.\n");
                }
                else if ((str[i] >= UNI::A && str[i] <= UNI::Z) || (str[i] >= UNI::a && str[i] <= UNI::z))
                {
                    std::wprintf(L"���� �б⸦ �Ϸ��Ͽ���!\n");
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
                    else if (thisLineCurrentNumber == atomCol)
                    {
                        if (floatValue != atomType)
                        {
                            skipThis = true;
                        }
                    }

                    if (findFirst && findSecond && findThird)
                    {
                        if (skipThis == false)
                        {
                            targetFunc->myPoints.push_back({ firstVal, secondVal, thirdVal });
                            std::wprintf(L"%d��° ������ (%f,%f,%f)�� �����ͼ¿� �־���.\n", targetFunc->myPoints.size(), firstVal, secondVal, thirdVal);
                        }
                        findFirst = false;
                        findSecond = false;
                        findThird = false;
                        skipThis = false;
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
    std::wprintf(L"�߰��� �Լ��� ���� ���� %d���̰�, ���� %d���� �Լ��� �����Ѵ�.\n", targetFunc->myPoints.size(), funcSet.size());
    return funcSet.size() - 1;
}


export int readTrjString(std::string inputStr, int startLine, int endLine, int xCol, int yCol, int zCol, int atomCol, int atomType)
{
    Func* targetFunc = new Func(funcFlag::scalarField);
    targetFunc->funcName = L"NONE";

    int enterNumber = 0;
    std::string str = inputStr;
    std::string strFragment;//ǥ �� ĭ�� ���ڿ��� ����Ǵ� ��ü, �Ź� �ʱ�ȭ��

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
    bool skipThis = false;

    int thisLineCurrentNumber = 0;
    int numLF = 0;
    for (int i = 0; i < str.size(); i++)
    {
        if (numberReadStart == false)
        {
            if (enterNumber == startLine)
            {
                std::wprintf(L"���ݺ��� ���ڸ� �н��ϴ�.\n");
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
                    //std::wprintf(L"���� ���� �����ڵ带 �߰��Ͽ���.\n");
                }
                else if ((str[i] >= UNI::A && str[i] <= UNI::Z) || (str[i] >= UNI::a && str[i] <= UNI::z))
                {
                    std::wprintf(L"���� �б⸦ �Ϸ��Ͽ���!\n");
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
                    else if (thisLineCurrentNumber == atomCol)
                    {
                        if (floatValue != atomType)
                        {
                            skipThis = true;
                        }
                    }

                    if (findFirst && findSecond && findThird)
                    {
                        if (skipThis == false)
                        {
                            targetFunc->myPoints.push_back({ firstVal, secondVal, thirdVal });
                            //std::wprintf(L"%d��° ������ (%f,%f,%f)�� �����ͼ¿� �־���.\n", targetFunc->myPoints.size(), firstVal, secondVal, thirdVal);
                        }
                        findFirst = false;
                        findSecond = false;
                        findThird = false;
                        skipThis = false;
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
    std::wprintf(L"�߰��� �Լ��� ���� ���� %d���̰�, ���� %d���� �Լ��� �����Ѵ�.\n", targetFunc->myPoints.size(), funcSet.size());
    return funcSet.size() - 1;
}

