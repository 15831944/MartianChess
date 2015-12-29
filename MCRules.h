#ifndef _MC_RULES_H
#define _MC_RULES_H

enum SelMode {SEL_PIECE, SEL_BOARD, SEL_NONE, SEL_OVER};

struct BoardInfo
{
	int Size;
	int Red, Green, Blue;
	int Name;
	bool IsBlack;
};

struct MCMove
{
	int X1,Y1;
	int X2,Y2;
};

void SetupGame(int inNum);
void AddPiece(int inX, int inY, int inSize);
bool IsLegalPath(int px, int py, int sx, int sy);
bool IsLegalMove(int px, int py, int sx, int sy);
bool IsLegalPiece(int px, int py);
bool IsGameOver();
void MovePiece(int px, int py, int sx, int sy);

void PointSelected(int xPos, int yPos);

#endif