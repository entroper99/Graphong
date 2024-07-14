#include <windows.h>

export module utilFile;

import std;

export std::wstring openFileDialog()
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