using System;
using System.Net.Http;
using System.Threading.Tasks;
using System.Net.Http.Headers;
using System.IO;

namespace DiscordScriptError
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Posting error...");

            var url = @"https://discord.com/api/webhooks/848207736189878272/rDQKlyg24r0-fCqlhiNbsPY5Z_0NwHLZht02HVAt283BIEAv7QJSY3L61aCQYOrs2oaD";
            var error = args.Length > 0 ? args[0] : "No error given.";

            SendPostRequest(url, error).Wait();

            Console.WriteLine("All done.");
        }

        static async Task SendPostRequest(string url, string error)
        {
            try
            {
                HttpClient client = new HttpClient();
                var requestContent = new StringContent("{\"content\": \"Error: `" + error + "`\"}");
                requestContent.Headers.ContentType = new MediaTypeHeaderValue("application/json");
                var response = await client.PostAsync(url, requestContent);
                var responseString = await response.Content.ReadAsStringAsync();
                Console.WriteLine("Got response: " + responseString);
            }
            catch (Exception e)
            {
                Console.WriteLine("Error: " + e.GetType() + ": " + e.Message + "\n" + e);
            }
        }
    }
}
