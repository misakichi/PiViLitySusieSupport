using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace PvlSsp
{

    enum SpiBridgeMessageType 
    {
        None = 0,
	    Standard = 1,
    };

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    struct SpiBridgeMessageHeader
    {
        SpiBridgeMessageType msgType;
        int bytes;
    }

    class StandardComms
    {
        static StandardComms Instance { get; } = new();

        private Process? _bridgeProcess;

        public bool IsRunnningBridge => _bridgeProcess != null;
        public StreamReader? StandardReader => _bridgeProcess?.StandardOutput;
        public StreamReader? ErrorReader => _bridgeProcess?.StandardError;
        public StreamWriter? StandardWriter => _bridgeProcess?.StandardInput;
        private StandardComms()
        {
            var psi = new ProcessStartInfo("PvlWin32SpiBridge.exe")
            {
                UseShellExecute = false, // IOリダイレクトには必須
                RedirectStandardInput = true,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true // コンソールを表示しない
            };
            _bridgeProcess = Process.Start(psi);
        }

        ~StandardComms()
        {
        }

    }
}
