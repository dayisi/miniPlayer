
// Module Name: Client.c
//
// Description:
// This sample is the echo client. It connects to the TCP server,
// sends data, and reads data back from the server.
//
// Compile:
// cl -o Client Client.c ws2_32.lib
//
// Command Line Options:
// client [-p:x] [-s:IP] [-n:x] [-o]
// -p:x Remote port to send to
// -s:IP Server's IP address or hostname
// -n:x Number of times to send message
// -o Send messages only; don't receive
//
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <strsafe.h>
#include <Windows.h>

#define DEFAULT_COUNT 20
#define DEFAULT_PORT 5150
#define DEFAULT_BUFFER 2048
#define DEFAULT_MESSAGE "This is a test of the emergency \
broadcasting system"
#pragma comment(lib, "ws2_32.lib")

char szServer[128], // Server to connect to
szMessage[1024]; // Message to send to sever
int iPort = DEFAULT_PORT; // Port on server to connect to
DWORD dwCount = DEFAULT_COUNT; // Number of times to send message
BOOL bSendOnly = FALSE; // Send data only; don't receive

//
// Function: usage:
//
// Description:
// Print usage information and exit
//
void usage()
{
	printf("usage: client [-p:x] [-s:IP] [-n:x] [-o]\n\n");
	printf(" -p:x Remote port to send to\n");
	printf(" -s:IP Server's IP address or hostname\n");
	printf(" -n:x Number of times to send message\n");
	printf(" -o Send messages only; don't receive\n");
	ExitProcess(1);
}

//
// Function: main
//
// Description:
// Main thread of execution. Initialize Winsock, parse the
// command line arguments, create a socket, connect to the
// server, and then send and receive data.
//
int main(int argc, char **argv)
{
	WSADATA wsd;
	SOCKET sClient;
	char szBuffer[DEFAULT_BUFFER];
	int ret,
		i;
	struct sockaddr_in server;
	struct hostent *host = NULL;

	// Parse the command line and load Winsock
	//
//	ValidateArgs(argc, argv);
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		printf("Failed to load Winsock library!\n");
		return 1;
	}
	strcpy(szMessage, DEFAULT_MESSAGE);
	//
	// Create the socket, and attempt to connect to the server
	//
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET)
	{
		printf("socket() failed: %d\n", WSAGetLastError());
		return 1;
	}
	//szServer = "192.168.110.83";
	server.sin_family = AF_INET;
	server.sin_port = htons(iPort);
	server.sin_addr.s_addr = inet_addr("192.168.110.83");

	if (connect(sClient, (struct sockaddr *)&server,
		sizeof(server)) == SOCKET_ERROR)
	{
		printf("connect() failed: %d\n", WSAGetLastError());
		return 1;
	}
	printf("Connect successfully!\n");
	// Send and receive data

	while (1){
		printf("Wait for server sending file name...\n");
		//		ret = recv(sClient, szBuffer, DEFAULT_BUFFER, 0);
		memset(szBuffer, '\0', DEFAULT_BUFFER);
		while ((ret = recv(sClient, szBuffer, DEFAULT_BUFFER, 0)) == 0);
		printf("has received file name %s\n", szBuffer);

		FILE *sourcefile;
		char* filename;
		filename = (char*)malloc(sizeof(char)*(strlen(szBuffer) + 1));
		strcpy(filename, szBuffer);
		printf("filename is %s\n", filename);
		sourcefile = fopen(filename, "rb");
		if (sourcefile == NULL){
			printf("No source file %s!\n", szBuffer);
		}
		fseek(sourcefile, 0L, SEEK_END);
		long filesize = ftell(sourcefile);
		fseek(sourcefile, 0L, SEEK_SET);
//		fclose(sourcefile);
		sprintf(szMessage, "%d", filesize);
		printf("Before send file size, which is %d\n",filesize);
		ret = send(sClient, szMessage, strlen(szMessage), 0);
		printf("Has sent file size %s\n", szMessage);
		printf("Waiting for receive start...\n");
		memset(szBuffer, '\0', DEFAULT_BUFFER);
		while ((ret = recv(sClient, szBuffer, DEFAULT_BUFFER, 0)) == 0);
		long start, end;
		start = 0;
		end = 0;
		printf("Received start is %s\n");
		for (int i = 0; i < strlen(szBuffer); i++)
		{
			start *= 10;
			start =start+ szBuffer[i]-'0';
		}
		printf("start is %d\n", start);
		printf("Waiting for receive end...\n");
		memset(szBuffer, '\0', DEFAULT_BUFFER);
		while ((ret = recv(sClient, szBuffer, DEFAULT_BUFFER, 0)) == 0);
		for (int i = 0; i < strlen(szBuffer); i++)
		{
			end *= 10;
			end =end+ szBuffer[i]-'0';
		}
		printf("end is %d\n", end);
		char* fileBuffer;
		fileBuffer = (char*)malloc(sizeof(char)*(end - start + 1));
		memset(fileBuffer, '\0', end - start + 1);
//		sourcefile = fopen(filename, "rb");
		if (sourcefile == NULL){
			printf("Open %s failed!\n", filename);
			exit(1);
		}
		fseek(sourcefile, start, SEEK_SET);
		int result;
		result = fread(fileBuffer, 1, end - start, sourcefile);
		if (result != end - start){
			printf("Reading %s failed!\n", filename);
			exit(2);
		}
		fclose(sourcefile);
		ret = send(sClient, fileBuffer, strlen(fileBuffer), 0);
		if (ret == SOCKET_ERROR){
			printf("Send file content failed!\n");
			exit(3);
		}
		printf("Content is %s\n", fileBuffer);
		printf("Sending file content successfully!\n");
		free(filename);
		free(fileBuffer);
		break;

	}
	closesocket(sClient);

	WSACleanup();
	return 0;
}