#include "stdafx.h"
#include "InstanceSession.h"
#include "Pipe.h"
#include <string>

using namespace PvlIpc;

CInstanceSession::CInstanceSession()
	: CSessionMessage()
{
	for (int i = 0; i < OverlapHandles; i++)
	{
		overlapEvent_[i] = INVALID_HANDLE_VALUE;
		overlap_[i] = {};
	}
}

//CInstanceSession::~CInstanceSession()
//{
//}
static std::wstring MakePipeFullName(const wchar_t* name)
{
	return std::wstring(L"\\\\.\\pipe\\") + name;
}

/// <summary>
/// サーバー初期化
/// </summary>
/// <param name="inName"></param>
/// <param name="outName"></param>
/// <returns></returns>
bool CInstanceSession::InitServer(const wchar_t* name, bool isOverlaped)
{
	for (int i = 0; i < OverlapHandles; i++)
		overlapEvent_[i] = CreateEvent(NULL, FALSE, FALSE, NULL);

	int withOverlapped = isOverlaped ? FILE_FLAG_OVERLAPPED : 0;

	PipeCreateOptions options;
	options.defaultWaitTimeMs = PipeDefaultWait;
	options.inBufferSize = MessageSizeMax * 10;
	options.outBufferSize = MessageSizeMax * 10;
	options.isOverlaped = isOverlaped;
	auto fullName = MakePipeFullName(name);
	auto pipe = new Pipe();
	if(pipe->Create(fullName.c_str(), &options))
	{
		DebugPrint(L"Pipe %s created.\n", fullName.c_str());
		InitIo(*pipe, isOverlaped);
		pipe_ = pipe;
		for (int i = 0; i < OverlapHandles; i++)
		{
			overlap_[i].Internal = STATUS_PENDING;
			overlap_[i].hEvent = overlapEvent_[i];
		}
		ConnectNamedPipe(*pipe_, &overlap_[0]);
		StartProcessThread();
		return true;
	}
	else
	{
		DebugPrint(L"Failed to create pipe %s(Last error:0x%08x)\n", fullName.c_str(), GetLastError());
		delete pipe;
		Terminate();
		return false;
	}
}

/// <summary>
/// クライアント初期化
/// </summary>
bool CInstanceSession::InitClient(const wchar_t* name, bool isOverlaped)
{
	for (int i = 0; i < OverlapHandles; i++)
		overlapEvent_[i] = CreateEvent(NULL, FALSE, FALSE, NULL);

	auto fullName = MakePipeFullName(name);
	auto pipe = new Pipe();
	PipeConnectOptions options;
	options.isOverlaped = isOverlaped;
	options.direction = PipeDirection::InOut;

	if (pipe->Connect(name, &options))
	{
		pipe_ = pipe;
		InitIo(*pipe, isOverlaped);
		StartProcessThread();
		return true;
	}
	else
	{
		delete pipe;
		Terminate();
		return false;
	}
}

/// <summary>
/// 終了処理
/// </summary>
void CInstanceSession::Terminate()
{
	for (int i = 0; i < OverlapHandles; i++)
	{
		if(overlapEvent_[i]!= INVALID_HANDLE_VALUE )
		{
			CloseHandle(overlapEvent_[i]);
		}	
	}

	InitIo(INVALID_HANDLE_VALUE,false);
	for (int i = 0; i < OverlapHandles; i++)
		overlapEvent_[i] = INVALID_HANDLE_VALUE;

}

/// <summary>
/// メッセージ処理スレッド開始
/// </summary>
void CInstanceSession::StartProcessThread()
{
	CreateThread(NULL, 0, [](void* p) -> DWORD
		{
			auto instance = ((CInstanceSession*)p);
			instance->running_ = true;
			SessionUpdateResult result;
			while ((result = instance->Update()).IsFailed() == false)
			{
				if(result == SessionUpdateResult::Enum::Timeout)
				{
					Sleep(0);
					continue;
				}
			}
			instance->running_ = false;
			return 0;
		}, this, 0, NULL);
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="msg"></param>
/// <returns></returns>
SessionUpdateResult CInstanceSession::UpdateInner(SessionMessageHeader* msg)
{
	return Base::UpdateInner(msg);
}
