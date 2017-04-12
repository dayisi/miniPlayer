


//
// Module Name: Server.c


#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "TcpServer.hpp"



int BLOCK_SIZE = 1000;
int REC_BUFFER_SIZE = 500;

int iPort = DEFAULT_PORT; // Port to listen for clients on
BOOL bInterface = FALSE, // Listen on the specified interface
bRecvOnly = FALSE; // Receive data only; don't echo back
char szAddress[128]; // Interface to listen for clients on
char* filename;      //file want to retrieve from clients
BOOL recvFile[MAX_THREADS];
int threadIds[MAX_THREADS];
char *fileBuffer[MAX_THREADS];
BOOL Stored[] = {FALSE,FALSE,FALSE,FALSE,FALSE};
int MB = 1024 * 1024;
int numOfClient;
typedef struct Mydata{
	long start;
	long end;
	miniPlayer::mini myPlayer;
	int flag;
	SOCKET sClient;
	int tid;
	char** fileBuffers;
}MYDATA, *PMYDATA;

MMCKINFO RI;
MMCKINFO FI;
MMCKINFO DI;
WAVEFORMATEX WF;
long filesize = 0;
char* dataBuff = (char*)malloc(sizeof(char)*(MB * 200));

char** buffers=(char**)malloc(sizeof(char*)*(MAX_THREADS));

//
// Function: ClientThread
//
// Description:
// This function is called as a thread, and it handles a given
// client connection. The parameter passed in is the socket
// handle returned from an accept() call. This function reads
// data from the client and writes it back.
//
DWORD WINAPI ClientThread(LPVOID lpParam)
{
	HANDLE hStout;
	PMYDATA pDataArray;
	pDataArray = (PMYDATA)lpParam;
	numOfClient++;

	hStout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStout == INVALID_HANDLE_VALUE)
		return 1;
		char szBuff[DEFAULT_BUFFER];
		int ret;

		//send file name
		printf("send file name is %s\n", filename);
		ret = send(pDataArray->sClient, filename, strlen(filename), 0);
		if (ret == SOCKET_ERROR)
		{
			printf("Send file name %s failed!\n", filename);
			exit(0);
		}
		printf("Has sent file name.\n");

		
		//Receive data format
		// receive RIFFInfo
		
		unsigned int bytes_recvd = recv(pDataArray->sClient, szBuff, sizeof(MMCKINFO), 0);
		printf("Waiting RIFF\n");
		memcpy(&RI, szBuff, sizeof(MMCKINFO));
		printf("Received RIFFInfo: %d bytes\n", bytes_recvd);

		// receive FMTChunkInfo
		memset(szBuff, 0, sizeof(MMCKINFO));
		bytes_recvd = recv(pDataArray->sClient, szBuff, sizeof(MMCKINFO), 0);
		memcpy(&FI, szBuff, sizeof(MMCKINFO));
		printf("Received FMTChunkInfo: %d bytes\n", bytes_recvd);

		// receive DATAChunkInfo
		memset(szBuff, 0, sizeof(MMCKINFO));
		bytes_recvd = recv(pDataArray->sClient, szBuff, sizeof(MMCKINFO), 0);
		memcpy(&DI, szBuff, sizeof(MMCKINFO));
		printf("Received DATAChunkInfo: %d bytes\n", bytes_recvd);

		// receive waveFMT
		memset(szBuff, 0, sizeof(WAVEFORMATEX));
		bytes_recvd = recv(pDataArray->sClient, szBuff, sizeof(WAVEFORMATEX), 0);
		memcpy(&WF, szBuff, sizeof(WAVEFORMATEX));
		printf("Received waveFMT: %d bytes\n", bytes_recvd);

		// create myPlayer using RI, FI, DI, WF
		miniPlayer::mini myPlayer(RI, FI, DI, WF);
		myPlayer.data = new char[myPlayer.dataSize];
        
		filesize = myPlayer.dataSize;
		//wait for data
		pDataArray->start = filesize/MAX_THREADS*pDataArray->tid;
		if (pDataArray->tid == MAX_THREADS - 1)
			pDataArray->end = filesize;
		else
			pDataArray->end = filesize/MAX_THREADS*(pDataArray->tid+1);
		printf("Receiving data:\n");
		memset(szBuff, '\0', DEFAULT_BUFFER);
		buffers[pDataArray->tid] = (char*)malloc(sizeof(char)*(pDataArray->end-pDataArray->start));
		unsigned long index;

		for (index = 0; index < pDataArray->end - pDataArray->start; index += REC_BUFFER_SIZE){
			bytes_recvd = recv(pDataArray->sClient, (char*)szBuff, sizeof(char)*REC_BUFFER_SIZE, 0);
			for (int a = 0; a < REC_BUFFER_SIZE; a++)
			{
			
				buffers[pDataArray->tid][a + index] = szBuff[a];
				if (a + index >= pDataArray->end - pDataArray->start - 1)
				{
					break;
				}
			}
		}
		printf("Receiving data end!\n");
		printf("going to store data!\n");

		return 1;
}

