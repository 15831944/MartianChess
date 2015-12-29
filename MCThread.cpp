#include "stdafx.h"
#include "MCRules.h"
#include "MCDraw.h"
#include "MCThread.h"

extern int PlayerNum, WhoseTurn, NumPlayers;
extern char PlayerNames[4][16];
extern bool GameOver;
extern HWND theDlg;

SOCKET clientSkt;


void LaunchClientThread(ThreadData data)
{
	HANDLE ThreadHandle;
	DWORD ThreadID;

	ThreadHandle = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ClientThread,&data,0,&ThreadID);
	// I was having problems with the thread param being lost, so I added this artificial delay
	WaitForSingleObject(ThreadHandle,1000);
	CloseHandle(ThreadHandle);
}

void SendToServer(char *msg)
{
	//MessageBox(NULL,msg,"Sending",MB_OK);
	send(clientSkt,msg,(int)strlen(msg),0);
}

void ClientThread(LPVOID param)
{
	ThreadData data = *((ThreadData*)param);
	int port = data.port;
	char *hostname = data.servername;
	int Bytes = 0;
	struct sockaddr_in name;
	WORD version;
	WSADATA wsaData;
	char tempbuf[256];

	version = MAKEWORD(2,0);
	WSAStartup(version,&wsaData);

	ZeroMemory(&name,sizeof(SOCKADDR_IN));
	name.sin_family = AF_INET;
	name.sin_port = htons((u_short)port);
	struct hostent *host;
	host = gethostbyname(hostname);
	CopyMemory(&name.sin_addr,host->h_addr_list[0],host->h_length);

	clientSkt = socket(AF_INET,SOCK_STREAM,0);
	int res = connect(clientSkt,(sockaddr*)&name,sizeof(name));
	if (res == SOCKET_ERROR)
	{
		MessageBox(NULL,"Could not connect to server","Socket Error",MB_OK);
		WSACleanup();
		return;
	}

	sprintf(tempbuf,"GAME:0:01:%i",NumPlayers);
	SendToServer(tempbuf);
	Bytes = recv(clientSkt,tempbuf,8,0);
	tempbuf[Bytes] = 0;
	sscanf(tempbuf,"GAME:%i:%i",&PlayerNum,&NumPlayers); // Get the actual number of players

	// Tell the server this player's name
	int namelen = (int)strlen(PlayerNames[0]);
	sprintf(tempbuf,"NAME:%i:%02i:%s",PlayerNum,namelen,PlayerNames[0]);
	tempbuf[10 + namelen] = 0;
	SendToServer(tempbuf);

	// Get all of the players' names
	for (int x = 0; x < 4; x++)
	{
		Bytes = recv(clientSkt,tempbuf,4,0);
		tempbuf[Bytes] = 0;
		int len;
		sscanf(tempbuf,":%i:",&len);
		Bytes = recv(clientSkt,PlayerNames[x],len,0);
		PlayerNames[x][Bytes] = 0;
	}

	SetupGame(PlayerNum);
	SendMessage(theDlg,WM_PAINT,NULL,NULL);

	//Main game loop
	char Code[5] = "CODE";
	int NumBytes = 0;
	int x1, y1, x2, y2;
	while (!GameOver)
	{
		Bytes = recv(clientSkt,Code,4,0);
		Code[Bytes] = 0;
		Bytes = recv(clientSkt,tempbuf,4,0);
		tempbuf[Bytes] = 0;
		//sscanf(tempbuf,":%02i:",&NumBytes);
		NumBytes = 8; // I can't seem to parse this out of the packet correctly
		if (strcmp(Code,"MOVE") == 0) // A move has been made
		{
			Bytes = recv(clientSkt,tempbuf,NumBytes,0);
			tempbuf[Bytes] = 0;
			sscanf(tempbuf,"r%ic%ir%ic%i",&x1,&y1,&x2,&y2);
			MovePiece(x1,y1,x2,y2);
			SendMessage(theDlg,WM_PAINT,NULL,NULL);
		}
	}

	closesocket(clientSkt);
	WSACleanup();
}