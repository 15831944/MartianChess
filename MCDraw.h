#ifndef _MC_DRAW_H
#define _MC_DRAW_H

#include <gl/gl.h>
#include <gl/glu.h>


void SetDCPixelFormat(HDC hDC);
void InitGL();
void ChangeSize(int w, int h);
void RenderScene();
void ProcessSelection(int xPos, int yPos);

void DrawPiece();
void DrawSpaceXY(int inX, int inY);
void DrawBoardAndPieces();

void ShowError();

#endif