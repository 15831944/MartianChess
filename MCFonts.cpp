#include "stdafx.h"
#include "MCFonts.h"
// Note: I copied this code (mostly) out of the Superbible

MCFont *MakeMCFont(HDC hdc, const char *typeface, int height, int weight, DWORD italic)
{
	MCFont *font;
	HFONT fontid;
	int charset;

	if ((font = (MCFont*)calloc(1,sizeof(MCFont))) == NULL)
		return NULL;

	if ((font->base = glGenLists(256)) == 0)
	{
		free(font);
		return NULL;
	}

	if (stricmp(typeface,"symbol") == 0)
		charset = SYMBOL_CHARSET;
	else
		charset = ANSI_CHARSET;

	fontid = CreateFont(
		height, 0, 0, 0, weight, italic, FALSE, FALSE, 
		charset, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 
		DRAFT_QUALITY, DEFAULT_PITCH, typeface);

	SelectObject(hdc, fontid);

	wglUseFontBitmaps(hdc, 0, 256, font->base);
	GetCharWidth(hdc, 0, 255, font->widths);
	font->height = height;

	return font;
}

void DestroyMCFont(MCFont *font)
{
	if (font == NULL)
		return;

	glDeleteLists(font->base, 256);
	free(font);
}

void DrawMCText(MCFont *font, const char *text, int xPos, int yPos)
{
	if (font == NULL || text == NULL)
		return;

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	glPushAttrib(GL_LIST_BIT | GL_TEXTURE_2D | GL_LIGHTING);
	glDisable(GL_TEXTURE_2D); // Having Textures and Lighting enabled messes up drawing the text
	glDisable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0,viewport[2],0,viewport[3],-1,1);

	glColor3f(1.0,1.0,1.0);
	glRasterPos2i(xPos, viewport[3] - yPos);
	glListBase(font->base);
	glCallLists((GLsizei)strlen(text),GL_UNSIGNED_BYTE, text);
	glEnable(GL_TEXTURE_2D);
	glPopAttrib();

	glPopMatrix();
}