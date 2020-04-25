#pragma once
enum ProfilerOptions {
	ProfilerOptions_Exceptions = 1,
	ProfilerOptions_FunctionEnter = 1 << 1,
	ProfilerOptions_FunctionLeave = 1 << 2,
	ProfilerOptions_StackCriticalLevel = 1 << 3
};

class ProfilerConfig {
public:
	unsigned int StackCriticalLevelThreshold;
	unsigned int ManagedThreadId;
	ProfilerOptions ProfilerOptions;
};

