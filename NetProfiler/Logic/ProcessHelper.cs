using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;

namespace NetProfiler.Logic
{
    public class ProcessHelper
    {
        public static Process StartNewProcess(string profilerId, string profilerPath, string exePath)
        {
            ProcessStartInfo psi = new ProcessStartInfo(exePath);
            profilerId = Regex.Match(profilerId, @"^\{.*\}$").Success ? profilerId : ("{" + profilerId + "}");
            psi.EnvironmentVariables.Add("COR_ENABLE_PROFILING", "1");
            psi.EnvironmentVariables.Add("COR_PROFILER", profilerId);
            psi.EnvironmentVariables.Add("COR_PROFILER_PATH", profilerPath);
            psi.EnvironmentVariables.Add("COMPLUS_ProfAPI_ProfilerCompatibilitySetting", "EnableV2Profiler");

            psi.UseShellExecute = false;
            return Process.Start(psi);
        }
    }
}