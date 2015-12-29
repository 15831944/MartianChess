#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include "MCDraw.h"
#include "MCRules.h"
#include "MCThread.h"

int PlayerNum, WhoseTurn, NumPlayers, HighestScore;
float EyeX,EyeY,EyeZ;
int Scores[4];
int Chosen,PieceX,PieceY,BoardX,BoardY,LastX,LastY,CurX,CurY;
char *ErrorMsg;
char PlayerNames[4][16];
int MaxNameLen;
bool NetGame;
bool GameOver;
SelMode CurrSelMode;
BoardInfo TheBoard[8][8];
char TitleInfo[256];

void SetupGame(int inNum)
{
	PlayerNum = inNum;
	WhoseTurn = 0;
	HighestScore = 0;
	CurrSelMode = SEL_NONE;
	Chosen = 0;
	GameOver = false;
	PieceX = PieceY = BoardX = BoardY = LastX = LastY = -1;

	switch (PlayerNum){
	case 0:
		EyeX = (float)(NumPlayers == 4 ? 30 : 0); EyeZ = -30; break;
	case 1:
		EyeX = (float)(NumPlayers == 4 ? 30 : 0); EyeZ = 30; break;
	case 2:
		EyeX = -30; EyeZ = 30; break;
	case 3:
		EyeX = -30; EyeZ = -30; break;
	}
	EyeY = 13;

	for (int x = 0; x < 4; x++)
		Scores[x] = 0;
	
	bool IsBlack = false;
	int Num = 1;
	for (int x = 0; x < 8; x++)
	{
		for (int y = 0; y < 8; y++)
		{
			TheBoard[x][y].Size=
				TheBoard[x][y].Red=
				TheBoard[x][y].Green=
				TheBoard[x][y].Blue=0;
			TheBoard[x][y].Name=Num++;
			TheBoard[x][y].IsBlack=IsBlack;
			IsBlack = !IsBlack;
		}
		IsBlack = !IsBlack;
	}
	srand((unsigned int)time(NULL));

	//Player 0
	if (NumPlayers == 4)
	{
		AddPiece(6,0,3); AddPiece(7,1,3); AddPiece(7,0,3);
		AddPiece(5,0,2); AddPiece(6,1,2); AddPiece(7,2,2);
		AddPiece(5,1,1); AddPiece(5,2,1); AddPiece(6,2,1);
	}
	else
	{
		AddPiece(4,0,3); AddPiece(4,1,3); AddPiece(5,0,3);
		AddPiece(6,0,2); AddPiece(5,1,2); AddPiece(4,2,2);
		AddPiece(6,1,1); AddPiece(6,2,1); AddPiece(5,2,1);
	}
	
	//Player 1
	AddPiece(6,7,3); AddPiece(7,6,3); AddPiece(7,7,3);
	AddPiece(5,7,2); AddPiece(6,6,2); AddPiece(7,5,2);
	AddPiece(5,5,1); AddPiece(5,6,1); AddPiece(6,5,1);
	
	//Player 2
	AddPiece(0,6,3); AddPiece(0,7,3); AddPiece(1,7,3);
	AddPiece(0,5,2); AddPiece(1,6,2); AddPiece(2,7,2);
	AddPiece(1,5,1); AddPiece(2,5,1); AddPiece(2,6,1);
	
	//Player 3
	AddPiece(0,0,3); AddPiece(0,1,3); AddPiece(1,0,3);
	AddPiece(0,2,2); AddPiece(1,1,2); AddPiece(2,0,2);
	AddPiece(1,2,1); AddPiece(2,2,1); AddPiece(2,1,1);

	if (!NetGame && NumPlayers == 2)
	{
		sprintf(TitleInfo,"Martian Chess - %s","Two Player Game");
	}
	else if (!NetGame && NumPlayers == 4)
	{
		sprintf(TitleInfo,"Martian Chess - %s","Four Player Game");
	}
	else if (NetGame)
	{
		sprintf(TitleInfo,"Martian Chess - Playing as %s",PlayerNames[PlayerNum]);
	}
}

void PointSelected(int xPos, int yPos)
{
	if (WhoseTurn != PlayerNum)
		return;

	if (CurrSelMode!=SEL_NONE)
		ProcessSelection(xPos,yPos);
	if (CurrSelMode==SEL_PIECE && Chosen > 0 && Chosen <= 64)
	{
		int x = (Chosen - 1) / 8;
		int y = Chosen - (x * 8) - 1;

		if (IsLegalPiece(x,y))
		{
			PieceX=x; PieceY=y;
			CurrSelMode=SEL_BOARD;
		}
	}
	else if (CurrSelMode==SEL_BOARD && Chosen > 0 && Chosen <= 64)
	{
		int x = (Chosen - 1) / 8;
		int y = Chosen - (x * 8) - 1;

		CurrSelMode=SEL_PIECE;
		if (PieceX!=x || PieceY!=y) //The player didn't select the orignal space. 
		{
			if (IsLegalMove(PieceX,PieceY,x,y))
			{
				if (!NetGame)
					MovePiece(PieceX,PieceY,x,y);
				else
				{
					char MoveMsg[19];
					sprintf(MoveMsg,"MOVE:%i:08:r%ic%ir%ic%i",PlayerNum,PieceX,PieceY,x,y);
					SendToServer(MoveMsg);
				}
			}
			else
				ShowError();
		}
		PieceX=-1; PieceY=-1;
		Chosen=-1;
	}
}

void AddPiece(int inX, int inY, int inSize)
{
	TheBoard[inX][inY].Size=inSize;
	TheBoard[inX][inY].Red=rand()%255;
	TheBoard[inX][inY].Green=rand()%255;
	TheBoard[inX][inY].Blue=rand()%255;
}

