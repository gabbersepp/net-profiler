#include <WinBase.h>;
#include "stdafx.h"
#include "EventLogger.h";
#include <iostream>;
#include "ProfilerConfig.h";
#include <cstring>;

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

ProfilerConfig* EventLogger::Initialize(unsigned long * error) {
	fileHandle = CreateFileW(TEXT("\\\\.\\pipe\\netprofiler"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (GetLastError() != 0) {
		*error = GetLastError();
		return nullptr;
	}

	char* buffer = new char[1000];
	memset(buffer, 0, 1000);
	unsigned long read = 0;
	ReadFile(fileHandle, buffer, 1000, &read, NULL);

	std::cout << "\r\nconfig\r\n\t" << buffer;

	ProfilerConfig* result = ParseConfig(&buffer);
	//delete[] buffer;
	return result;
}

unsigned int ReadInt(char** line) {
	char* occurence = strchr(*line, ';');

	if (occurence == nullptr) {
		return -1;
	}

	char* inputBuffer = new char[30];
	memset(inputBuffer, 0, 30);
	//std::cout << (unsigned long)occurence << "----" << (unsigned long)*line;
	//return 0;
	strncpy(inputBuffer, *line, occurence - *line);
	unsigned int value = std::atoi(inputBuffer);
	delete[] inputBuffer;
	*line = occurence + 1;
	return value;
}

ProfilerConfig* EventLogger::ParseConfig(char** line) {
	ProfilerConfig* config = new ProfilerConfig();
	char* copy = *line;

	config->ProfilerOptions = static_cast<ProfilerOptions>(ReadInt(line));
	config->ManagedThreadId = ReadInt(line);
	config->StackCriticalLevelThreshold = ReadInt(line);

	*line = copy;
	return config;
}

