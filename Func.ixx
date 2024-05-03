#include <SDL.h>

export module Func;

import std;

export struct Func
{
    std::string funcName = "NULL";
    std::vector<std::array<float, 3>> myPoints;
    std::vector<std::array<float, 3>> myInterPoints;
    SDL_Color myColor = { 0xff,0xff,0xff };
};