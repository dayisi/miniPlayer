#include <winsock.h>
#include <fstream>
#include "playWave.hpp"

#include <tchar.h>
#include <strsafe.h>


namespace miniPlayer
{
	class miniServer
	{
	public:

		// constructor
		miniServer();

		// close
		void miniClose();

		// WSADATA
		WSADATA wsaData;

		// sock
		SOCKET listen_sock;
		SOCKET new_sock;

		// addr
		struct sockaddr_in my_addr;

		// build Server
		void miniSend(mini &player);

	};

	class miniClient
	{
	public:
		// constructor
		miniClient(char* address = "192.168.110.54");

		// close
		void miniClose();

		// WSADATA
		WSADATA wsaData;

		// sock
		SOCKET conn_sock;

		// buffer
		char* buffer;
		
		// addr
		struct sockaddr_in remote_addr;

		// build Client
		mini receiveFMT();

		void receiveData(mini &myPlayer);

		void receiveAndPlay(mini &myPlayer);

	};

}