using System;
using System.Collections.Generic;
using System.ComponentModel;
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
using NetProfiler.EventViewModels;
using NetProfiler.FileMenu;
using NetProfiler.Logic;

namespace NetProfiler
{
    /// <summary>
    /// Interaktionslogik für MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, IMessageReceiver, INotifyPropertyChanged
    {
        Config config;
        public bool DebuggerRunning { get; set; }
        private DebugMode? debugMode { get; set; }
        private Process process;
        private CommunicationService communicationService;
        public BaseViewModel CurrentView { get; set; }
        private SearchLogic searchLogic = new SearchLogic();

        public string SearchText { get; set; } = string.Empty;

        private Dictionary<EventType, BaseViewModel> eventToViewModel = new Dictionary<EventType, BaseViewModel>
        {
            {EventType.Info, new ProfilerInfoViewModel()},
            {EventType.Exception, new ExceptionViewModel()},
            {EventType.FunctionEnter, new FunctionEnterViewModel()},
            {EventType.ObjectAllocated, new ObjectAllocatedViewModel() }
        };

        protected void RaisePropertyChanged(string propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
                handler(this, new PropertyChangedEventArgs(propertyName));
        }

        public event PropertyChangedEventHandler PropertyChanged;

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

        private ProfilerConfig GetProfilerConfig()
        {
            ProfilerOptions opts = ProfilerOptions.Default;
            if (config.StackCriticalLevel)
            {
                opts |= ProfilerOptions.StackCriticalLevel;
            }

            if (config.NotifyExceptions)
            {
                opts |= ProfilerOptions.Exceptions;
            }

            if (config.NotifyFunctionEnter)
            {
                opts |= ProfilerOptions.FunctionEnter;
            }

            if (config.NotifyFunctionLeave)
            {
                opts |= ProfilerOptions.FunctionLeave;
            }

            if (config.NotifyObjectAllocated)
            {
                opts |= ProfilerOptions.ObjectAllocated;
            }

            if (config.LogInfo)
            {
                opts |= ProfilerOptions.Info;
            }

            return new ProfilerConfig
            {
                ManagedThreadId = 0,
                StackCriticalLevelThreshold = uint.Parse(config.CriticalStackDepth ?? "0"),
                ProfilerOptions = opts
            };
        }

        private void StartNewProcess_OnClick(object sender, RoutedEventArgs e)
        {
            if (new StartNewProcess(config).ShowDialog() == true)
            {

                communicationService = new CommunicationService(this, GetProfilerConfig());
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
            line.Split('#').Select(x => x.Trim())
                .Where(x => x != string.Empty).Select(x =>
                {
                    var type = (EventType)x[0];
                    return new { type, msg = x.Substring(1)};
                }).GroupBy(x => x.type)
                .ToList().ForEach(x =>
                {
                    if (eventToViewModel.TryGetValue(x.Key, out var viewModel))
                    {
                        viewModel.Content += "\r\n" + string.Join("\r\n", x.Select(y => y.msg));
                    }
                });
        }

        private void EventCategories_OnSelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            TreeViewItem item = e.NewValue as TreeViewItem;
            if (item == null)
            {
                return;
            }

            var eventType = GetEventType(item.Name);

            if (eventType == null || !eventToViewModel.TryGetValue(eventType.Value, out var viewModel))
            {
                MessageBox.Show($"treeview item {item.Name} not mapped to viewmodel");
                return;
            }

            CurrentView = viewModel;
            RaisePropertyChanged("CurrentView");
        }

        private EventType? GetEventType(string treeViewItemName)
        {
            switch (treeViewItemName)
            {
                case "FunctionEnterCategory":
                    return EventType.FunctionEnter;
                case "ExceptionsCategory":
                    return EventType.Exception;
                case "ProfilerInfoLogging":
                    return EventType.Info;
                case "ObjectAllocationsCategory":
                    return EventType.ObjectAllocated;
                default:
                    return null;

            }
        }

        private void SearchInEvents_OnClick(object sender, RoutedEventArgs e)
        {
            var searchFor = SearchText;
            var searchIn = CurrentView.Content;
            var position = searchLogic.FindNextPosition(searchFor, searchIn, CurrentView.GetType());
            if (position == -1)
            {
                MessageBox.Show($"Could not find '{searchFor}'");
                return;
            }
            var linePosition = searchLogic.GetLineOf(searchIn, position);
            Enumerable.Range(0, linePosition - CurrentView.LastScrolledLine).ToList().ForEach(x => ScrollViewer.LineDown());
            
            CurrentView.LastScrolledLine = linePosition;
        }
    }
}
