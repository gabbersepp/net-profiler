using System.Windows;
using NetProfiler.Contracts;

namespace NetProfiler.FileMenu
{
    /// <summary>
    /// Interaktionslogik für AttachToProcess.xaml
    /// </summary>
    public partial class AttachToProcess : Window
    {
        private readonly Config config;

        public AttachToProcess(Config config)
        {
            this.config = config;
            InitializeComponent();
        }

        private void Ok_OnClick(object sender, RoutedEventArgs e)
        {
            if (!int.TryParse(ProcessId.Text, out int result))
            {
                MessageBox.Show($"The input '{ProcessId.Text}' can not be parsed to an integer");
                return;
            }

            DialogResult = true;
            config.ProcessId = result;
            Close();
        }
    }
}