//
// Function: main
//
// Description:
// Main thread of execution. Initialize Winsock, parse the
// command line arguments, create the listening socket, bind
// to the local address, and wait for client connections.
//
int TcpServer(char* input)
{
	numOfClient = 0;
	WSADATA wsd;
	SOCKET sListen;
	int iAddrSize;
	memset(dataBuff, '\0', 70 * MB);
	struct sockaddr_in local,
		client;

	if (WSAStartup(MAKEWORD(2, 2),  &wsd) != 0)
	{
		printf("Failed to load Winsock!\n");
		return 1;
	}
	// Create our listening socket
	//
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (sListen == SOCKET_ERROR)
	{
		printf("socket() failed: %d\n", WSAGetLastError());
		return 1;
	}

	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_family = AF_INET;
	local.sin_port = htons(iPort);

	if (bind(sListen, (struct sockaddr *)&local,
		sizeof(local)) == SOCKET_ERROR)
	{
		printf("bind() failed: %d\n", WSAGetLastError());
		return 1;
	}

	filename = (char*)malloc(sizeof(char)* 100);
	filename = input;
//	scanf("%s", filename);
	listen(sListen, 8);
	//
	// In a continous loop, wait for incoming clients. Once one
	// is detected, create a thread and pass the handle off to it.
	//

	SOCKET sClients[MAX_THREADS];

	PMYDATA* pDataArray = (PMYDATA*)malloc(sizeof(PMYDATA)*MAX_THREADS);
	DWORD* dwThreadArray = (DWORD*)malloc(sizeof(DWORD)*MAX_THREADS);
	HANDLE *hThreadArray = (HANDLE*)malloc(sizeof(HANDLE)*MAX_THREADS);
//	HANDLE hThread;
//	DWORD dwThreadId;
	for (int j = 0; j < MAX_THREADS; j++)
	{
		pDataArray[j] = (PMYDATA)malloc(sizeof(MYDATA));
		iAddrSize = sizeof(client);
		printf("Wait for Clients to connect...\n");
		pDataArray[j]->tid = j;
		while ((pDataArray[j]->sClient = accept(sListen, (struct sockaddr *)&client, &iAddrSize)) == INVALID_SOCKET) printf("Error client\n");
		hThreadArray[j] = CreateThread(NULL, 0, ClientThread,
			(LPVOID)pDataArray[j], 0, &dwThreadArray[j]);
		if (hThreadArray[j] == NULL)
		{
			printf("CreateThread() failed: %d\n", GetLastError());
			break;
		}
	}
	WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);

/*	
*/	
	Sleep(200);
	printf("In the save data thread\n");
	miniPlayer::mini myPlayer(RI, FI, DI, WF);
	if (filesize == 0)
	{
		printf("Wrong file size!\n");
	}
//	printf("Received filesize is %d and dataSize is %d", filesize, myPlayer.dataSize);
	myPlayer.data = (char*)malloc(sizeof(char)*(filesize + 1));
	memset(myPlayer.data, '\0', filesize + 1);

	int i = 0;
	for (int j = 0; j < MAX_THREADS; j++)
	{
		for (int k = 0; k < pDataArray[j]->end - pDataArray[j]->start; k++)
		{
			myPlayer.data[i] = buffers[j][k];
			i++;
		}
	}

	myPlayer.miniPlay();
	printf("Please choose one action:\n1. Continue.\n2. Restart.\n3. Stop\n");
	int choice;
	std::cin >> choice;
	if (choice == 0 || choice == 1){
		myPlayer.miniPlay();
	}
	else if (choice == 2){
		myPlayer.miniRestart();
	}
	else{
		Sleep(200);
		int choice;
		std::cout << "1. plause" << std::endl;
		std::cout << "2. stop" << std::endl;
		std::cin >> choice;
		if (choice == 1) {
			myPlayer.miniPlause();
		}
		else {
			myPlayer.miniStop();
		}

	}
	FILE* aimfile;

	WSACleanup();
	return 0;
}
/*
int main(void){
	printf("Input the filename you want to retrieve:");
	char *input = (char*)malloc(sizeof(char)* 100);
	std::cin >> input;
	int c = TcpServer(input);
	if (c == 0)
		printf("success!\n");
	return 0;
}
*/