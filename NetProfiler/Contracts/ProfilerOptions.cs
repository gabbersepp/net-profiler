using System;

namespace NetProfiler.Contracts
{
    [Flags]
    public enum ProfilerOptions
    {
        Exceptions = 1,
        FunctionEnter = 1 << 1,
        FunctionLeave = 1 << 2,
        StackCriticalLevel = 1 << 3
    }
}