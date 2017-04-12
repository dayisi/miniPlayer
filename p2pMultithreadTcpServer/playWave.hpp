#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <mmsystem.h>
#include <string>
#include <fstream>
#pragma comment(lib, "winmm.lib")

namespace miniPlayer
{
	class mini
	{
	public:
		// constructor
		mini(char* filename);
		mini(MMCKINFO &RI, MMCKINFO &FI, MMCKINFO &DI, WAVEFORMATEX &WF);

		// public function

		// for continue
		void miniPlay();
		// for stop
		void miniStop();
		// for restart
		void miniRestart();
		// plause
		void miniPlause();

		// public attributes
		char* data;
		unsigned int dataSize;

		// info
		MMCKINFO RIFFInfo;
		MMCKINFO FMTChunkInfo;
		WAVEFORMATEX waveFmt;
		MMCKINFO DATAChunkInfo;

		// socket
		void Server();
		void Client();


	private:
		// private attributes
		char* filename;

		unsigned int BUFFERSIZE;
		// unsigned int bufferCount;
		unsigned int state;
		unsigned int plause;

		// hmmio
		HMMIO hmmioIn;

		// device
		HWAVEOUT WaveHandle;

		// buffer
		WAVEHDR* waveBuffer;

		// initialize
		void initialize();
		void miniOpen();
		void readRIFF();
		void readFmt();
		void readData();

		// void setBuffer();

		// query a device
		void setDevice();

	};
}