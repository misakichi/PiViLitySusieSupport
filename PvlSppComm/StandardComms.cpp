#include "stdafx.h"
#include "StandardComms.h"
#include "InstanceComms.h"
#include "SpiInstanceComms.h"
#include "Debug/Console.h"
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
StandardComms::~StandardComms()
{
    Terminate();
}

static void s_callbackFunc(PvlIpc::CInstanceSession* newClientSession, void* arg)
{
    auto handle = GCHandle::FromIntPtr((System::IntPtr)arg);
    auto comms = safe_cast<StandardComms^>(handle.Target);
    comms->RecieveNewSession(newClientSession);
}


/// <summary>
/// 名前付きパイプを作成し、外部ブリッジプロセスを起動してセッションを初期化し、セッション用スレッドを開始します。既に実行中の場合は初期化を行いません。
/// </summary>
/// <returns>初期化とプロセス起動に成功して通信が開始された場合にtrueを返します。既に実行中であるか、初期化や起動に失敗した場合はfalseを返します。</returns>
bool StandardComms::StartupServer()
{
	if (isRunnning_)
		return false;
    
#if 1
    session_ = new PvlIpc::CStandardSession();
    if (session_->StartupServer())
    {
        GCHandle handle = GCHandle::Alloc(this);
        thisHandle_ = GCHandle::ToIntPtr(handle).ToPointer();
        session_->SetNewSessionResultCallback(s_callbackFunc, thisHandle_);
        isRunnning_ = true;
        return true;
    }
    else
    {
        return false;
    }
#else
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
		if(auto th = CreateThread(nullptr, 0, StandardSessionThreadEntry, session_, 0, nullptr))
		    SetThreadDescription(th, L"PvlSppComm::StandardComms::SessionThread");
    }
    else
    {
        delete pipe;
        CloseHandle(overlap.hEvent);
    }


	return isRunnning_;
	return false;
#endif
}


bool StandardComms::Terminate()
{
    if (thisHandle_)
    {
        GCHandle::FromIntPtr((System::IntPtr)thisHandle_).Free();
        thisHandle_ = nullptr;
    }
    if (isRunnning_)
    {
        SendExit();

        WaitForSingleObject(process_, 60 * 1000);//1 minites
        delete session_;
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
        if(newSpiSessionoHandler_)
            newSpiSessionoHandler_->Invoke(spiComms);
    }
}

