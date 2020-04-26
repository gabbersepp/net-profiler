#pragma once
#include "ProfilerConfig.h";

enum class EventType {
	Exception = 1,
	FunctionEnter = 2,
	FunctionLeave = 3,
	ObjectAllocated = 4,
	CriticalStack = 5,
	Info = 6
};

#define maxEvSize 50

class EventLogger {
public:
	ProfilerConfig* Initialize(unsigned long* error);
	void Finalize(); // replacer with destructor
	void ProcessEvent(EventType eventType, char* input, unsigned long inputSize);
	long Flush();
	char* GetFormattedLine(EventType eventType, char* input, unsigned long inputSize);

private:
	HANDLE fileHandle;
	ProfilerConfig* ParseConfig(char** line);
	bool logInfo;
	unsigned const int maxEventBufferIndex = maxEvSize;
	char* eventBuffer[maxEvSize];
	// points to the next free place
	unsigned int eventBufferIndex = 0;
};
