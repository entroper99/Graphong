#include <SDL.h>

export module rainbow;

import std;

//0과 1 사이의 값이 입력됨
export SDL_Color rainbow(float value) 
{
    SDL_Color color;
    value = std::max(0.0f, std::min(1.0f, value));
    float hue = value * 360.0f;
    float saturation = 1.0f;
    float lightness = 0.5f;
    float chroma = (1.0f - std::abs(2.0f * lightness - 1.0f)) * saturation;
    float huePrime = hue / 60.0f;
    float x = chroma * (1.0f - std::abs(std::fmod(huePrime, 2.0f) - 1.0f));
    float m = lightness - chroma / 2.0f;
    if (huePrime >= 0.0f && huePrime < 1.0f) 
    {
        color.r = static_cast<Uint8>((chroma + m) * 255.0f);
        color.g = static_cast<Uint8>((x + m) * 255.0f);
        color.b = static_cast<Uint8>(m * 255.0f);
    }
    else if (huePrime >= 1.0f && huePrime < 2.0f) 
    {
        color.r = static_cast<Uint8>((x + m) * 255.0f);
        color.g = static_cast<Uint8>((chroma + m) * 255.0f);
        color.b = static_cast<Uint8>(m * 255.0f);
    }
    else if (huePrime >= 2.0f && huePrime < 3.0f) 
    {
        color.r = static_cast<Uint8>(m * 255.0f);
        color.g = static_cast<Uint8>((chroma + m) * 255.0f);
        color.b = static_cast<Uint8>((x + m) * 255.0f);
    }
    else if (huePrime >= 3.0f && huePrime < 4.0f) 
    {
        color.r = static_cast<Uint8>(m * 255.0f);
        color.g = static_cast<Uint8>((x + m) * 255.0f);
        color.b = static_cast<Uint8>((chroma + m) * 255.0f);
    }
    else if (huePrime >= 4.0f && huePrime < 5.0f) 
    {
        color.r = static_cast<Uint8>((x + m) * 255.0f);
        color.g = static_cast<Uint8>(m * 255.0f);
        color.b = static_cast<Uint8>((chroma + m) * 255.0f);
    }
    else 
    {
        color.r = static_cast<Uint8>((chroma + m) * 255.0f);
        color.g = static_cast<Uint8>(m * 255.0f);
        color.b = static_cast<Uint8>((x + m) * 255.0f);
    }
    color.a = 255; 
    return color;
}