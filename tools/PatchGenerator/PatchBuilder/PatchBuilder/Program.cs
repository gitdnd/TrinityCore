using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PatchBuilder
{
    class Program
    {
        static void Main(string[] args)
        {
            //uint x = 1397311310;
            //string y = string.Format("{0:X}", x);

            var dir = "C:\\Users\\Harry_\\Desktop\\World\\Maps\\Azeroth";

            var filePaths = Directory.GetFiles(dir);
            foreach (var filePath in filePaths)
            {
                if (filePath.Contains("Azeroth"))
                {
                    var offset = filePath.LastIndexOf('\\');
                    var xStr = filePath.Substring(offset + "Azeroth".Length + 2, 2);
                    var yStr = filePath.Substring(offset + "Azeroth".Length + 5, 2);

                    var x = int.Parse(xStr);
                    var y = int.Parse(yStr);

                    if (x >= 27 && x <= 39 && y >= 32 && y <= 38)
                    {
                        var newPath = filePath.Replace("Azeroth", "Stromgarde");
                        File.Copy(filePath, newPath);
                    }
                }
            }
        }
    }
}
