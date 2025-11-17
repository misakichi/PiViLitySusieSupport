#include "stdafx.h"
#include "StandardComms.h"

#include "InstanceComms.h"
#include "Session/BridgeMessage.h"
#include "Session/StandardSession.h"
#include "Pipe.h"
#include <thread>

using namespace PvlSppComm;
using namespace System::Runtime::InteropServices;


StandardComms::StandardComms()
{
}

static void s_callbackFunc(PvlIpc::CInstanceSession* newClientSession, void* arg)
{
    auto handle = GCHandle::FromIntPtr((System::IntPtr)arg);
    auto comms = safe_cast<StandardComms^>(handle.Target);
    comms->RecieveNewSession(newClientSession);
}

bool StandardComms::Initialize()
{
	if (isRunnning_)
		return false;
    
    const wchar_t * pipeBaseName = L"\\\\.\\pipe\\PvlSppComm_In_";
    wchar_t inReadName[256];
    wchar_t inWriteName[256];
    auto curProcID = GetCurrentProcessId();
	swprintf_s(inReadName, L"%s%08x_ServerRead", pipeBaseName, curProcID);
	swprintf_s(inWriteName, L"%s%08x_ServerWrite", pipeBaseName, curProcID);

	ioPipe_ = new PvlIpc::Pipe(inReadName, inWriteName, true);

    STARTUPINFOW si = {};
    si.cb =sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi = {};

	OVERLAPPED olIn = {};
	olIn.Internal = STATUS_PENDING;
	olIn.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    OVERLAPPED olOut = {};
    olOut.Internal = STATUS_PENDING;
    olOut.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    auto ret1 = ConnectNamedPipe(ioPipe_->read_, &olIn);
	auto ret2 = ConnectNamedPipe(ioPipe_->write_, &olOut);

    wchar_t cmd[2048];
    swprintf_s(cmd, L"PvlWin32SpiBridge.exe -i=%s -o=%s -dbgConsole -is-overlap", inWriteName, inReadName);
    isRunnning_ = CreateProcessW(
        nullptr,
        cmd,
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi
    );
    if (isRunnning_)
    {
		WaitForSingleObject(olIn.hEvent, 2*1000);
        WaitForSingleObject(olOut.hEvent, 0);
		CloseHandle(olIn.hEvent);
		CloseHandle(olOut.hEvent);

        CloseHandle(pi.hThread);
        process_ = pi.hProcess;
        session_ = new PvlIpc::CStandardSession();
        GCHandle thisHandle = GCHandle::Alloc(this);

		session_->SetNewSessionResultCallback(s_callbackFunc, GCHandle::ToIntPtr(thisHandle).ToPointer());
        session_->InitIo(ioPipe_->read_, ioPipe_->write_, true);
    }
    else
    {
        CloseHandle(olIn.hEvent);
        CloseHandle(olOut.hEvent);
    }


	return isRunnning_;
	return false;
}

bool StandardComms::Terminate()
{
    if (isRunnning_)
    {
        SendExit();

        WaitForSingleObject(process_, 60 * 1000);//1 minites
        delete session_;
        delete ioPipe_;
        CloseHandle(process_);
        ioPipe_ = nullptr;
        process_ = nullptr;
        session_ = nullptr;
        isRunnning_ = false;
    }
    return true;
}


bool StandardComms::SendNone()
{
    return session_->SendNone();
}

bool StandardComms::SendExit()
{
    return session_->SendExit();
}
bool StandardComms::SendNewSession()
{
    return session_->SendNewSession();
}

void StandardComms::RecieveNewSession(PvlIpc::CInstanceSession* newInstance)
{
//	auto client = gcnew InstanceComms();
//    client->
}
