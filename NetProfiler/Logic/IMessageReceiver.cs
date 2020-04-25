using System;

namespace NetProfiler.Logic
{
    public interface IMessageReceiver
    {
        void Receive(string line);
        void Error(Exception e, string additionalInfo = null);
    }
}