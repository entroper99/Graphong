#include <SDL.h>

export module inputCol;

import std;

export SDL_Color inputCol()
{
    std::string str;
    std::wprintf(L"R,G,B 값을 입력해주세요. eg) 255,32,180 \n");
    std::wprintf(L"또는 알파벳으로 기준 색상 입력 \n");
    std::wprintf(L"[a] white   [b] red   [c] green  [d] blue   [e] yellow   [f] pink   \n[g] purple   [h] skyBlue   [i] lightGreen   [j] orange\n");
    std::cin >> str;

    if (str == "a") return { 0xff,0xff,0xff };
    if (str == "b") return { 0xdc,0x3e,0x3e };
    if (str == "c") return { 0x58,0xef,0x8e };
    if (str == "d") return { 0x1b,0x73,0xe1 };
    if (str == "e") return { 0xd4,0xe1,0x1b };
    if (str == "f") return { 0xd4,0x66,0xa1 };
    if (str == "g") return { 0xab,0x66,0xd4 };
    if (str == "h") return { 0x66,0xb4,0xd4 };
    if (str == "i") return { 0xb0,0xff,0xa8 };
    if (str == "j") return { 0xff,0xa0,0x53 };


    SDL_Color rtnCol;
    size_t pos = 0;
    int index = 0;

    try
    {
        rtnCol.r = stoi(str.substr(0, str.find(',')));
        str.erase(0, str.find(',') + 1);

        rtnCol.g = stoi(str.substr(0, str.find(',')));
        str.erase(0, str.find(',') + 1);

        rtnCol.b = stoi(str);

        return rtnCol;
    }
    catch (const std::invalid_argument& e)
    {
        return { 255,255,255 };
    }
}