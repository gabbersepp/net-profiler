using System;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;
using NetProfiler.Contracts;

namespace NetProfiler.Logic
{
    public class CommunicationService : IDisposable
    {
        private readonly IMessageReceiver receiver;
        private NamedPipeServerStream namedPipeServer;
        private Task readTask;
        private CancellationTokenSource cancellationTokenSource = new CancellationTokenSource();
        private StreamReader streamReader;
        private StreamWriter streamWriter;

        public CommunicationService(IMessageReceiver receiver, ProfilerConfig profilerConfig)
        {
            this.receiver = receiver;
            namedPipeServer = new NamedPipeServerStream("netprofiler", PipeDirection.InOut, 20, PipeTransmissionMode.Message);
            streamReader = new StreamReader(namedPipeServer);
            //streamWriter = new StreamWriter(namedPipeServer);
            readTask = Task.Run(() => TaskAction(profilerConfig), cancellationTokenSource.Token);
        }

        private void WriteConfig(ProfilerConfig profilerConfig)
        {
            var sw = new StreamWriter(namedPipeServer);
            var str = $"{(int)profilerConfig.ProfilerOptions};{profilerConfig.ManagedThreadId};{profilerConfig.StackCriticalLevelThreshold}";
            str = str + ";";
            // the other side expects exactly 1000 characters :-) 
            sw.Write(str + Enumerable.Range(0, 1000 - str.Length).Select(x => '\0').Aggregate(string.Empty, (x, y) => y + x));
            sw.Flush();

            // if the length here does not fit the length at the other side, this line is waiting forever!
            namedPipeServer.WaitForPipeDrain();
        }

        private void TaskAction(ProfilerConfig profilerConfig)
        {
            try
            {
                namedPipeServer.WaitForConnection();
                WriteConfig(profilerConfig);

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

        public void Dispose()
        {
            cancellationTokenSource.Cancel();
            namedPipeServer.Close();
        }
    }
}