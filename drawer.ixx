#include <SDL.h>
#include <SDL_ttf.h>
#include <GL/glew.h>

export module drawer;

import std;

export GLuint LoadTextTexture(const char* text, TTF_Font* font, SDL_Color textColor)
{
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, textColor);
    if (!surface) return 0;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    SDL_FreeSurface(surface);
    return textureID;
}

export GLuint LoadTextTexture(std::wstring text, TTF_Font* font, SDL_Color textColor)
{
    Uint16* unicode = new Uint16[text.size() + 1]();
    for (int i = 0; i < text.size(); i++) { unicode[i] = text[i]; }
    unicode[text.size()] = 0;
    SDL_Surface* surface = TTF_RenderUNICODE_Blended(font, unicode, textColor);//병목 1/3
    if (!surface)
    {
        delete[] unicode;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    SDL_FreeSurface(surface);
    delete[] unicode;
    return textureID;
}

export void drawBillboardText(const std::string& text, TTF_Font* font, SDL_Color color, float x, float y, float z, float scale = 0.05f)
{
    GLuint textureID = LoadTextTexture(text.c_str(), font, color);

    glPushMatrix();
    glTranslatef(x, y, z);
    float modelView[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i == j) modelView[i * 4 + j] = 1.0;
            else modelView[i * 4 + j] = 0.0;
        }
    }
    glLoadMatrixf(modelView);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f(-scale, -scale * 0.5f, 0.0f);
    glTexCoord2f(1.0, 1.0); glVertex3f(scale, -scale * 0.5f, 0.0f);
    glTexCoord2f(1.0, 0.0); glVertex3f(scale, scale * 0.5f, 0.0f);
    glTexCoord2f(0.0, 0.0); glVertex3f(-scale, scale * 0.5f, 0.0f);
    glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glDeleteTextures(1, &textureID);
}


export void drawTextHUD(const std::string& text, TTF_Font* font, SDL_Color color, int x, int y)
{
    GLuint textureID = LoadTextTexture(text.c_str(), font, color);
    if (textureID == 0) return;

    int textWidth, textHeight;
    if (TTF_SizeText(font, text.c_str(), &textWidth, &textHeight) != 0) //텍스트 사이즈 계산 실패
    {
        glDeleteTextures(1, &textureID);
        return;
    }

    // 2D 투영 행렬 설정
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 640, 480, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 텍스트 렌더링
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x + textWidth, y);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x + textWidth, y + textHeight);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y + textHeight);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // 이전 투영 행렬과 모델뷰 행렬 복원
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glDeleteTextures(1, &textureID);
}

export void drawTextHUD(const std::wstring& text, TTF_Font* font, SDL_Color color, int x, int y)
{
    GLuint textureID = LoadTextTexture(text.c_str(), font, color);
    if (textureID == 0) return;

    int* w;
    int* h;
    int textWidth, textHeight;
    Uint16* unicode = new Uint16[text.size() + 1]();
    for (int i = 0; i < text.size(); i++) { unicode[i] = text[i]; }
    unicode[text.size()] = 0; //마지막 널문자

    if (TTF_SizeUNICODE(font, unicode, &textWidth, &textHeight) != 0) //텍스트 사이즈 계산 실패
    {
        delete[] unicode;
        glDeleteTextures(1, &textureID);
        return;
    }

    // 2D 투영 행렬 설정
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 640, 480, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 텍스트 렌더링
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x + textWidth, y);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x + textWidth, y + textHeight);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y + textHeight);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // 이전 투영 행렬과 모델뷰 행렬 복원
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glDeleteTextures(1, &textureID);

    delete[] unicode;
}