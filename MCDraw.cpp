#include "stdafx.h"
#include <stdio.h>
#include "MCDraw.h"
#include "MCTextures.h"
#include "MCRules.h"
#include "MCFonts.h"


extern int PlayerNum, WhoseTurn, NumPlayers, HighestScore;
extern float EyeX,EyeY,EyeZ;
extern int Scores[4];
extern int Chosen,PieceX,PieceY,BoardX,BoardY;
extern char *ErrorMsg;
extern char PlayerNames[4][16];
extern int MaxNameLen;
extern SelMode CurrSelMode;
extern BoardInfo TheBoard[8][8];

extern GLuint Texture[3];

extern MCFont *theFont;

float EyeRotate;

#define SPACE_SCALE 1.2f
#define SPACE_SHIFT .30f
#define PIECE_OFFSET .1f

GLfloat PieceArray[]={ //The vertices needed to draw a piece
	0.5,0.0,0.5,
	0.5,0.0,-0.5,
	-0.5,0.0,-0.5,
	-0.5,0.0,0.5,
	0,1.75,0};

void SetDCPixelFormat(HDC hDC)
{
	int nPixelFormat;

	static PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 16;

	nPixelFormat = ChoosePixelFormat(hDC,&pfd);

	SetPixelFormat(hDC, nPixelFormat, &pfd);
}

void InitGL()
{
	GLfloat light_ambient[]={0.6f,0.6f,0.6f,1.0f};
	GLfloat light_pos[]={10.0f,10.0f,-10.0f,1.0f};
	glLightfv(GL_LIGHT0,GL_AMBIENT,light_ambient);
	glLightfv(GL_LIGHT0,GL_POSITION,light_pos);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glColorMaterial(GL_FRONT,GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	
	glClearColor(0.0f,0.3f,0.0f,0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_NORMALIZE);
	glFrontFace(GL_CW);
	glPolygonMode(GL_FRONT,GL_FILL);
	glCullFace(GL_BACK);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,0,PieceArray);

	loadGLTextures();
	glEnable(GL_TEXTURE_2D);

	EyeRotate = 0;
}

