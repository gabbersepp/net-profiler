using System.IO;
using System.Text;
using NetProfiler.Contracts;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace NetProfiler.Logic
{
    public class ConfigurationManager
    {
        public static void WriteConfig(Config config)
        {
            var json = JsonSerializer.Serialize(config);
            File.WriteAllText("./config.json", json, Encoding.UTF8);
        }

        public static Config ReadConfig()
        {
            if (File.Exists("./config.json"))
            {
                return JsonSerializer.Deserialize<Config>(File.ReadAllText("./config.json"));
            }

            return null;
        }
    }
}