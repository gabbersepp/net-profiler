using System;
using System.IO;
using System.IO.Pipes;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;

namespace NetProfiler.Logic
{
    public class CommunicationService
    {
        private readonly IMessageReceiver receiver;
        private NamedPipeServerStream namedPipeServer;
        private Task readTask;
        private CancellationTokenSource cancellationTokenSource = new CancellationTokenSource();
        private StreamReader streamReader;
        private StreamWriter streamWriter;

        public CommunicationService(IMessageReceiver receiver)
        {
            this.receiver = receiver;
            namedPipeServer = new NamedPipeServerStream("netprofiler", PipeDirection.In, 20, PipeTransmissionMode.Message);
            streamReader = new StreamReader(namedPipeServer);
            //streamWriter = new StreamWriter(namedPipeServer);
            readTask = Task.Run(TaskAction, cancellationTokenSource.Token);
        }

        private void TaskAction()
        {
            try
            {
                namedPipeServer.WaitForConnection();

                // reradline == null -> error /disconnect
                while (!cancellationTokenSource.IsCancellationRequested)
                {
                    // one line is one object
                    var line = streamReader.ReadLine();
                    // thread war blockiert?? evtl zu schnelles senden
                    Application.Current.Dispatcher.BeginInvoke(new Action(() => receiver.Receive(line)));
                    //streamWriter.WriteLine("\r\n");
                    //streamWriter.Flush();
                }
            }
            catch (Exception e)
            {
                Application.Current.Dispatcher.BeginInvoke(new Action(() => receiver.Error(e)));
                //Dispatcher.CurrentDispatcher.Invoke(() => { receiver.Error(e); });
            }
        }

        ~CommunicationService()
        {
            cancellationTokenSource.Cancel();
            namedPipeServer.Close();
        }
    }
}