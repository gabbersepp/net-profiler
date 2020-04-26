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

        public CommunicationService(IMessageReceiver receiver, ProfilerConfig profilerConfig)
        {
            this.receiver = receiver;
            namedPipeServer = new NamedPipeServerStream("netprofiler", PipeDirection.InOut, 20, PipeTransmissionMode.Message);
            streamReader = new StreamReader(namedPipeServer);
            readTask = Task.Run(() => TaskAction(profilerConfig), cancellationTokenSource.Token);
        }

        private void WriteConfig(ProfilerConfig profilerConfig)
        {
            var sw = new StreamWriter(namedPipeServer);
            var str = $"{(int)profilerConfig.ProfilerOptions};{profilerConfig.ManagedThreadId};{profilerConfig.StackCriticalLevelThreshold};";
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
                string line;

                while (!cancellationTokenSource.IsCancellationRequested && (line = streamReader.ReadLine()) != null)
                {
                    var lineCopy = line;
                    Application.Current.Dispatcher.BeginInvoke(new Action(() => receiver.Receive(lineCopy)));
                }
            }
            catch (Exception e)
            {
                Application.Current.Dispatcher.BeginInvoke(new Action(() => receiver.Error(e)));
            }
        }

        public void Dispose()
        {
            cancellationTokenSource.Cancel();
            namedPipeServer.Close();
        }
    }
}