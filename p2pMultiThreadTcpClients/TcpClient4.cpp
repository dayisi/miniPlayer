
#include "TcpClient.hpp"

int SEND_BUFFER_SIZE = 500;
//char szServer[128], // Server to connect to
char szMessage[1024]; // Message to send to sever
int iPort = DEFAULT_PORT; // Port on server to connect to
DWORD dwCount = DEFAULT_COUNT; // Number of times to send message
BOOL bSendOnly = FALSE; // Send data only; don't receive



//
// Function: main
//
// Description:
// Main thread of execution. Initialize Winsock, parse the
// command line arguments, create a socket, connect to the
// server, and then send and receive data.
//
int TcpClient(char* ServerIP)
{
	WSADATA wsd;
	SOCKET sClient;
	char szBuffer[DEFAULT_BUFFER];
	int ret;
	struct sockaddr_in server;
	struct hostent *host = NULL;


	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		printf("Failed to load Winsock library!\n");
		return 1;
	}

	//
	// Create the socket, and attempt to connect to the server
	//
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET)
	{
		printf("socket() failed: %d\n", WSAGetLastError());
		return 1;
	}
	//szServer = "192.168.110.44";
	server.sin_family = AF_INET;
	server.sin_port = htons(iPort);
	server.sin_addr.s_addr = inet_addr(ServerIP);

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

		//		FILE *sourcefile;
		char* filename;
		filename = (char*)malloc(sizeof(char)*(strlen(szBuffer) + 1));
		strcpy(filename, szBuffer);
		printf("filename is %s\n", filename);
		// Using filename to find file, getting PATH to file and returning PATH to filename;
		miniPlayer::mini myPlayer(filename);


		//Send Format
		//send RIFFInfo
		memset(szBuffer, '\0', DEFAULT_BUFFER);
		memcpy(szBuffer, &myPlayer.RIFFInfo, sizeof(MMCKINFO));
		send(sClient, szBuffer, sizeof(MMCKINFO), 0);

		//send FMTChunkInfo
		memset(szBuffer, '\0', sizeof(MMCKINFO));
		memcpy(szBuffer, &myPlayer.FMTChunkInfo, sizeof(MMCKINFO));
		send(sClient, szBuffer, sizeof(MMCKINFO), 0);

		//send DATAChunkInfo
		memset(szBuffer, '\0', sizeof(MMCKINFO));
		memcpy(szBuffer, &myPlayer.DATAChunkInfo, sizeof(MMCKINFO));
		send(sClient, szBuffer, sizeof(MMCKINFO), 0);

		//send waveFmt
		memset(szBuffer, '\0', sizeof(WAVEFORMATEX));
		memcpy(szBuffer, &myPlayer.waveFmt, sizeof(WAVEFORMATEX));
		send(sClient, szBuffer, sizeof(WAVEFORMATEX), 0);

		//send data 
		//send data size
		long filesize = myPlayer.dataSize;
		long start, end;
		start = myPlayer.dataSize / 4 * 3;
		end = myPlayer.dataSize;
		//send data
		printf("Sending data...\n");
		memset(szBuffer, '\0', DEFAULT_BUFFER);
		unsigned long index = 0;
		for (index = start; index < end; index += SEND_BUFFER_SIZE)
		{
			for (int i = 0; i < SEND_BUFFER_SIZE; i++){
				szBuffer[i] = myPlayer.data[i + index];
				if (i + index >= end - 1 || i + index >= myPlayer.dataSize - 1){
					break;
				}
			}
			send(sClient, (char*)szBuffer, sizeof(char)*SEND_BUFFER_SIZE, 0);
		}
		printf("Sending data Over!\n");
		break;
	}
	closesocket(sClient);

	WSACleanup();
	return 0;
}


int main(void){
	printf("Please input server IP:");
	char ip[20];
	scanf("%s", ip);
	int c;
	c = TcpClient(ip);
	if (c == 0)
		printf("Success!\n");
	return 0;
}