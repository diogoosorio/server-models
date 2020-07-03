using System;
using System.Net.Http;
using System.Threading.Tasks;

namespace DotnetAsync
{
    class Program
    {
        static async Task Main(string[] args)
        {
            var httpClient = new HttpClient();

            await httpClient.GetStringAsync("https://marleyspoon.com");
            Console.WriteLine("Done!");
        }
    }
}
