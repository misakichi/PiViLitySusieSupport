#include "stdafx.h"
#include "InstanceSession.h"
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

CInstanceSession::~CInstanceSession()
{
}
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
bool CInstanceSession::InitServer(const wchar_t* inName, const wchar_t* outName, bool isOverlaped)
{
	for (int i = 0; i < OverlapHandles; i++)
		overlapEvent_[i] = CreateEvent(NULL, FALSE, FALSE, NULL);

	int withOverlapped = isOverlaped ? FILE_FLAG_OVERLAPPED : 0;
	auto inPipe = CreateNamedPipeW(MakePipeFullName(inName).c_str(), PIPE_ACCESS_INBOUND | withOverlapped, 0, 2, MessageSizeMax * 10, MessageSizeMax * 10, PipeDefaultWait, NULL);
	auto outPipe = CreateNamedPipeW(MakePipeFullName(outName).c_str(), PIPE_ACCESS_OUTBOUND | withOverlapped, 0, 2, MessageSizeMax * 10, MessageSizeMax * 10, PipeDefaultWait, NULL);
	InitIo(inPipe, outPipe, isOverlaped);
	if(inPipe!=INVALID_HANDLE_VALUE && outPipe!=INVALID_HANDLE_VALUE)
	{
		OVERLAPPED ol1 = overlap_[0];
		ol1.Internal = STATUS_PENDING;
		ol1.hEvent = overlapEvent_[0];
		OVERLAPPED ol2 = overlap_[1];
		ol2.Internal = STATUS_PENDING;
		ol2.hEvent = overlapEvent_[1];
		ConnectNamedPipe(read_, &ol1);
		ConnectNamedPipe(write_, &ol2);
		return true;
	}
	else
	{
		Terminate();
		return false;
	}
}

/// <summary>
/// クライアント初期化
/// </summary>
bool CInstanceSession::InitClient(const wchar_t* inName, const wchar_t* outName, bool isOverlaped)
{
	for (int i = 0; i < OverlapHandles; i++)
		overlapEvent_[i] = CreateEvent(NULL, FALSE, FALSE, NULL);

	int flag = isOverlaped ? FILE_FLAG_OVERLAPPED : 0;
	auto inPipe = CreateFileW(MakePipeFullName(outName).c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, flag, NULL);
	auto outPipe = CreateFileW(MakePipeFullName(inName).c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, flag, NULL);
	InitIo(inPipe, outPipe, isOverlaped);
	if (inPipe != INVALID_HANDLE_VALUE && outPipe != INVALID_HANDLE_VALUE)
	{
		return true;
	}
	else
	{
		Terminate();
		return false;
	}
}
void CInstanceSession::Terminate()
{
	for (int i = 0; i < OverlapHandles; i++)
	{
		if(overlapEvent_[i]!= INVALID_HANDLE_VALUE )
		{
			CloseHandle(overlapEvent_[i]);
		}	
	}
	if( read_ != INVALID_HANDLE_VALUE )
	{
		CloseHandle(read_);
	}
	if( write_ != INVALID_HANDLE_VALUE )
	{
		CloseHandle(write_);
	}
	InitIo(INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE,false);
	for (int i = 0; i < OverlapHandles; i++)
		overlapEvent_[i] = INVALID_HANDLE_VALUE;

}

bool CInstanceSession::Update()
{
	//if (ret >= WAIT_OBJECT_0 && ret <= WAIT_OBJECT_0 + 1)
	//{	//待機完了
	//}
	//else
	//{
	//	//失敗した
	//}


	return true;
}

