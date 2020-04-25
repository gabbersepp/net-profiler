using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using NetProfiler.Contracts;
using NetProfiler.FileMenu;
using NetProfiler.Logic;

namespace NetProfiler
{
    /// <summary>
    /// Interaktionslogik für MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, IMessageReceiver
    {
        Config config;
        public bool DebuggerRunning { get; set; }
        private DebugMode? debugMode { get; set; }
        private Process process;
        private CommunicationService communicationService;

        public MainWindow()
        {
            config = ConfigurationManager.ReadConfig() ?? new Config();
            DataContext = this;
            this.Closed += (sender, args) => process?.Kill();
            InitializeComponent();
        }

        private void AttachToProcess_OnClick(object sender, RoutedEventArgs e)
        {
            if (new AttachToProcess(config).ShowDialog() == true)
            {
                ConfigurationManager.WriteConfig(config);
                StopDebugging.IsEnabled = true;
                AttachToProcess.IsEnabled = false;
                StartNewProcess.IsEnabled = false;
                debugMode = DebugMode.Attach;
            }
        }

        private void StartNewProcess_OnClick(object sender, RoutedEventArgs e)
        {
            if (new StartNewProcess(config).ShowDialog() == true)
            {
                communicationService = new CommunicationService(this, new ProfilerConfig{ManagedThreadId = 123, ProfilerOptions = ProfilerOptions.Exceptions | ProfilerOptions.FunctionEnter, StackCriticalLevelThreshold = 11});
                ConfigurationManager.WriteConfig(config);
                StopDebugging.IsEnabled = true;
                AttachToProcess.IsEnabled = false;
                StartNewProcess.IsEnabled = false;
                debugMode = DebugMode.NewProcess;
                process = ProcessHelper.StartNewProcess(config.ProfilerId, config.ProfilerDll, config.FilePath);
                process.EnableRaisingEvents = true;
                process.Exited += (o, args) => { Dispatcher.Invoke(() =>
                {
                    StopDebugging.IsEnabled = false;
                    AttachToProcess.IsEnabled = true;
                    StartNewProcess.IsEnabled = true;
                    debugMode = null;
                }); };
            }
        }

        private void ConfigureProfilerMenuItem_OnClick(object sender, RoutedEventArgs e)
        {
            if (new ConfigureProfiler(config).ShowDialog() == true)
            {
                ConfigurationManager.WriteConfig(config);
            }
        }

        private void StopDebugging_OnClick(object sender, RoutedEventArgs e)
        {
            StopDebugging.IsEnabled = false;
            AttachToProcess.IsEnabled = true;
            StartNewProcess.IsEnabled = true;
            communicationService.Dispose();
        }

        public void Error(Exception e, string additionalInfos = null)
        {

        }

        public void Receive(string line)
        {
            Test.Text = Test.Text + "\r\n" + line;
        }
    }
}
