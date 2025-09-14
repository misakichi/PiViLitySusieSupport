using PiViLityCore.Plugin;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Resources;
using System.Text;
using System.Threading.Tasks;

namespace PvlSsp
{
    public class PvlSsp : PiViLityCore.Plugin.IModuleInformation
    {


        public string Name => throw new NotImplementedException();

        public string Description => throw new NotImplementedException();

        public string OptionItemName => throw new NotImplementedException();

        public ResourceManager? ResourceManager => throw new NotImplementedException();

        public bool Initialize() => true;

        public bool Terminate() => true;
    }
}
