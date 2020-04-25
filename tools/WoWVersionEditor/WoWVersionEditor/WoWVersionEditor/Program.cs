using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WoWVersionEditor
{
    class Program
    {
        private const uint _VersionOffset = 0x4C99F0; // 5020144 

        static void Main(string[] args)
        {
            args = new string[] { "D:\\WoW 3.3.5a\\Wow_customclass.exe", "+" };
            try
            {
                var path = args[0];
                var currentVersion = ReadVersion(path);
                var newVersion = args[1].Equals("+") ? ushort.Parse((currentVersion + 1).ToString()) : ushort.Parse(args[1]);
                Console.WriteLine("Updating: " + path);
                Console.WriteLine("Current version: " + currentVersion);
                Console.WriteLine("New version: " + newVersion);
                WriteVersion(path, $"{path}_{newVersion}.exe", newVersion);
                Console.WriteLine("Done");
            }
            catch (Exception e)
            {
                Console.WriteLine($"[ERROR] {e.GetType()}: {e.Message}\n{e}");
            }
        }

        static ushort ReadVersion(string filePath)
        {
            using (var stream = File.Open(filePath, FileMode.Open))
            {
                stream.Position = _VersionOffset;
                using (var reader = new BinaryReader(stream))
                {
                    return reader.ReadUInt16();
                }
            }
        }

        static void WriteVersion(string oldFilePath, string newFilePath, ushort version)
        {
            File.Copy(oldFilePath, newFilePath, true);
            using (var stream = File.Open(newFilePath, FileMode.Open))
            {
                stream.Position = _VersionOffset;
                using (var reader = new BinaryWriter(stream))
                {
                    reader.Write(version);
                }
            }
        }
    }
}
