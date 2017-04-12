#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <strsafe.h>
#include <Windows.h>
#include <string.h>
#include "miniSocket.hpp"
#define DEFAULT_PORT 5150
#define DEFAULT_BUFFER 4096
#pragma comment(lib, "ws2_32.lib")
#define MAX_THREADS 4

DWORD WINAPI ClientThread(LPVOID lpParam);

int TcpServer(char* input);  //input is the file name user want to retrieve