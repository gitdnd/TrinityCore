using MonoTorrent;
using MonoTorrent.Client;
using MonoTorrent.TorrentWatcher;
using MonoTorrent.Tracker;
using MonoTorrent.Tracker.Listeners;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace TorrentManager
{
    class TrackerInstance
    {
        readonly TrackerServer tracker;
        TorrentFolderWatcher watcher;
        const string TORRENT_DIR = "Torrents";

        ///<summary>Start the Tracker. Start Watching the TORRENT_DIR Directory for new Torrents.</summary>
        public TrackerInstance()
        {
            tracker = new TrackerServer
            {
                AllowUnregisteredTorrents = false
            };

            Console.WriteLine("Creating HTTP endpoint");

            var announceUrl = "http://*:3725/announce/";
            var listeners = new[] {
                TrackerListenerFactory.CreateHttp(announceUrl),
                TrackerListenerFactory.CreateUdp(3725)
            };

            /*var httpEndpoint = new System.Net.IPEndPoint(System.Net.IPAddress.Any, 3725);
            var udpEndpoint = new System.Net.IPEndPoint(System.Net.IPAddress.Any, 3726);
            Console.WriteLine("Listening for HTTP requests at: {0}", httpEndpoint);
            Console.WriteLine("Listening for UDP requests at: {0}", udpEndpoint);

            var listeners = new[] {
                TrackerListenerFactory.CreateHttp(httpEndpoint),
                TrackerListenerFactory.CreateUdp(udpEndpoint)
            };*/

            foreach (var listener in listeners)
            {
                tracker.RegisterListener(listener);
                listener.Start();
            }

            SetupTorrentWatcher();

            while (true)
            {
                System.Threading.Thread.Sleep(10000);
            }
        }

        private void SetupTorrentWatcher()
        {
            watcher = new TorrentFolderWatcher(Path.GetFullPath(TORRENT_DIR), "*.torrent");

            watcher.TorrentFound += delegate (object sender, TorrentWatcherEventArgs e) {
                try
                {
                    // This is a hack to work around the issue where a file triggers the event
                    // before it has finished copying. As the filesystem still has an exclusive lock
                    // on the file, monotorrent can't access the file and throws an exception.
                    // The best way to handle this depends on the actual application. 
                    // Generally the solution is: Wait a few hundred milliseconds
                    // then try load the file.
                    System.Threading.Thread.Sleep(500);

                    Torrent torrent = Torrent.Load(e.TorrentPath);

                    // There is also a predefined 'InfoHashTrackable' MonoTorrent.Tracker which
                    // just stores the infohash and name of the torrent. This is all that the tracker
                    // needs to run. So if you want an ITrackable that "just works", then use InfoHashTrackable.

                    // ITrackable trackable = new InfoHashTrackable(t);
                    ITrackable trackable = new InfoHashTrackable(torrent);

                    Console.WriteLine("Loaded torrent: " + e.TorrentPath);

                    // The lock is here because the TorrentFound event is asyncronous and I have
                    // to ensure that only 1 thread access the tracker at the same time.
                    lock (tracker)
                        tracker.Add(trackable);

                    /*var engine = new ClientEngine();
                    var manager = new MonoTorrent.Client.TorrentManager(torrent, "");
                    engine.Register(manager);
                    manager.StartAsync();*/
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Error loading torrent from disk: " + ex.Message);
                    Console.WriteLine("Stacktrace: " + ex.ToString());
                }
            };

            watcher.Start();
            watcher.ForceScan();
        }
    }
}
