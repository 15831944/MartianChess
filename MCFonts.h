#ifndef _MC_FONTS_H
#define _MC_FONTS_H

#include "stdafx.h"

struct MCFont
{
	GLuint base;
	int widths[256];
	int height;
};

MCFont *MakeMCFont(HDC hdc, const char *typeface, int height, int weight, DWORD italic);
void DestroyMCFont(MCFont *font);
void DrawMCText(MCFont *font, const char *text, int xPos, int yPos);

#endif