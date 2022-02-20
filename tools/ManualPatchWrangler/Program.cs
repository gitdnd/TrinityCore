using System;
using StormLibSharp;
using System.IO.Compression;

namespace ManualPatchWrangler
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
            try
            {
                var directory = "D:\\_NewProjectWoW\\Client";
                Log($"Parsing {directory} directory...");
                var fileList = ParseDirectory(directory, new List<string>());
                Log("Creating MPQ...");
                CreateMpqFromFileList("patch-4.MPQ", directory, fileList);
                Log("Creating ZIP...");
                var zipList = new List<string>
                {
                    "patch-4.MPQ",
                    "Wow_HourOfTwilight.exe"
                };
                CreateZip("HourOfTwilight_Manual.zip", zipList);
                Log("Deleting patch-4.MPQ");
                File.Delete("patch-4.MPQ");
            }
            catch (Exception ex)
            {
                Log($"[ERROR]: {ex.GetType()}: {ex.Message}\n{ex.InnerException}\n{ex}");
            }
            Log("All done.");
        }

        private static List<string> ParseDirectory(string directory, List<string> files)
        {
            if (Directory.Exists(directory))
            {
                foreach (var file in Directory.EnumerateFiles(directory))
                {
                    files.Add(file);
                }
                foreach (var dir in Directory.EnumerateDirectories(directory))
                {
                    files = ParseDirectory(dir, files);
                }
            }
            return files;
        }

        private static void CreateMpqFromFileList(string archiveName, string dataDir, List<string> exportList)
        {
            var archivePath = archiveName;
            if (File.Exists(archivePath))
            {
                File.Delete(archivePath);
            }
            var runPath = Path.GetFullPath(dataDir);
            using (var archive = MpqArchive.CreateNew(archivePath, MpqArchiveVersion.Version1))
            {
                exportList.ForEach((file) =>
                {
                    var pathToAdd = file.Substring(runPath.Length + 1);
                    archive.AddFileFromDiskWithCompression(file, pathToAdd, MpqCompressionTypeFlags.MPQ_COMPRESSION_ZLIB);
                });
            }
        }

        private static void CreateZip(string zipName, List<string> files)
        {
            using (var fileStream = new FileStream(zipName, FileMode.Create))
            {
                using (var archive = new ZipArchive(fileStream, ZipArchiveMode.Create, true))
                {
                    foreach (var file in files)
                    {
                        archive.CreateEntryFromFile(file, file, CompressionLevel.Optimal);
                    }
                }
            }
        }
    }
}
