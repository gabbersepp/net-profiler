using System;

namespace NetProfiler.Contracts
{
    [Flags]
    public enum ProfilerOptions
    {
        Default = 0,
        Exceptions = 1,
        FunctionEnter = 1 << 1,
        FunctionLeave = 1 << 2,
        StackCriticalLevel = 1 << 3,
        ObjectAllocated = 1 << 4,
        Info = 1 << 5
    }
}