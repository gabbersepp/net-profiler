namespace NetProfiler.Contracts
{
    public enum EventType
    {
        Exception = 1,
        FunctionEnter = 2,
        FunctionLeave = 3,
        ObjectAllocated = 4,
        CriticalStack = 5,
        Info = 6
    };
}