#pragma once
#include "ProfilerConfig.h";

class EventLogger {
public:
	ProfilerConfig* Initialize(unsigned long* error);
	void Finalize(); // replacer with destructor
	long ProcessEvent(char* input, unsigned long inputSize, char* output, unsigned long outputSize, unsigned long * read);

private:
	HANDLE fileHandle;
	ProfilerConfig* ParseConfig(char** line);
};
