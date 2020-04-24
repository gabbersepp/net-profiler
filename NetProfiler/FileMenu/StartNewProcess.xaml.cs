using System.Windows;
using Microsoft.Win32;
using NetProfiler.Contracts;

namespace NetProfiler.FileMenu
{
    /// <summary>
    /// Interaktionslogik für StartNewProcess.xaml
    /// </summary>
    public partial class StartNewProcess : Window
    {
        private readonly Config config;

        public StartNewProcess(Config config)
        {
            this.config = config;
            DataContext = config;
            InitializeComponent();
        }

        private void SelectFile_OnClick(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            if (openFileDialog.ShowDialog() == true)
            {
                FilePath.Content = openFileDialog.FileName;
            }
        }

        private void Ok_OnClick(object sender, RoutedEventArgs e)
        {
            config.FilePath = FilePath.Content.ToString();
            DialogResult = true;
            Close();
        }
    }
}
