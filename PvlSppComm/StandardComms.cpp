#include "stdafx.h"
#include "StandardComms.h"
#include "InstanceComms.h"
#include "SpiInstanceComms.h"

#include "Session/BridgeMessage.h"
#include "Session/StandardSession.h"
#include "Session/SpiInstanceSession.h"
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

DWORD __stdcall StandardSessionThreadEntry(void* p)
{
    auto session = ((PvlIpc::CStandardSession*)p);
    while (session->Update().IsFailed() == false)
    {
    }
    delete session;
	return 0;
}

bool StandardComms::Initialize()
{
	if (isRunnning_)
		return false;
    
    const wchar_t * pipeBaseName = L"\\\\.\\pipe\\PvlSppComm_In_";
    wchar_t name[256];
    auto curProcID = GetCurrentProcessId();
	swprintf_s(name, L"%s%08x", pipeBaseName, curProcID);

    PvlIpc::PipeCreateOptions options;
	options.isOverlaped = true;
	auto pipe = new PvlIpc::Pipe();
    pipe->Create(name, &options);

    STARTUPINFOW si = {};
    si.cb =sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi = {};

	OVERLAPPED overlap = {};
    overlap.Internal = STATUS_PENDING;
    overlap.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    auto ret1 = ConnectNamedPipe(*pipe, &overlap);

    wchar_t cmd[2048];
    swprintf_s(cmd, L"PvlWin32SpiBridge.exe -pipe=%s -dbgConsole -is-overlap", name);
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
		WaitForSingleObject(overlap.hEvent, 2*1000);
		CloseHandle(overlap.hEvent);

        CloseHandle(pi.hThread);
        process_ = pi.hProcess;
        session_ = new PvlIpc::CStandardSession();
        GCHandle thisHandle = GCHandle::Alloc(this);

		session_->SetNewSessionResultCallback(s_callbackFunc, GCHandle::ToIntPtr(thisHandle).ToPointer());
        session_->InitIo(*pipe, true);
        ioPipe_ = pipe;
		//_beginthreadex(nullptr, 0, StandardSessionThreadEntry, session_, 0, nullptr);
		auto th = CreateThread(nullptr, 0, StandardSessionThreadEntry, session_, 0, nullptr);
		SetThreadDescription(th, L"PvlSppComm::StandardComms::SessionThread");
    }
    else
    {
        delete pipe;
        CloseHandle(overlap.hEvent);
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
    if (auto spiSession = dynamic_cast<PvlIpc::CSpiInstanceSession*>(newInstance))
    {
        auto spiComms = gcnew SpiInstanceComms(spiSession);
        OnReciveNewSession(spiComms);
    }
}

