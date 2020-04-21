using System;

namespace ErrorThrowingApp
{
    class Program
    {
        static void Main(string[] args)
        {
            
            while (true)
            {
                Console.WriteLine("\r\n------------------\r\nFrom App: Press any key or 'a' to exit");
                if (Console.ReadKey().KeyChar == 'a')
                {
                    break;
                }
                Console.Write("\r\n");
                new Test().TestFn();

                try
                {
                    throw new Exception();
                }
                catch { }
                Console.ReadKey();
            }
        }
    }

    class Test
    {
        public void TestFn()
        {
            var t = new Test();
            t.TestFn2();
        }

        public void TestFn2()
        {
            Console.WriteLine("TestFn2");
        }
    }
}
