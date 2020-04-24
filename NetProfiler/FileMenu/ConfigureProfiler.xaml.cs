using System;
using System.Collections.Generic;
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
using System.Windows.Shapes;
using Microsoft.Win32;
using NetProfiler.Contracts;

namespace NetProfiler.FileMenu
{
    /// <summary>
    /// Interaktionslogik für ConfigureProfiler.xaml
    /// </summary>
    public partial class ConfigureProfiler : Window
    {
        public Config Config { get; set; }

        public ConfigureProfiler(Config config)
        {
            Config = config;
            DataContext = config;
            InitializeComponent();
        }

        private void SelectProfilerDllButton_OnClick(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            if (openFileDialog.ShowDialog() == true)
            {
                FilePath.Content = openFileDialog.FileName;
            }
        }

        private void Ok_OnClick(object sender, RoutedEventArgs e)
        {
            Config.ProfilerDll = FilePath.Content.ToString();
            DialogResult = true;
            Close();
        }
    }
}
