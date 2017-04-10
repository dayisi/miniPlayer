


//
// Module Name: Server.c
//
// Description:
// This example illustrates a simple TCP server that accepts
// incoming client connections. Once a client connection is
// established, a thread is spawned to read data from the
// client and echo it back (if the echo option is not
// disabled).
//
// Compile:
// cl -o Server Server.c ws2_32.lib
//
// Command line options:
// server [-p:x] [-i:IP] [-o]
// -p:x Port number to listen on
// -i:str Interface to listen on
// -o Receive only, don't echo the data back
//
#include <winsock2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PORT 5150
#define DEFAULT_BUFFER 4096
#pragma comment(lib, "ws2_32.lib")
#define MAX_THREADS 4

int iPort = DEFAULT_PORT; // Port to listen for clients on
BOOL bInterface = FALSE, // Listen on the specified interface
bRecvOnly = FALSE; // Receive data only; don't echo back
char szAddress[128]; // Interface to listen for clients on
char* filename;      //file want to retrieve from clients
int NumOfCli;
BOOL recvFile[MAX_THREADS];
int threadIds[MAX_THREADS];
char *fileBuffer[MAX_THREADS];
//
// Function: usage
//
// Description:
// Print usage information and exit
//
void usage()
{
	printf("usage: server [-p:x] [-i:IP] [-o]\n\n");
	printf(" -p:x Port number to listen on\n");
	printf(" -i:str Interface to listen on\n");
	printf(" -o Don't echo the data back\n\n");
	ExitProcess(1);
}

typedef struct Mydata{
	long start;
	long end;
	SOCKET sock;
}MYDATA, *PMYDATA;
//
// Function: ValidateArgs
//
// Description:
// Parse the command line arguments, and set some global flags
// to indicate what actions to perform
//

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
	SOCKET sock = (SOCKET)lpParam;

	char szBuff[DEFAULT_BUFFER];
	int ret,
		nLeft,
		idx;
	    printf("send file name is %s\n", filename);
		ret = send(sock, filename, strlen(filename), 0);
		if (ret == SOCKET_ERROR)
		{
			printf("Send file name %s failed!\n", filename);
			exit(0);
		}
		printf("Has sent file name.\n");
//		ret = recv(sock, szBuff, DEFAULT_BUFFER, 0);
		memset(szBuff, '\0', DEFAULT_BUFFER);
		printf("has sent file name and waiting for get file size...\n");
		while ((ret = recv(sock, szBuff, DEFAULT_BUFFER, 0)) == 0);

		printf("Has received file size\n");
		long filesize=0;
		int i;
		for ( i = 0; i <strlen(szBuff); i++){
			filesize *= 10;
			filesize =filesize + szBuff[i]-'0';
		}
		printf("received file size is %s and filesize is %d\n", szBuff, filesize);
		printf("NumOfCli is %d\n", NumOfCli);

		long start, end;
		for (i = 0; i < MAX_THREADS; i++){
			printf("threadIds[%d] is %d\n", i, threadIds[i]);
			if (threadIds[i] == GetCurrentThreadId())
			{
				printf("Current thread id is %d\n", GetCurrentThreadId());
				printf("i is %d\n",i);
				start = filesize / MAX_THREADS*i;
				if (i == MAX_THREADS - 1){
					end = filesize;
				}
				else{
					end = filesize / MAX_THREADS*(i + 1);
				}
				fileBuffer[i] = (char*)malloc(sizeof(char)*(end - start + 1));
				break;
			}
		}
		sprintf(szBuff, "%d", start);
		printf("start is %s\n", szBuff);
		ret = send(sock, szBuff, strlen(szBuff), 0);
		printf("Has sent start\n");
		sprintf(szBuff, "%d", end);
		printf("end is %s\n", szBuff);
		ret = send(sock, szBuff, strlen(szBuff), 0);
		printf("Has sent end\n");
		printf("Waiting for file content...\n");
//		ret = recv(sock, fileBuffer[i], sizeof(fileBuffer[i]), 0);
		memset(fileBuffer[i], '\0', end-start+1);

		while ((ret = recv(sock, fileBuffer[i], end-start, 0)) == 0);

		printf("Received file content successfully!\n");
		printf("content is :\n%s\n", fileBuffer[i]);
		printf("Received content is:\n%s\n", fileBuffer[i]);
		printf("Receive file content successfully!\n");

	

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
int main(int argc, char **argv)
{
	WSADATA wsd;
	SOCKET sListen,
		sClient;
	int iAddrSize;
	HANDLE hThread;
	DWORD dwThreadId;
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
	// Select the local interface and bind to it
	//
	if (bInterface)
	{
		local.sin_addr.s_addr = inet_addr(szAddress);
		if (local.sin_addr.s_addr == INADDR_NONE)
			usage();
	}
	else
		local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_family = AF_INET;
	local.sin_port = htons(iPort);

	if (bind(sListen, (struct sockaddr *)&local,
		sizeof(local)) == SOCKET_ERROR)
	{
		printf("bind() failed: %d\n", WSAGetLastError());
		return 1;
	}

	printf("Input the filename you want to retrieve:"); 
	filename = (char*)malloc(sizeof(char)* 1024);
	scanf("%s", filename);
	listen(sListen, 8);
	//
	// In a continous loop, wait for incoming clients. Once one
	// is detected, create a thread and pass the handle off to it.
	//
	NumOfCli = 0;
	SOCKET sClients[MAX_THREADS];
	HANDLE hThreadArray[MAX_THREADS];
	for (int j = 0; j < MAX_THREADS; j++)
	{
		iAddrSize = sizeof(client);
//		sClient = accept(sListen, (struct sockaddr *)&client, &iAddrSize);
//		if (sClient == INVALID_SOCKET)
//		{
//			printf("accept() failed: %d\n", WSAGetLastError());
//			break;
//		}
		printf("in loop\n");
		printf("curNumOfCli is %d\n", NumOfCli);
		while ((sClients[j] = accept(sListen, (struct sockaddr *)&client, &iAddrSize)) == INVALID_SOCKET) printf("Error client\n");
//		if ((sClient = accept(sListen, (struct sockaddr *)&client, &iAddrSize)) != INVALID_SOCKET){
			printf("Accepted client: %s:%d\n",
				inet_ntoa(client.sin_addr), ntohs(client.sin_port));
			hThreadArray[j] = CreateThread(NULL, 0, ClientThread,
				(LPVOID)sClients[j], 0, &dwThreadId);
			if (hThreadArray[j] == NULL)
			{
				printf("CreateThread() failed: %d\n", GetLastError());
				break;
			}
			NumOfCli += 1;
			printf("curNumOfCli is %d and ThreadId is %d\n", NumOfCli, dwThreadId);
			threadIds[NumOfCli - 1] = dwThreadId;
		
	}
	WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);

	FILE* aimfile;
	aimfile = fopen(filename, "wb");
	if (aimfile == NULL)
	{
		printf("Open aimfile %s failed!\n", filename);
	}
	for (int i = 0; i < MAX_THREADS; i++)
	{
		fwrite(fileBuffer[i], 1, strlen(fileBuffer[i]), aimfile);
		free(fileBuffer[i]);
	}
	for (int i = 0; i < MAX_THREADS; i++){
		closesocket(sClients[i]);
		CloseHandle(hThreadArray[i]);
	}
	closesocket(sListen);
	fclose(aimfile);

	WSACleanup();
	return 0;
}