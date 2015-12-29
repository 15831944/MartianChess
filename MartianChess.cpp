// MartianChessWin32.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MartianChess.h"
#include "MCDraw.h"
#include "MCTextures.h"
#include "MCRules.h"
#include "MCFonts.h"
#include "MCThread.h"
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

/***************************
 * A.J.'s Global Variables *
 ***************************/

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

HWND theDlg;
static HGLRC hRC = NULL;
static HDC hdc;
MCFont *theFont = NULL;
float PrevXRot, PrevYRot;
int PrevX, PrevY;
int PortNum;
char Server[256];

extern int NumPlayers;
extern char PlayerNames[4][16];
extern int MaxNameLen;
extern bool NetGame;
extern float EyeRotate;
extern char TitleInfo[256];

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	GameSetup(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK  Rules(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MARTIANCHESS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_MARTIANCHESS);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_CHESS);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; //(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_MARTIANCHESS;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_CHESS);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

	NumPlayers = 4;
	NetGame = false;
	strcpy(PlayerNames[0],"Dave");
	strcpy(PlayerNames[1],"Peter");
	strcpy(PlayerNames[2],"Paul");
	strcpy(PlayerNames[3],"Bert");

	if (DialogBox(hInstance,(LPCTSTR)IDD_GAMESETUP,NULL,(DLGPROC)GameSetup) == IDOK)
		hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			CW_USEDEFAULT, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
	else
		hWnd = NULL;

   if (!hWnd)
   {
      return FALSE;
   }

	theDlg = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	HWND LoadDlg;

	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_FILE_NEW:
			if (DialogBox(hInst,(LPCTSTR)IDD_GAMESETUP, hWnd, (DLGPROC)GameSetup) == IDOK)
			{
				EyeRotate = 0;
				SetupGame(0);
				SendMessage(hWnd,WM_PAINT,NULL,NULL);
			}
			break;
		case ID_HELP_RULES:
			DialogBox(hInst, (LPCTSTR)IDD_RULES, hWnd, (DLGPROC)Rules);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			if (MessageBox(hWnd,"Are you sure you want to quit?","Quit game",MB_OKCANCEL) == IDOK)
				DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CREATE:
		LoadDlg = CreateDialog(hInst,(LPCSTR)IDD_LOADING,hWnd,(DLGPROC)NULL);
		hdc = GetDC(hWnd);
		SetDCPixelFormat(hdc);
		hRC = wglCreateContext(hdc);
		wglMakeCurrent(hdc, hRC);
		theFont = MakeMCFont(hdc,"Courier",11,2,0);
		InitGL();
		if (NetGame)
		{
			// Connect to the server - get other players' names, etc
			sprintf(TitleInfo,"Martian Chess - Waiting for Players"); // Temporary title bar
			for (int x = 1; x <= 3; x++) 
				sprintf(PlayerNames[x],"<waiting>"); // Temporary player names, until the real ones are sent
			ThreadData data;
			data.port = PortNum;
			data.servername = new char[(int)strlen(Server)];
			strcpy(data.servername,Server);
			LaunchClientThread(data);
		}
		else
		{
			SetupGame(0); // Pass the player number to this function
		}
		DestroyWindow(LoadDlg);
		break;
	case WM_SIZE:
		ChangeSize(LOWORD(lParam),HIWORD(lParam)); // LO = Width, HI = Height
		break;
	case WM_PAINT:
		SetWindowText(hWnd,TitleInfo); // Updates too often, but it works....
		RenderScene();
		SwapBuffers(hdc);
		ValidateRect(hWnd,NULL);
		break;
	case WM_LBUTTONUP:
		PointSelected(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		SendMessage(hWnd,WM_PAINT,NULL,NULL);
		break;
	case WM_RBUTTONDOWN:
		PrevX = LOWORD(lParam);
		PrevY = HIWORD(lParam);
		PrevXRot = EyeRotate;
		break;
	case WM_RBUTTONUP:
		PrevX = -1;
		break;
	case WM_MOUSEMOVE:
	{
		if (!(wParam & MK_RBUTTON))
			break;

		int diff = LOWORD(lParam) - PrevX;

		if (diff != 0)
		{
			RECT rect;
			GetWindowRect(hWnd,&rect);
			EyeRotate = diff / ((float)rect.right - rect.left) * 360 + PrevXRot;
		}
		SendMessage(hWnd,WM_PAINT,NULL,NULL);
		break;
	}
	case WM_DESTROY:
		DestroyMCFont(theFont);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

#define LENCHECK(a) temp = (a); if (temp > MaxNameLen) MaxNameLen = temp;

LRESULT CALLBACK GameSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		if (NumPlayers == 4)
			CheckRadioButton(hDlg,IDC_FOURPLAYERS,IDC_TWOPLAYERS,IDC_FOURPLAYERS);
		else
			CheckRadioButton(hDlg,IDC_FOURPLAYERS,IDC_TWOPLAYERS,IDC_TWOPLAYERS);
		CheckDlgButton(hDlg,IDC_NETWORK,NetGame);
		EnableWindow(GetDlgItem(hDlg,IDC_PORT),NetGame);
		SetDlgItemText(hDlg,IDC_PORT,"1500");
		EnableWindow(GetDlgItem(hDlg,IDC_SERVERNAME),NetGame);
		SetDlgItemText(hDlg,IDC_SERVERNAME,"localhost");
		EnableWindow(GetDlgItem(hDlg,IDC_NAME2),!NetGame);
		EnableWindow(GetDlgItem(hDlg,IDC_NAME3),NumPlayers == 4 && !NetGame);
		EnableWindow(GetDlgItem(hDlg,IDC_NAME4),NumPlayers == 4 && !NetGame);
		SetDlgItemText(hDlg,IDC_NAME1,PlayerNames[0]);
		SetDlgItemText(hDlg,IDC_NAME2,PlayerNames[1]);
		SetDlgItemText(hDlg,IDC_NAME3,PlayerNames[2]);
		SetDlgItemText(hDlg,IDC_NAME4,PlayerNames[3]);
		return true;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			MaxNameLen = 0;
			int temp = 0;
			LENCHECK(GetDlgItemText(hDlg,IDC_NAME1,PlayerNames[0],16));
			LENCHECK(GetDlgItemText(hDlg,IDC_NAME2,PlayerNames[1],16));
			LENCHECK(GetDlgItemText(hDlg,IDC_NAME3,PlayerNames[2],16));
			LENCHECK(GetDlgItemText(hDlg,IDC_NAME4,PlayerNames[3],16));
			if (IsDlgButtonChecked(hDlg,IDC_FOURPLAYERS) == BST_CHECKED)
				NumPlayers = 4;
			else
				NumPlayers = 2;
			NetGame = (IsDlgButtonChecked(hDlg,IDC_NETWORK) == BST_CHECKED);
			PortNum = GetDlgItemInt(hDlg,IDC_PORT,false,false);
			GetDlgItemText(hDlg,IDC_SERVERNAME,Server,256);
			EndDialog(hDlg,LOWORD(wParam));
			return true;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg,LOWORD(wParam));
			return false;
		}
		else if (LOWORD(wParam) == IDC_FOURPLAYERS)
		{
			UINT network = IsDlgButtonChecked(hDlg,IDC_NETWORK);
			EnableWindow(GetDlgItem(hDlg,IDC_NAME3),!network);
			EnableWindow(GetDlgItem(hDlg,IDC_NAME4),!network);
		}
		else if (LOWORD(wParam) == IDC_TWOPLAYERS)
		{
			EnableWindow(GetDlgItem(hDlg,IDC_NAME3),false);
			EnableWindow(GetDlgItem(hDlg,IDC_NAME4),false);
		}
		else if (LOWORD(wParam) == IDC_NETWORK)
		{
			UINT network = IsDlgButtonChecked(hDlg,IDC_NETWORK);
			EnableWindow(GetDlgItem(hDlg,IDC_PORT),network);
			EnableWindow(GetDlgItem(hDlg,IDC_SERVERNAME),network);
			EnableWindow(GetDlgItem(hDlg,IDC_NAME2),!network);
			EnableWindow(GetDlgItem(hDlg,IDC_NAME3),(!network && IsDlgButtonChecked(hDlg,IDC_FOURPLAYERS)));
			EnableWindow(GetDlgItem(hDlg,IDC_NAME4),(!network && IsDlgButtonChecked(hDlg,IDC_FOURPLAYERS)));
		}
		break;
	}
	return false;
}

LRESULT CALLBACK Rules(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg,LOWORD(wParam));
			return true;
		}
		break;
	}
	return false;
}