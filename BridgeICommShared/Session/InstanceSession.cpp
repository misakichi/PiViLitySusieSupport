#include "stdafx.h"
#include "InstanceSession.h"
#include "Pipe.h"
#include "Debug/Console.h"
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
		Debug::Print(L"Instance Pipe %s created.\n", fullName.c_str());
		InitIo(*pipe);
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
		Debug::PrintLastErrorMessage();
		Debug::Print(L"Failed to create pipe %s\n", fullName.c_str());
		Debug::PrintLastErrorMessage();

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
		overlapEvent_[i] = PVL_CHECK_HANDLE(CreateEvent(NULL, FALSE, FALSE, NULL));

	auto fullName = MakePipeFullName(name);
	auto pipe = new Pipe();
	PipeConnectOptions options;
	options.isOverlaped = isOverlaped;
	options.direction = PipeDirection::InOut;
	Debug::Print(L"Instance Pipe Connectting....\n");

	if (pipe->Connect(fullName.c_str(), &options))
	{
		Debug::Print(L"Instance Pipe %s connected.\n", fullName.c_str());
		pipe_ = pipe;
		InitIo(*pipe);
		StartProcessThread();
		return true;
	}
	else
	{
		Debug::PrintLastErrorMessage();
		Debug::Print(L"Failed to connect pipe %s\n", fullName.c_str());
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

	InitIo(INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, false);
	for (int i = 0; i < OverlapHandles; i++)
		overlapEvent_[i] = INVALID_HANDLE_VALUE;

}

/// <summary>
/// メッセージ処理スレッド開始
/// </summary>
void CInstanceSession::StartProcessThread()
{
	isRunning_ = true;
	if (CreateThread(NULL, 0, [](void* p) -> DWORD
		{
			Debug::Print(L"Instance Session Thread Started(waiting connect....).\n");
			auto instance = ((CInstanceSession*)p);
			WaitForSingleObject(instance->overlapEvent_[0], INFINITE);
			Debug::Print(L"Instance Session Thread Connected.\n");
			instance->running_ = true;

			instance->ThreadFunc();

			Debug::Print(L"Shutdown Instance Session Thread.\n");
			instance->running_ = false;
			return 0;
		}, this, 0, NULL) == NULL)
	{
		isRunning_ = false;
	}
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