void ChangeSize(int w, int h)
{
	glViewport(0,0,(GLint)w,(GLint)h);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(25,(GLfloat)w/(GLfloat)h,.1,100);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void RenderScene()
{
	if (WhoseTurn == PlayerNum && CurrSelMode == SEL_NONE)
		CurrSelMode=SEL_PIECE;
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawBoardAndPieces();
	GLint mode;
	glGetIntegerv(GL_RENDER_MODE,&mode);
	if (mode == GL_RENDER)
	{
		char *NameScore = new char[MaxNameLen + 35];
		for (int x = 0; x < NumPlayers; x++)
		{
			char Format[15];
			char *ExtraText = "";
			if (x == WhoseTurn)
				ExtraText = "<=Player to move";
			else if (WhoseTurn == -1 && Scores[x] == HighestScore)
				ExtraText = "Winner!!!";
			sprintf(Format,"%%%is : %%i %%s",MaxNameLen);
			sprintf(NameScore,Format,PlayerNames[x],Scores[x],ExtraText);
			DrawMCText(theFont,NameScore,10,x * 20 + 20);
		}
		delete[] NameScore;
	}
}

void ProcessSelection(int xPos, int yPos)
{
	GLint viewport[4],hits;
	GLuint selbuff[512];

	ZeroMemory(selbuff,512);
	glGetIntegerv(GL_VIEWPORT,viewport);
	glSelectBuffer(512,selbuff);
	glRenderMode(GL_SELECT);

	glInitNames();
	glPushName(0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluPickMatrix(xPos,viewport[3]-yPos,1,1,viewport);
	gluPerspective(25,(GLdouble)viewport[2]/(GLdouble)viewport[3],.1,100);

	RenderScene();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	hits=glRenderMode(GL_RENDER);

	int Name=selbuff[3];
	unsigned int Depth=selbuff[1];
	for (int q=0; q<hits; q++)
	{
		if (selbuff[q*4+1]<Depth)
		{
			Depth=selbuff[q*4+1];
			Name=selbuff[q*4+3];
		}
	}
	Chosen=Name;
}

void DrawPiece()
{
	glMatrixMode(GL_MODELVIEW);
	glBegin(GL_TRIANGLES);
		glNormal3f(.962f,.275f,0.0f);
		glTexCoord2f(1.0,0.0); glArrayElement(0);
		glTexCoord2f(1.0,1.0); glArrayElement(4);
		glTexCoord2f(0.0,0.0); glArrayElement(1);

		glNormal3f(0.0f,.275f,-.962f);
		glTexCoord2f(0.0,0.0); glArrayElement(1);
		glTexCoord2f(1.0,1.0); glArrayElement(4);
		glTexCoord2f(1.0,0.0); glArrayElement(2);

		glNormal3f(-.962f,.275f,0.0f);
		glTexCoord2f(1.0,0.0); glArrayElement(2);
		glTexCoord2f(1.0,1.0); glArrayElement(4);
		glTexCoord2f(0.0,0.0); glArrayElement(3);

		glNormal3f(0.0f,.275f,.962f);
		glTexCoord2f(0.0,0.0); glArrayElement(3);
		glTexCoord2f(1.0,1.0); glArrayElement(4);
		glTexCoord2f(1.0,0.0); glArrayElement(0);
	glEnd();
}

void DrawSpaceXY(int inX, int inY)
{
	glMatrixMode(GL_MODELVIEW);
	glBegin(GL_QUADS);
		glNormal3f(0,1,0);
		glTexCoord2f(0.8f,0.8f); glVertex3i(1,0,1); 
		glTexCoord2f(0.2f,0.8f); glVertex3i(-1,0,1);
		glTexCoord2f(0.2f,0.2f); glVertex3i(-1,0,-1);
		glTexCoord2f(0.8f,0.2f); glVertex3i(1,0,-1);

		if (inY==7 || inY==3)
		{
			glNormal3f(0,0,1); 
			glTexCoord2f(0.2f,0.8f); glVertex3f(-1,0,1);
			glTexCoord2f(0.8f,0.8f); glVertex3f(1,0,1);
			glTexCoord2f(0.8f,1.0f); glVertex3f(1,-.3f,1);
			glTexCoord2f(0.2f,1.0f); glVertex3f(-1,-.3f,1);
		}

		if (inY==0 || inY==4)
		{
			glNormal3f(0,0,-1); 
			glTexCoord2f(0.8f,0.2f); glVertex3f(1,0,-1);
			glTexCoord2f(0.2f,0.2f); glVertex3f(-1,0,-1);
			glTexCoord2f(0.2f,0.0f); glVertex3f(-1,-.3f,-1);
			glTexCoord2f(0.8f,0.0f); glVertex3f(1,-.3f,-1);
		}

		if (inX==7 || inX==3)
		{
			glNormal3f(1,0,0); 
			glTexCoord2f(0.8f,0.8f); glVertex3f(1,0,1);
			glTexCoord2f(0.8f,0.2f); glVertex3f(1,0,-1);
			glTexCoord2f(1.0f,0.2f); glVertex3f(1,-.3f,-1);
			glTexCoord2f(1.0f,0.8f); glVertex3f(1,-.3f,1);
		}

		if (inX==0 || inX==4)
		{
			glNormal3f(-1,0,0); 
			glTexCoord2f(0.2f,0.2f); glVertex3f(-1,0,-1);
			glTexCoord2f(0.2f,0.8f); glVertex3f(-1,0,1);
			glTexCoord2f(0.0f,0.8f); glVertex3f(-1,-.3f,1);
			glTexCoord2f(0.0f,0.2f); glVertex3f(-1,-.3f,-1);
		}
	glEnd();
}

void DrawBoardAndPieces()
{
	// It might be possible to make this function more efficient, but it's working now
	GLint mode;

	glGetIntegerv(GL_RENDER_MODE,&mode);
	int x = 0, xx = -7;
	if (NumPlayers == 2)
	{
		x = 4; xx = 0;
	}
	for (; x < 8; x++, xx+=2)
	{
		for (int y = 0, yy = -7; y < 8; y++, yy+=2)
		{
			//Transformations for both the space and the piece
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			gluLookAt(EyeX,EyeY,EyeZ,0,0,0,0,1,0);
			glRotatef(EyeRotate,0,1,0);
			if (NumPlayers == 2)
				glTranslatef(-(2 * SPACE_SCALE + 5 * SPACE_SHIFT),0,0); // This equation works, not sure why exactly... 
			glTranslatef(xx*SPACE_SCALE,0,yy*SPACE_SCALE);
			if (xx<0)
				glTranslatef(-SPACE_SHIFT,0,0);
			else
				glTranslatef(SPACE_SHIFT,0,0);
			if (yy<0)
				glTranslatef(0,0,-SPACE_SHIFT);
			else
				glTranslatef(0,0,SPACE_SHIFT);

			//Draw the Space
			if (mode==GL_SELECT && CurrSelMode==SEL_BOARD)
				glLoadName(TheBoard[x][y].Name);
			else
				glLoadName(0);
			glPushMatrix();
			glScalef(SPACE_SCALE,1.0,SPACE_SCALE);
			if (TheBoard[x][y].IsBlack)
				glBindTexture(GL_TEXTURE_2D,Texture[0]);
			else
				glBindTexture(GL_TEXTURE_2D,Texture[1]);
			if ((WhoseTurn==0 && x>=4 && y<=3) || (WhoseTurn==1 && x>=4 && y>=4) ||
				 (WhoseTurn==2 && x<=3 && y>=4) || (WhoseTurn==3 && x<=3 && y<=3))
				glColor3f(1.0,1.0,0.75f);
			else
				glColor3f(1.0,1.0,1.0);
			DrawSpaceXY(x,y);
			glPopMatrix();
			
			//Draw the Piece
			if (mode!=GL_SELECT || CurrSelMode==SEL_PIECE)
			{
				if (TheBoard[x][y].Size!=0)
				{
					if (mode==GL_SELECT)
						glLoadName(TheBoard[x][y].Name);
					else
						glLoadName(0);
					
					glPushMatrix();
					glTranslatef(0,PIECE_OFFSET,0);
					if (TheBoard[x][y].Size==1)
						glScalef(0.56f,0.57f,0.56f);
					else if (TheBoard[x][y].Size==2)
						glScalef(0.78f,0.78f,0.78f);
					glBindTexture(GL_TEXTURE_2D,Texture[2]);
					//if this piece has been chosen, draw it in red
					bool DrawMe=(PieceX==x && PieceY==y);
					if (DrawMe)
						glColor3ub(255,0,0);
					else
						glColor3ub(TheBoard[x][y].Red,TheBoard[x][y].Green,TheBoard[x][y].Blue);
				
					if (DrawMe)
						glDisable(GL_TEXTURE_2D);
					
					DrawPiece();

					if (DrawMe)
						glEnable(GL_TEXTURE_2D);

					glPopMatrix();
				}
			}		
			glPopMatrix();
		}
	}
}

void ShowError()
{
	MessageBox(NULL,ErrorMsg,"Error",MB_OK);
}
