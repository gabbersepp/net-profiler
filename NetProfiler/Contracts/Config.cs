using System.Text.Json.Serialization;

namespace NetProfiler.Contracts
{
    public class Config
    {
        public string FilePath { get; set; }
        [JsonIgnore]
        public int ProcessId { get; set; }
        public string ProfilerId { get; set; } = "9E2B38F2-7355-4C61-A54F-434B7AC266C0";
        public string ProfilerDll { get; set; } = "./ProfilerAtl.dll";

        public bool NotifyObjectAllocated { get; set; }
        public bool NotifyFunctionEnter { get; set; }
        public bool NotifyFunctionLeave { get; set; }
        public bool NotifyExceptions { get; set; }
        public bool StackCriticalLevel { get; set; }
        public string CriticalStackDepth { get; set; }
        public bool LogInfo { get; set; }
    }
}