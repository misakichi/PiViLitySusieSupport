using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace PvlSppComm
{
    enum SpiBridgeMessageType
    {
        None = 0,
        Standard = 1,
    };

    [StructLayout(LayoutKind.Sequential, Pack = 4)]
    struct SpiBridgeMessageHeader
    {
        unsafe public SpiBridgeMessageHeader()
        {
            msgType = SpiBridgeMessageType.None;
            bytes = sizeof(SpiBridgeMessageHeader);
        }
        public SpiBridgeMessageHeader(SpiBridgeMessageType _type, int _bytes)
        {
            msgType = _type;
            bytes = _bytes;
        }
        public SpiBridgeMessageType msgType;
        public int bytes;
    }



}
