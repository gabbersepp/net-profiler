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
                var chr = Console.ReadKey().KeyChar;
                if (chr == 'a')
                {
                    break;
                }

                if (chr == 'R')
                {
                    Console.WriteLine("recursive");
                    Recursive();
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

        static void Recursive()
        {
            Recursive();
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
