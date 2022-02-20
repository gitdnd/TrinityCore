using MonoTorrent;
using MonoTorrent.BEncoding;
using System;

namespace TorrentWrangler
{
    internal class Program
    {
        public static void Log(string message)
        {
            Console.WriteLine(message);
        }

        static void Main(string[] args)
        {
            Log("Starting up");
            var downloadFileName = "HourOfTwilight_Installer.exe";
            Log($"Creating download for file: {downloadFileName}");
            Log($"Creating torrent file: {downloadFileName}.torrent");
            try
            {
                var creator = new TorrentCreator
                {
                    Announce = "http://gw.elunatech.com:3725/announce",
                    PieceLength = 16777216,
                    CreatedBy = "Hour of Twilight"
                };
                creator.SetCustom(new BEncodedString("choose path"), new BEncodedNumber(0));
                creator.SetCustom(new BEncodedString("download type"), new BEncodedNumber(1));
                creator.SetCustom(new BEncodedString("launch target"), new BEncodedString(downloadFileName));
                creator.Create(new TorrentFileSource(downloadFileName), $"{downloadFileName}.torrent");
            }
            catch (Exception ex)
            {
                Log($"[ERROR]: {ex.GetType()}: {ex.Message}\n{ex.InnerException}\n{ex}");
            }
            Log("All done.");
        }
    }
}
