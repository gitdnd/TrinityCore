using MonoTorrent;
using MonoTorrent.Client;
using MonoTorrent.Tracker;
using System.IO;

namespace TorrentManager
{
    class Program
    {
        static void Main(string[] args)
        {
            new TrackerInstance();
        }

        /*
        // torrentDirectory is a folder containing .torrent files
        // which should be loaded into the engine
        public void StartTracker(string torrentDirectory)
        {
            // Create the tracker and register a listener as in Example 1
            Tracker tracker = new Tracker();
            HttpListener listener = new HttpListener("http://myserver.com/announce");
            tracker.RegisterListener(listener);
            listener.Start();

            // If an announce request is received for a torrent which is not registered with the
            //the tracker an error will be returned.
            tracker.AllowUnregisteredTorrents = false;

            // Load all torrents into the engine. Only announce requests for these torrents
            // (or torrents added in the future) will be processed.
            foreach (string file in Directory.GetFiles(torrentDirectory))
            {
                Torrent torrent = Torrent.Load(file);

                // InfoHashTrackable is a basic subclass of ITrackable. It only stores
                // the infohash and name of the torrent. If you need to store additional
                // data for each torrent you're tracking, just subclass ITrackable or
                // InfoHashTrackable and add the extra data there.
                InfoHashTrackable trackable = new InfoHashTrackable(torrent);
                tracker.Add(trackable);
            }
        }
        */
    }
}
