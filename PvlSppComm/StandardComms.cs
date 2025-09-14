using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace PvlSppComm
{


    public class StandardComms
    {
        static StandardComms _instance = new();
        public static StandardComms Instance =>_instance;


        public bool IsRunnningBridge => _bridgeProcess != null;
        private Process? _bridgeProcess;
        private BinaryReader? StandardReader = null;
        private BinaryReader? ErrorReader = null;
        private BinaryWriter? StandardWriter = null;

        /// <summary>
        /// 唯一のインスタンス生成要コンストラクタ。
        /// プロセスを生成する
        /// </summary>
        private StandardComms()
        {
        }

        ~StandardComms()
        {
        }

        public bool Initialize()
        {
            if (IsRunnningBridge)
                return false;

            var psi = new ProcessStartInfo("PvlWin32SpiBridge.exe")
            {
                UseShellExecute = false, // IOリダイレクトには必須
                RedirectStandardInput = true,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true // コンソールを表示しない
            };
            _bridgeProcess = Process.Start(psi);
            if (_bridgeProcess != null)
            {
                StandardReader = new BinaryReader(_bridgeProcess.StandardOutput.BaseStream);
                ErrorReader = new BinaryReader(_bridgeProcess.StandardError.BaseStream);
                StandardWriter = new BinaryWriter(_bridgeProcess.StandardInput.BaseStream);
            }

            var tick1 = DateTime.Now;
            SendNone();
            var tick2 = DateTime.Now;
            var resultStr = $"Initilized bridge. send <none> message.(send:{(tick2 - tick1).Milliseconds}msec";
            Trace.WriteLine(resultStr);

            return IsRunnningBridge;
        }
        public bool Terminate()
        {
            if (!IsRunnningBridge)
                return false;


            var tick1 = DateTime.Now;
            SendExit();

            //StandardReader?.Dispose(); StandardWriter = null;
            //ErrorReader?.Dispose(); ErrorReader = null;

            var tick2 = DateTime.Now;
            var exitRet = _bridgeProcess?.WaitForExit(new TimeSpan(1, 1, 20)) ?? false;
            var tick3 = DateTime.Now;
            string resultStr;
            if (exitRet && _bridgeProcess!=null)
                resultStr = $"Success eixt ipc process.(code:{_bridgeProcess.ExitCode} send:{(tick2-tick1).Milliseconds}msec wait:{(tick3 - tick2).Milliseconds}msec";
            else
                resultStr = $"Failed eixt ipc process.";
            Trace.WriteLine(resultStr);
            
            _bridgeProcess = null;
            StandardReader = null;
            ErrorReader = null;
            StandardWriter = null;

            return !IsRunnningBridge;
        }

        void SendNone()
        {
            if (StandardWriter == null)
                return;

            Span<SpiBridgeStandardMessageHeader> message = stackalloc SpiBridgeStandardMessageHeader[1];
            message[0] = new();
            Span<byte> byteSpan = MemoryMarshal.Cast<SpiBridgeStandardMessageHeader, byte>(message);
            StandardWriter.Write(byteSpan);
            StandardWriter.Flush();
            
        }
        void SendExit()
        {
            if (StandardWriter == null)
                return;

            Span<SpiBridgeStandardMessageExit> message = stackalloc SpiBridgeStandardMessageExit[1];
            message[0] = new();
            Span<byte> byteSpan = MemoryMarshal.Cast<SpiBridgeStandardMessageExit, byte>(message);
            StandardWriter.Write(byteSpan);
            StandardWriter.Flush();
        }
    }
}
