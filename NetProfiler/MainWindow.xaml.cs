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
    public partial class MainWindow : Window
    {
        Config config;
        public bool DebuggerRunning { get; set; }
        public DebugMode DebugMode { get; set; }
        private Process process;

        public MainWindow()
        {
            config = ConfigurationManager.ReadConfig() ?? new Config();
            DataContext = this;
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
                DebugMode = DebugMode.Attach;
            }
        }

        private void StartNewProcess_OnClick(object sender, RoutedEventArgs e)
        {
            if (new StartNewProcess(config).ShowDialog() == true)
            {
                ConfigurationManager.WriteConfig(config);
                StopDebugging.IsEnabled = true;
                AttachToProcess.IsEnabled = false;
                StartNewProcess.IsEnabled = false;
                DebugMode = DebugMode.NewProcess;
                process = ProcessHelper.StartNewProcess(config.ProfilerId, config.ProfilerDll, config.FilePath);
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
        }
    }
}
