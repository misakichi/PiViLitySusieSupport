using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace PvlSppComm
{
    enum SpiBridgeStdMessage
    {
        None = 0,

        // 新規セッション要求
        NewSession = 100,
        // 新規セッション応答
        CreatedSession = 110,
        // 新規セッション応答(失敗)
        FailedNewSession = 120,

        // セッション解放要求
        FreeSession = 200,

        // 終了要求
        Exit = 1000,
    };

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    struct SpiBridgeStandardMessageHeader
    {
        unsafe public SpiBridgeStandardMessageHeader()
        {
            commonHeader.msgType = SpiBridgeMessageType.Standard;
            commonHeader.bytes = sizeof(SpiBridgeStandardMessageHeader);
            message = SpiBridgeStdMessage.None;
        }
        public SpiBridgeStandardMessageHeader(SpiBridgeStdMessage _message, int _bytes)
        {
            commonHeader.msgType = SpiBridgeMessageType.Standard;
            commonHeader.bytes = _bytes;
            message = _message;

        }
        SpiBridgeMessageHeader commonHeader;
        SpiBridgeStdMessage message;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    struct SpiBridgeStandardMessageNewSession
    {
        unsafe public SpiBridgeStandardMessageNewSession()
        {
            header = new SpiBridgeStandardMessageHeader(SpiBridgeStdMessage.NewSession, sizeof(SpiBridgeStandardMessageNewSession));
        }
        public SpiBridgeStandardMessageHeader header;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    unsafe struct SpiBridgeStandardMessageCreatedSession
    {
        unsafe public SpiBridgeStandardMessageCreatedSession()
        {
            header = new SpiBridgeStandardMessageHeader(SpiBridgeStdMessage.CreatedSession, sizeof(SpiBridgeStandardMessageCreatedSession));
        }
        public SpiBridgeStandardMessageHeader header;
        public int sessionId;
        public fixed char namedSuffix[256];

    };

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    struct SpiBridgeStandardMessageFailedNewSession
    {
        unsafe public SpiBridgeStandardMessageFailedNewSession()
        {
            header = new SpiBridgeStandardMessageHeader(SpiBridgeStdMessage.NewSession, sizeof(SpiBridgeStandardMessageFailedNewSession));
        }
        public SpiBridgeStandardMessageHeader header;
        public int errorCode;
    };
    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    struct SpiBridgeStandardMessageFreeSession
    {
        unsafe public SpiBridgeStandardMessageFreeSession()
        {
            header = new SpiBridgeStandardMessageHeader(SpiBridgeStdMessage.FreeSession, sizeof(SpiBridgeStandardMessageFreeSession));
        }
        public SpiBridgeStandardMessageHeader header;
        public int processId;
        public int sessionId;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    struct SpiBridgeStandardMessageExit
    {
        unsafe public SpiBridgeStandardMessageExit()
        {
            header = new SpiBridgeStandardMessageHeader(SpiBridgeStdMessage.Exit, sizeof(SpiBridgeStandardMessageExit));
        }
        public SpiBridgeStandardMessageHeader header;
    };
}
