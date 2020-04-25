#pragma once

class EventLogger {
public:
	long Initialize();
	void Finalize(); // replacer with destructor
	long ProcessEvent(char* input, unsigned long inputSize, char* output, unsigned long outputSize, unsigned long * read);

private:
	HANDLE fileHandle;

};
