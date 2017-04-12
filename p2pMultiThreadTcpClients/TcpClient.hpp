#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <strsafe.h>
#include <Windows.h>
#include "H:\3280\miniPlayer-master\lib\miniSocket.hpp"
#include <Ws2tcpip.h>


#define DEFAULT_COUNT 20
#define DEFAULT_PORT 5150
#define DEFAULT_BUFFER 2048
#pragma comment(lib, "ws2_32.lib")

int TcpClient(char* ServerIPAddr);