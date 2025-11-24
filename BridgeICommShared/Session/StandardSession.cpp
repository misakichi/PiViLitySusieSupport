#include "stdafx.h"
#include "StandardSession.h"
#include "SpiInstanceSession.h"
#include "Debug/Console.h"
#include "Pipe.h"

#pragma comment(lib, "Ole32.lib")

using namespace PvlIpc;

CStandardSession::CStandardSession()
{
}

CStandardSession::~CStandardSession() 
{
}

void CStandardSession::StartProcessThread()
{
	
	isRunning_ = true;
	if (auto th = CreateThread(
		nullptr, 0
		, [](void* p)
		{
			auto session = ((PvlIpc::CStandardSession*)p);
			auto ret = (DWORD)session->ThreadFunc();
			return ret;
		}
		, this, 0, nullptr))
	{
		SetThreadDescription(th, L"PvlSppComm::StandardComms::SessionThread");
	}
	else
	{
		isRunning_ = false;
	}
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
bool CStandardSession::StartupServer()
{
	const wchar_t* pipeBaseName = L"\\\\.\\pipe\\PvlSppComm_In_";
	wchar_t name[256];
	auto curProcID = GetCurrentProcessId();
	swprintf_s(name, L"%s%08x", pipeBaseName, curProcID);

	PvlIpc::PipeCreateOptions options;
	options.isOverlaped = true;
	auto pipe = new PvlIpc::Pipe();
	if (!pipe->Create(name, &options))
	{

		return false;
	}

	STARTUPINFOW si = {};
	si.cb = sizeof(si);
	si.dwFlags |= STARTF_USESTDHANDLES;

	PROCESS_INFORMATION pi = {};

	OVERLAPPED overlap = {};
	overlap.Internal = STATUS_PENDING;
	overlap.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (overlap.hEvent == NULL)
	{
		delete pipe;
		return false;
	}
	ConnectNamedPipe(*pipe, &overlap);
	if (GetLastError()!= ERROR_IO_PENDING)
	{
		CloseHandle(overlap.hEvent);
		delete pipe;
		return false;
	}

	wchar_t cmd[2048];
	swprintf_s(cmd, L"PvlWin32SpiBridge.exe -pipe=%s -dbgConsole -is-overlap", name);
	auto isStartup = CreateProcessW(
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
	if (isStartup)
	{
		WaitForSingleObject(overlap.hEvent, 2 * 1000);
		CloseHandle(overlap.hEvent);
		CloseHandle(pi.hThread);
		process_ = pi.hProcess;
		InitIo(*pipe);
		pipe_ = pipe;

		StartProcessThread();

	}
	else
	{
		process_ = NULL;
		delete pipe;
		CloseHandle(overlap.hEvent);
	}


	return process_ != NULL;
}



bool CStandardSession::ConnectToParent(const wchar_t* pipeName, bool isOverlap)
{
	PVL_DBG_PRINT(L"Pipe New:");
	auto pipe = new PvlIpc::Pipe();
	PVL_DBG_PRINT(L"%p\n", pipe);

	PVL_DBG_PRINT(L"Connecting to pipe %s\n", pipeName);
	PvlIpc::PipeConnectOptions options;
	options.isOverlaped = isOverlap != 0;
	if (pipe->Connect(pipeName, &options))
	{
		PVL_DBG_PRINT(L"Pipe connected.\n");
	}
	else
	{
		PVL_DBG_PRINT(L"Failed to connect pipe %s\n", pipeName);
		return false;
	}

	pipe_ = pipe;
	InitIo(*pipe);

	StartProcessThread();

	return true;
}

bool CStandardSession::ConnectToParent(const char* pipeName, bool isOverlap)
{
	PVL_DBG_PRINT_A("Pipe New:");
	auto pipe = new PvlIpc::Pipe();
	PVL_DBG_PRINT_A("%p\n", pipe);

	PVL_DBG_PRINT_A("Connecting to pipe %s\n", pipeName);
	PvlIpc::PipeConnectOptions options;
	options.isOverlaped = isOverlap != 0;
	if (pipe->Connect(pipeName, &options))
	{
		PVL_DBG_PRINT_A("Pipe connected.\n");
	}
	else
	{
		PVL_DBG_PRINT_A("Failed to connect pipe %s\n", pipeName);
		return false;
	}

	pipe_ = pipe;
	InitIo(*pipe);

	StartProcessThread();

	return true;
}


/// <summary>
/// update session.
/// read message and process it.
/// </summary>
/// <returns>
/// true: continue, false: exit
/// </returns>
SessionUpdateResult CStandardSession::UpdateInner(SessionMessageHeader* _msg)
{
	PVL_DBG_PRINT(L"CStandardSession::UpdateInner\n");
	if (auto msg = _msg->Cast<StandardSessionMessageHeader>())
	{
		if (msg->message == StandardSessionMessageNone::Message)
		{
			return SessionUpdateResult::Enum::Succcess;
		}
		else if (auto newSessionMsg = msg->Cast<StandardSessionMessageNewSession>())
		{
			Debug::Print(L"New Session Request.\n");
			//新規セッション
			auto inId = sessionId_++;
			auto outId = sessionId_++;
			auto procId = GetCurrentProcessId();
			Name256Buffer name;
			bool named = CraetePipeName(procId, inId, L"_BridgePipe", name);
			if (named)
			{
				Debug::Print(L"Create Session Server Pipe(%s).\n", name.data());
				auto instance = new CSpiInstanceSession();
				instance->SetEnableDebug(debug_);
				if (instance->InitServer(name.data(), true))
				{
					Debug::Print(L"Server Initialized.\n");
					if (SendResponseCreatedSession(procId, inId, outId))
					{
					}
				}
				else
				{
					Debug::Print(L"Failed Server Init.\n");
					delete instance;
				}
			}
			return SessionUpdateResult::Enum::Succcess;
		}
		else if (auto createdSession = msg->Cast<StandardSessionMessageResponseCreatedSession>())
		{
			Debug::Print(L"New Session Created Response.\n");
			//セッション作成応答
			if (newSessionResultCallback_)
			{
				auto procId = createdSession->procId;
				Name256Buffer name;
				CraetePipeName(procId, createdSession->inId, L"_BridgePipe", name);
				auto instance = newSessionCreateCallback_ ? newSessionCreateCallback_(newSessionCreateCallbackArg_) : new CSpiInstanceSession();
				if (instance->InitClient(name.data(), true))
				{
					newSessionResultCallback_(instance, newSessionResultCallbackArg_);
				}
				else
				{
					delete instance;
					newSessionResultCallback_(nullptr, newSessionResultCallbackArg_);
				}
			}
			return SessionUpdateResult::Enum::Succcess;
		}
		else if (auto failedSession = msg->Cast<StandardSessionMessageResponseFailedNewSession>())
		{
			Debug::Print(L"New Session Failed Response.\n");
			//セッション作成応答
			if (newSessionResultCallback_)
			{
				newSessionResultCallback_(nullptr, newSessionResultCallbackArg_);
			}
			return SessionUpdateResult::Enum::Succcess;
		}
		else if (auto freeSessionMsg = msg->Cast<StandardSessionMessageFreeSession>())
		{
			Debug::Print(L"Free Session Request.\n");
			//セッション解放
			return SessionUpdateResult::Enum::Succcess;
		}
		else if (auto exitMsg = msg->Cast<StandardSessionMessageExit>())
		{
			Debug::Print(L"Exit Session Request.\n");
			//終了
			exitCode_ = 0;
			return SessionUpdateResult::Enum::Exit;
		}
	}
	return Base::UpdateInner(_msg);

}


/// <summary>
/// return exit code
/// </summary>
/// <returns></returns>
int CStandardSession::ExitCode() const
{
	return exitCode_;
}

/// <summary>
/// 何もしないメッセージ送信
/// </summary>
bool CStandardSession::SendNone()
{
	StandardSessionMessageNone msg;
	msg.Init();
	return SendCommMessage(msg);
}

/// <summary>
/// セッションの終了メッセージを送信します。
/// </summary>
bool CStandardSession::SendExit()
{
	StandardSessionMessageExit msg;
	msg.Init();
	return SendCommMessage(msg);
}

/// <summary>
/// インスタンスセッション作成要求メッセージ送信
/// </summary>
/// <returns></returns>
bool CStandardSession::SendNewSession()
{
	if(!newSessionResultCallback_)
	{
		//コールバックが設定されていない場合は送信しない
		return false;
	}

	StandardSessionMessageNewSession msg;
	msg.Init();
	return SendCommMessage(msg);
}

/// <summary>
/// インスタンスセッション作成応答メッセージ送信
/// </summary>
/// <param name="_ProocId"></param>
/// <param name="_InPipeId"></param>
/// <param name="_OutPipeId"></param>
/// <returns></returns>
bool CStandardSession::SendResponseCreatedSession(int _ProocId, uint32_t _InPipeId, uint32_t _OutPipeId)
{
	StandardSessionMessageResponseCreatedSession msg;
	msg.Init();
	msg.procId = _ProocId;
	msg.inId = _InPipeId;
	msg.outId = _OutPipeId;
	return SendCommMessage(msg);
}

/// <summary>
/// セッションIDに基づいてパイプ名を生成します。
/// </summary>
/// <param name="_SessionId">パイプ名の生成に使用するセッションID。</param>
/// <param name="name">生成されたパイプ名を格納するName256Buffer型の参照。</param>
bool CStandardSession::CraetePipeName(int _ProcId, int _SessionId, const wchar_t* _SuffixStr, Name256Buffer& _Name)
{
	if (StringFromGUID2(s_NamedPipeGuid, _Name.data(), (int)_Name.size()) == 0)
		return false;
		
	wchar_t addStr[64] = {};
	swprintf_s(addStr, L"_%x_%s_%x", _ProcId, _SuffixStr, _SessionId);
	wcscat_s(_Name.data(), _Name.size(), addStr);

	return true;
}
