using System.IO;
using System.Text;
using NetProfiler.Contracts;

namespace NetProfiler.Logic
{
    public class ConfigurationManager
    {
        public static void WriteConfig(Config config)
        {
            var json = JsonHelper.Serialize(config);
            File.WriteAllText("./config.json", json, Encoding.UTF8);
        }

        public static Config ReadConfig()
        {
            if (File.Exists("./config.json"))
            {
                return JsonHelper.Deserialize<Config>(File.ReadAllText("./config.json"));
            }

            return null;
        }
    }
}