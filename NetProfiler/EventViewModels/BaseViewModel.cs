using System.ComponentModel;

namespace NetProfiler.EventViewModels
{
    public abstract class BaseViewModel : INotifyPropertyChanged
    {
        protected void RaisePropertyChanged(string propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
                handler(this, new PropertyChangedEventArgs(propertyName));
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public string Content { get; set; } = string.Empty;
        public int LastScrolledLine { get; set; }
    }
}