#define SIGN(a) ((a) == 0 ? 0 : ((a) > 0 ? 1 : -1))

bool IsLegalPath(int px, int py, int sx, int sy)
{
	bool Res=true;
	int DiffX,DiffY,x,y;

	DiffX = SIGN(sx - px);
	DiffY = SIGN(sy - py);

	x=px+DiffX;
	y=py+DiffY;

	while(x!=sx || y!=sy)
	{
		if (TheBoard[x][y].Size!=0)
			return false;
		if (x!=sx)
			x+=DiffX;
		if (y!=sy)
			y+=DiffY;
	}

	return Res;
}

bool IsLegalMove(int px, int py, int sx, int sy)
{
	bool Res = true;

	int CurrSize = TheBoard[px][py].Size;
	int DifX = abs(sx-px);
	int DifY = abs(sy-py);

	if (CurrSize == 1) //Pawn's movement
	{
		if ((DifX != 1) || (DifY != 1))
		{
			Res = false;
			ErrorMsg = "Pawns only move 1 space diagonally";
		}
	}
	else if (CurrSize == 2) //Drone's movement
	{
		if (sx != px && sy != py) //Not moving straight
		{
			Res = false;
			ErrorMsg = "Drones must move straight";
		}
		if ((DifX > 2) || (DifY > 2)) //Moving more than 2 spaces
		{
			Res = false;
			ErrorMsg = "Drones only move 1 or 2 spaces";
		}
	}
	else if (CurrSize == 3) //Queen's movement
	{
		if (DifX != 0 && DifY != 0) //Not moving straight
		{
			if (DifX != DifY) //Not moving along a diagonal
			{
				Res = false;
				ErrorMsg = "Queens only move along straight paths";
			}
		}
	}

	// Make sure the player isn't attacking their own piece
	if (TheBoard[sx][sy].Size != 0 && ((PlayerNum == 0 && sx >= 4 && sy <= 3) || 
												 (PlayerNum == 1 && sx >= 4 && sy >= 4) ||
												 (PlayerNum == 2 && sx <= 3 && sy >= 4) || 
												 (PlayerNum == 3 && sx <= 3 && sy <= 3)))
	{
		Res = false;
		ErrorMsg = "You may not capture your own piece";
	}

	// Make sure the path chosen is not blocked
	if (Res && !IsLegalPath(px,py,sx,sy))
	{
		Res = false;
		ErrorMsg = "That path is blocked";
	}

	// Make sure the piece isn't rejecting the previous move - 2 Player game only
	if (NumPlayers == 2 && sx == LastX && sy == LastY && px == CurX && py == CurY)
	{
		Res = false;
		ErrorMsg = "You may not reject the previous move";
	}

	return Res;
}

bool IsLegalPiece(int px, int py)
{
	if (PlayerNum==0 && px>=4 && py<=3)
		return true;
	else if (PlayerNum==1 && px>=4 && py>=4)
		return true;
	else if (PlayerNum==2 && px<=3 && py>=4)
		return true;
	else if (PlayerNum==3 && px<=3 && py<=3)
		return true;

	MessageBox(NULL,"You may not move a piece that is not yours.","Illegal Move",MB_OK);

	return false;
}

bool IsGameOver()
{
	bool Res = true;
	int StartX, EndX, StartY, EndY;
	switch (WhoseTurn)
	{
	case 0:
		StartX = 4; EndX = 8; StartY = 0; EndY = 4; break;
	case 1:
		StartX = 4; EndX = 8; StartY = 4; EndY = 8; break;
	case 2:
		StartX = 0; EndX = 4; StartY = 4; EndY = 8; break;
	case 3:
		StartX = 0; EndX = 4; StartY = 0; EndY = 4; break;
	default:
		StartX = 0; EndX = 0; StartY = 0; EndY = 0; break;
	}

	for (int x = StartX; x < EndX && Res == true; x++)
	{
		for (int y = StartY; y < EndY && Res == true; y++)
		{
			if (TheBoard[x][y].Size != 0)
				Res = false;
		}
	}

	return Res;
}

void MovePiece(int px, int py, int sx, int sy)
{
	Scores[WhoseTurn]+=TheBoard[sx][sy].Size;

	//Record where the piece came from
	LastX = px; LastY = py;
	CurX = sx; CurY = sy;

	//Move the piece to its new space
	TheBoard[sx][sy].Size = TheBoard[px][py].Size;
	TheBoard[sx][sy].Red = TheBoard[px][py].Red;
	TheBoard[sx][sy].Green = TheBoard[px][py].Green;
	TheBoard[sx][sy].Blue = TheBoard[px][py].Blue;

	//Remove the piece from its old space
	if ((px!=sx) || (py!=sy))
		TheBoard[px][py].Size = 0;

	//Check to see if the game has ended
	if (IsGameOver())
	{
		char OutMsg[256];
		// Everyone will send this, the server will only read one copy of it
		if (NetGame)
		{
			sprintf(OutMsg,"OVER:%i:08:GAMEOVER",PlayerNum);
			SendToServer(OutMsg);
		}
		HighestScore = 0;
		for (int x = 0; x < 4; x++)
		{
			if (Scores[x] > HighestScore)
				HighestScore = Scores[x];
		}
		MessageBox(NULL,"The game has ended.","Game Over",MB_OK);
		CurrSelMode = SEL_OVER;
		WhoseTurn = -1;
		PlayerNum = -1;
		GameOver = true;
	}
	else
	{
		//Go on to the next player
		WhoseTurn = (WhoseTurn + 1) % NumPlayers;
		if (!NetGame)
			PlayerNum = (PlayerNum + 1) % NumPlayers;
	}
}