namespace NetProfiler.Contracts
{
    public class ProfilerConfig
    {
        public uint StackCriticalLevelThreshold { get; set; }
        public ProfilerOptions ProfilerOptions { get; set; }
        // todo: should be recognized in profiler
        public uint ManagedThreadId { get; set; }

    }
}