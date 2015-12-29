// MChessServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MChessServer.h"
#include "MChessServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/********************************
 * A.J.'s Global Variables, etc *
 ********************************/

#define UPDATE_OUTPUT WM_USER+1

struct ServerData
{
	CString Name;
	int Port;
};

void ServerThread(LPVOID param);

HWND theDlg;
HANDLE hMainThread;
struct sockaddr_in serverName, clientName[4];
SOCKET listenSkt, clientSkt[4];
char *PlayerNames[4], TempBuffer[256], Code[5], Response[256];

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CMChessServerDlg dialog



CMChessServerDlg::CMChessServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMChessServerDlg::IDD, pParent)
	, m_ServerName(_T("MartianChessServer"))
	, m_Port(1500)
	, m_IPAddr(_T("000.000.000.000"))
	, m_Output(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMChessServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SERVERNAME, m_ServerName);
	DDX_Text(pDX, IDC_PORT, m_Port);
	DDV_MinMaxInt(pDX, m_Port, 1, 9999);
	DDX_Text(pDX, IDC_IPADDR, m_IPAddr);
	DDX_Text(pDX, IDC_OUTPUT, m_Output);
}

BEGIN_MESSAGE_MAP(CMChessServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BEGIN, OnBnClickedBegin)
	ON_BN_CLICKED(IDC_END, OnBnClickedEnd)
	ON_MESSAGE(UPDATE_OUTPUT, UpdateOutput)
END_MESSAGE_MAP()


// CMChessServerDlg message handlers

BOOL CMChessServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	theDlg = GetSafeHwnd();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMChessServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMChessServerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMChessServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMChessServerDlg::OnBnClickedBegin()
{
	UpdateData(true);

	ServerData data;
	data.Name = m_ServerName;
	data.Port = m_Port;

	DWORD ThreadID;
	hMainThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ServerThread,&data,0,&ThreadID);
	WaitForSingleObject(hMainThread,1);

}

void CMChessServerDlg::OnBnClickedEnd()
{
	// TODO: Add your control notification handler code here
}

LRESULT CMChessServerDlg::UpdateOutput(WPARAM wParam, LPARAM lParam)
{
	UpdateData(true);

	m_Output += (char*)lParam;
	m_Output += "\r\n";
	free((char*)lParam);

	UpdateData(false);

	// Scroll the Edit Window so that the latest printed text is visible
	CEdit *output = (CEdit*)GetDlgItem(IDC_OUTPUT);
	output->LineScroll(output->GetLineCount());

	return LRESULT();
}

#define OUTMSG(a) { char *msg = _strdup(a); PostMessage(theDlg,UPDATE_OUTPUT,0,(LPARAM)msg); }
#define READSOCK(a,b,c) { int B = recv(a,b,c,0); b[B] = 0; }

void ServerStartup(ServerData data)
{
	OUTMSG("Server listening....");

	int Bytes = 0,temp = 0;
	WORD version;
	WSADATA wsaData;
	
	version = MAKEWORD(2,0);
	WSAStartup(version,&wsaData);
	
	listenSkt = socket(AF_INET,SOCK_STREAM,0);
	if (listenSkt == INVALID_SOCKET)
	{
		OUTMSG("Invalid socket");
		WSACleanup();
		return;
	}

	ZeroMemory(&serverName,sizeof(SOCKADDR_IN));
	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(data.Port);
	serverName.sin_addr.s_addr = htonl(INADDR_ANY);

	temp = bind(listenSkt,(sockaddr*)&serverName,sizeof(serverName));
	if (temp != 0)
	{
		OUTMSG("Socket Binding Error");
		WSACleanup();
		return;
	}

	temp = listen(listenSkt,4);
	if (temp != 0)
	{
		OUTMSG("Socket listening error");
		WSACleanup();
		return;
	}
}

