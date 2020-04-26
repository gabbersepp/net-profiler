#include <WinBase.h>;
#include "stdafx.h"
#include "EventLogger.h";
#include <iostream>;
#include "ProfilerConfig.h";
#include <cstring>;

const char* newLine = "\r\n";

char* EventLogger::GetFormattedLine(EventType eventType, char* input, unsigned long size) {
	char* newInput = new char[size + 3];
	newInput[0] = (short)eventType; // must be interpreted as integer!
	strcpy(newInput + 1, input);
	newInput[size + 1] = '#';
	newInput[size + 2] = 0;
	return newInput;
}

long EventLogger::Flush() {
	unsigned long read = 0;

	for (int i = 0; i < eventBufferIndex; i++) {
		char* pData = eventBuffer[i];
		BOOL result = WriteFile(fileHandle, pData, strlen(pData), &read, NULL);
		if (result == FALSE) {
			return HRESULT_FROM_WIN32(GetLastError());
		}
		delete[] pData;
	}
	
	WriteFile(fileHandle, newLine, 2, &read, NULL);

	eventBufferIndex = 0;
	return TRUE;
}

void EventLogger::ProcessEvent(EventType eventType, char * input, unsigned long inputSize) {
	if (eventType == EventType::Info && !logInfo) {
		return;
	}

	char* newInput = GetFormattedLine(eventType, input, inputSize);
	std::cout << newInput << "\r\n";
	eventBuffer[eventBufferIndex++] = newInput;

	if (eventBufferIndex == maxEventBufferIndex) {
		Flush();
	}
}

void EventLogger::Finalize() {
	Flush();
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

	ProfilerConfig* result = ParseConfig(&buffer);
	logInfo = result->ProfilerOptions & ProfilerOptions_Info;
	delete[] buffer;
	return result;
}

unsigned int ReadInt(char** line) {
	char* occurence = strchr(*line, ';');

	if (occurence == nullptr) {
		return -1;
	}

	char* inputBuffer = new char[30];
	memset(inputBuffer, 0, 30);

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

