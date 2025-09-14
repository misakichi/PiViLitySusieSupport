using PiViLityCore.Plugin;
using System.Drawing;

namespace PvlSsp
{
    public class SusiePluginImageReader : ImageReaderBase
    {
        public override void Dispose()
        {
            return;
        }

        public override System.Drawing.Image? GetImage()
        {
            return null;
        }

        public override Size GetImageSize()
        {
            return new Size(0, 0);
        }

        public override List<string> GetSupportedExtensions()
        {
            return [];
        }

        public override System.Drawing.Image? GetThumbnailImage(Size size)
        {
            return null;
        }

        public override bool IsSupported()
        {
            return false;
        }

        public override bool SetFilePath(string filePath)
        {
            return false;
        }
    }
}
