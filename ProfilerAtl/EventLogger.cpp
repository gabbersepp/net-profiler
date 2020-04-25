#include <WinBase.h>;
#include "stdafx.h"
#include "EventLogger.h";
#include <iostream>;

long EventLogger::ProcessEvent(char * input, unsigned long inputSize, char * output, unsigned long outputSize, unsigned long * read) {
	long size = strlen(input);
	char* newInput = new char[size + 3];
	strcpy(newInput, input);
	newInput[size] = '\r';
	newInput[size + 1] = '\n';
	newInput[size + 2] = 0;
	std::cout << "\r\nmsg:\r\n\t" << newInput;
	/*BOOL result = CallNamedPipeA("\\\\.\\pipe\\netprofiler", newInput, inputSize + 2, output, outputSize, read, 5000);
	if (result == FALSE) {
		return HRESULT_FROM_WIN32(GetLastError());
	}*/

	BOOL result = WriteFile(fileHandle, newInput, size + 2, read, NULL);

	if (result == FALSE) {
		return HRESULT_FROM_WIN32(GetLastError());
	}

	delete[] newInput;

	return TRUE;
}

void EventLogger::Finalize() {
	CloseHandle(fileHandle);
}

long EventLogger::Initialize() {
	fileHandle = CreateFileW(TEXT("\\\\.\\pipe\\netprofiler"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (GetLastError() != 0) {
		return GetLastError();
	}

	char* buffer = new char[1000];
	memset(buffer, 0, 1000);
	unsigned long read = 0;
	ReadFile(fileHandle, buffer, 1000, &read, NULL);
	std::cout << "\r\nconfig\r\n\t" << buffer;

	return TRUE;
}