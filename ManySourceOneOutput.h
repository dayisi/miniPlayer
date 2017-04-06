#include <windows.h>
#include <tchar.h>
#include <strsafe.h>


#define BUF_SIZE 255
#define long int LONG

DWORD WINAPI MyThreadFunction(LPVOID lpParam);
void ErrorHandler(LPTSTR lpszFunction);
//char** source_filenames;
//char*  aim_filename;
int MAX_THREADS;
int GetSource(int sourceFileNum, char** sourceFileNames, char* aimFileName);

// Sample custom data structure for threads to use.
// This is passed by void pointer so it can be any data type
// that can be passed using a single void pointer (LPVOID).
typedef struct MyData {
	LONG start;
	LONG end;
	char* buffer;
	char* source_filename;
	//	char* aim_filename;
} MYDATA, *PMYDATA;

// thread number is the same to sourceFileNum. That is, if there is 3 source files,
// there will be 3 threads
// if there is only one files, there will be one threads.
int GetSource(int sourceFileNum, char** sourceFileNames, char* aimFileName)
{
	if (sourceFileNum < 2){
		printf("No source file or aim file!\n");
		return -1;
	}
	MAX_THREADS = sourceFileNum;
	PMYDATA* pDataArray = (PMYDATA*)malloc(sizeof(PMYDATA)*MAX_THREADS);
	DWORD*   dwThreadIdArray = (DWORD*)malloc(sizeof(DWORD)*MAX_THREADS);
	HANDLE*  hThreadArray = (HANDLE*)malloc(sizeof(HANDLE)*MAX_THREADS);
	if (pDataArray == NULL || dwThreadIdArray == NULL || hThreadArray == NULL){
		printf("No enough memeory!\n");
		return -1;
	}
	// get source_filenames and aim_filename

	/*	source_filenames = (char**)malloc(sizeof(char*)*argc - 2);
	for (int i = 1; i < argc - 1; i++)
	{
	source_filenames[i - 1] = argv[i];
	}
	aim_filename = argv[argc - 1];
	*/

	//find size of source file
	FILE* source_file;
	source_file = fopen(sourceFileNames[0], "rb");
	if (source_file == NULL){
		printf("Open %s failed!\n", sourceFileNames[0]);
		return -1;
	}
	fseek(source_file, 0L, SEEK_END);
	LONG fileSize = ftell(source_file);
	fseek(source_file, 0L, SEEK_SET);
	fclose(source_file);

	// Create MAX_THREADS worker threads.

	for (int i = 0; i<MAX_THREADS; i++)
	{
		// Allocate memory for thread data.

		pDataArray[i] = (PMYDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
			sizeof(MYDATA));

		if (pDataArray[i] == NULL)
		{
			// If the array allocation fails, the system is out of memory
			// so there is no point in trying to print an error message.
			// Just terminate execution.
			ExitProcess(2);
		}

		// Generate unique data for each thread to work with.

		pDataArray[i]->start = fileSize / MAX_THREADS*i;
		if (i == MAX_THREADS - 1)
			pDataArray[i]->end = fileSize;
		else
			pDataArray[i]->end = fileSize / MAX_THREADS*(i + 1);
		pDataArray[i]->buffer = (char*)malloc(sizeof(char)*(pDataArray[i]->end - pDataArray[i]->start));
		if (pDataArray[i]->buffer == NULL){
			printf("No enough memeory!\n");
			return -1;
		}
		pDataArray[i]->source_filename = sourceFileNames[i];
		// Create the thread to begin execution on its own.

		hThreadArray[i] = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			MyThreadFunction,       // thread function name
			pDataArray[i],          // argument to thread function 
			0,                      // use default creation flags 
			&dwThreadIdArray[i]);   // returns the thread identifier 


		// Check the return value for success.
		// If CreateThread fails, terminate execution. 
		// This will automatically clean up threads and memory. 

		if (hThreadArray[i] == NULL)
		{
			ErrorHandler(TEXT("CreateThread"));
			ExitProcess(3);
		}
	} // End of main thread creation loop.

	// Wait until all threads have terminated.

	WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);

	//print buffers into aim_file
	FILE* aim_file;
	aim_file = fopen(aimFileName, "wb");
	for (int i = 0; i < MAX_THREADS; i++){
		fwrite(pDataArray[i]->buffer, 1, pDataArray[i]->end - pDataArray[i]->start, aim_file);
	}
	fclose(aim_file);

	// Close all thread handles and free memory allocations.

	for (int i = 0; i<MAX_THREADS; i++)
	{
		CloseHandle(hThreadArray[i]);
		if (pDataArray[i] != NULL)
		{
			HeapFree(GetProcessHeap(), 0, pDataArray[i]);
			pDataArray[i] = NULL;    // Ensure address is not reused.
		}
	}

	return 0;
}


DWORD WINAPI MyThreadFunction(LPVOID lpParam)
{
	HANDLE hStdout;
	PMYDATA pDataArray;
	size_t result;   // result of fread
	TCHAR msgBuf[BUF_SIZE];
	//	size_t cchStringSize;
	//	DWORD dwChars;

	// Make sure there is a console to receive output results. 

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdout == INVALID_HANDLE_VALUE)
		return 1;

	// Cast the parameter to the correct data type.
	// The pointer is known to be valid because 
	// it was checked for NULL before the thread was created.

	pDataArray = (PMYDATA)lpParam;
	FILE* source_file;
	source_file = fopen(pDataArray->source_filename, "rb");
	if (source_file == NULL)
	{
		printf("Open %s failed!\n", pDataArray->source_filename);
		exit(1);
	}
	fseek(source_file, pDataArray->start, SEEK_SET);

	result = fread(pDataArray->buffer, 1, pDataArray->end - pDataArray->start, source_file);
	if (result != pDataArray->end - pDataArray->start){
		printf("Reading file error!\n");
		exit(2);
	}
	fclose(source_file);


	// Print the parameter values using thread-safe functions.

	//	StringCchPrintf(msgBuf, BUF_SIZE, TEXT("Parameters = %d, %d\n"),
	//		pDataArray->val1, pDataArray->val2);
	//	StringCchLength(msgBuf, BUF_SIZE, &cchStringSize);
	//	WriteConsole(hStdout, msgBuf, (DWORD)cchStringSize, &dwChars, NULL);

	return 1;
}



void ErrorHandler(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code.

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message.

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	// Free error-handling buffer allocations.

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}