int GameStartup()
{
	int NumPlayers = 0, PlayerNum = 0, NumBytes = 0;

	for (int x = 0; x < 4; x++)
		PlayerNames[x] = "NoName";

	// Get the first player - determines the number of other players
	int ClientSize = sizeof(clientName[0]);
	clientSkt[0] = accept(listenSkt,(sockaddr*)&clientName[0],&ClientSize);
	if (clientSkt[0] == INVALID_SOCKET)
	{
		OUTMSG("Socket accepting error");
		WSACleanup();
		return 0;
	}
	OUTMSG("Client Accepted");
	int Bytes = 0;
	READSOCK(clientSkt[0],TempBuffer,10);
	sscanf(TempBuffer,"GAME:%i:%i:",&PlayerNum,&NumBytes);
	READSOCK(clientSkt[0],TempBuffer,NumBytes);
	sscanf(TempBuffer,"%i",&NumPlayers);
	if (NumPlayers == 2)
		OUTMSG("Two player game requested. Waiting for other player.")
	else
		OUTMSG("Four player game requested. Waiting for other players.")
	sprintf(Response,"GAME:0:%i",NumPlayers); // Return the player number, and number of players
	send(clientSkt[0],Response,(int)strlen(Response),0);

	// Get the other player(s)
	for (int playnum = 1; playnum < NumPlayers; playnum++)
	{
		clientSkt[playnum] = accept(listenSkt,(sockaddr*)&clientName[playnum],&ClientSize);
		recv(clientSkt[playnum],TempBuffer,8,0);
		sscanf(TempBuffer,"GAME:%i:%i:",&PlayerNum,&NumBytes);
		recv(clientSkt[playnum],TempBuffer,256,0); // Clear out the socket buffer - this value is ignored
		sprintf(Response,"GAME:%i:%i",playnum,NumPlayers);
		send(clientSkt[playnum],Response,(int)strlen(Response),0);
		OUTMSG("New player Accepted");
	}

	// Get the players' names
	for (int playnum = 0; playnum < NumPlayers; playnum++) 
	{
		READSOCK(clientSkt[playnum],TempBuffer,10);
		sscanf(TempBuffer,"NAME:%i:%i:",&PlayerNum,&NumBytes);
		PlayerNames[playnum] = new char[NumBytes + 1];
		READSOCK(clientSkt[playnum],PlayerNames[playnum],NumBytes);
		OUTMSG("Added: ");
		OUTMSG(PlayerNames[playnum]);
	}
	
	// Send the players' names, the game can then begin
	// This needs to be record the length of each name, so that it can be read correctly
	int NameLens[4] = {0,0,0,0};
	for (int x = 0; x < 4; x++)
		NameLens[x] = (int)strlen(PlayerNames[x]);
	sprintf(Response,":%02i:%s:%02i:%s:%02i:%s:%02i:%s",
		NameLens[0],PlayerNames[0],
		NameLens[1],PlayerNames[1],
		NameLens[2],PlayerNames[2],
		NameLens[3],PlayerNames[3]);
	for (int playnum = 0; playnum < NumPlayers; playnum++)
	{
		send(clientSkt[playnum],Response,(int)strlen(Response),0);
	}

	return NumPlayers;
}

void MainGameLoop(int NumPlayers)
{
	int WhoseTurn = 0, PlayerNum = 0;
	bool GameOver = false;

	while (!GameOver)
	{
		int Bytes = recv(clientSkt[WhoseTurn],TempBuffer,18,0);
		if (Bytes > 0) // Good data was read
		{
			TempBuffer[Bytes] = 0;
			char *move = TempBuffer;
			strncpy(Code,move,4); Code[4] = 0; move+=5;
			sscanf(move,"%i:",&PlayerNum); move+=5;
			int NumBytes = 8; // For some reason I can't parse this value correctly....
			OUTMSG(PlayerNames[PlayerNum]);
			OUTMSG(Code);
			OUTMSG(move);
			if (strcmp(Code,"MOVE") == 0)
			{
				WhoseTurn = (WhoseTurn + 1) % NumPlayers;
				for (int playnum = 0; playnum < NumPlayers; playnum++)
				{
					sprintf(Response,"%s:%02i:%s",Code,NumBytes,move);
					//MessageBox(NULL,Response,"ServerSending",MB_OK);
					send(clientSkt[playnum],Response,(int)strlen(Response),0);
				}
			}
			else if (strcmp(Code,"OVER") == 0)
			{
				WhoseTurn = 0;
				GameOver = true;
			}
		}
		else
		{
			GameOver = true;
		}
	}
}

void ServerShutdown(int NumPlayers)
{
	for (int skt = 0; skt < NumPlayers; skt++)
	{
		closesocket(clientSkt[skt]);
		//delete PlayerNames[x];
	}

	WSACleanup();
}

void ServerThread(LPVOID param)
{
	ServerData data = *((ServerData*)param);

	ServerStartup(data);
	int NumPlayers = GameStartup();
	if (NumPlayers == 0)
		return;
	MainGameLoop(NumPlayers);
	ServerShutdown(NumPlayers);
	MessageBox(NULL,"Shut down nicely","MChessServer",MB_OK);
}
