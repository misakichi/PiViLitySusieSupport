#include "stdafx.h"
#include "SessionMessage.h"
#include "BridgeMessage.h"
#include "assert.h"
#include <string>
#include "Debug/Console.h"
#include "Pipe.h"

using namespace PvlIpc;




CSessionMessage::CSessionMessage()
	: read_(INVALID_HANDLE_VALUE)
	, write_(INVALID_HANDLE_VALUE)
{
#ifdef _DEBUG
	debug_ = true;
#endif
	readBufferSize_ = 32 * 1024;
	readBufferOffset_ = 0;
	readBuffer_ = new uint8_t[readBufferSize_];
}

CSessionMessage::~CSessionMessage()
{
	if (pipe_)
		delete pipe_;
}

void CSessionMessage::InitIo(Pipe& duplex)
{
	//PVL_TRACE();
	InitIo((HANDLE)duplex, (HANDLE)duplex, duplex.IsOverlaped());
}


void CSessionMessage::InitIo(HANDLE _read, HANDLE _write, bool isOverlaped)
{
	//PVL_TRACE();
	PVL_DBG_PRINT(L"InitIO(R:%p W:%p OL:%s\n", _read, _write, isOverlaped ? L"True" : L"False");
	read_ = _read;
	write_ = _write;
	isOverlapped_ = isOverlaped;
}

void CSessionMessage::InitFromStdIo()
{
	//PVL_TRACE();
	auto stdIn = GetStdHandle(STD_INPUT_HANDLE);
	auto stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	InitIo(stdIn, stdOut, false);
}

BOOL CSessionMessage::ReadSync(void* buffer, size_t bytes, LPDWORD lpReaded)
{
	if (isOverlapped_)
	{
		PVL_DBG_PRINT(L"Read %d bytes...\n", bytes);
		OVERLAPPED* ol = GetOverlapped();
		if (!ReadFile(read_, buffer, (DWORD)bytes, nullptr, ol))
		{
			auto le = GetLastError();
			if (le != ERROR_IO_PENDING)
			{
				Debug::PrintLastErrorMessage(le, L"ReadSync Failed. LastError=0x%08x\n", le);
				return FALSE;
			}
		}

		DWORD waitRet;
		if ((waitRet = WaitForSingleObject(ol->hEvent, SessionTimeoutMsec)) == WAIT_TIMEOUT)
		{
			if (CancelIoEx(read_, ol))
			{
				DWORD tmp;
				GetOverlappedResult(read_, ol, &tmp, TRUE);
				ReleaseOverlapped(ol);
				return false;
			}
		}

		DWORD readed = 0;
		auto result = GetOverlappedResult(read_, ol, &readed, FALSE);
		PVL_DBG_PRINT(L"Read result:%d(%d B)\n", result, readed);
		if (lpReaded)
			*lpReaded = readed;
		ReleaseOverlapped(ol);
		return result;

	}
	else
	{
		return ReadFile(read_, buffer, (DWORD)bytes, lpReaded, nullptr);
	}
}

BOOL CSessionMessage::WriteSync(const void* buffer, size_t bytes, LPDWORD lpWrited)
{
	if (isOverlapped_)
	{
		PVL_DBG_PRINT(L"Write %d bytes...\n", bytes);
		OVERLAPPED* ol = GetOverlapped();
		if (!WriteFile(write_, buffer, (DWORD)bytes, nullptr, ol))
		{
			auto le = GetLastError();
			if (le == ERROR_NO_DATA)
			{
				//終了
				return FALSE;
			}
			else if (le != ERROR_IO_PENDING)
			{
				Debug::PrintLastErrorMessage(le, L"WriteSync Failed. LastError=0x%08x\n", le);
				return FALSE;
			}
		}
		DWORD wrote = 0;
		auto result = GetOverlappedResult(write_, ol, &wrote, TRUE);
		if (lpWrited)
			*lpWrited = wrote;
		ReleaseOverlapped(ol);
		return result;
	}
	else
	{
		return WriteFile(write_, buffer, (DWORD)bytes, lpWrited, nullptr);
	}
}

/// <summary>
/// メッセージの読み込み
/// </summary>
/// <returns></returns>
SessionMessageHeader* CSessionMessage::ReadMessageBase()
{
	if (readBufferOffset_ + MessageSizeMax > readBufferSize_)
	{
		readBufferOffset_ = 0;
	}

#if 1
	DWORD readablesSize;
	DWORD readedSize;
	SessionMessageHeader* header = nullptr;
	if (PeekNamedPipe(read_, readBuffer_, sizeof(SessionMessageHeader), &readedSize, &readablesSize, 0) && readedSize>=sizeof(SessionMessageHeader))
	{
		header = (SessionMessageHeader*)readBuffer_;
		if (ReadSync(header, header->bytes))
		{
			readBufferOffset_ += (header->bytes + 8) & (~7);
			return header;
		}
	}

	Sleep(CSessionMessage::SessionTimeoutMsec);
	static CoreSessionMessageTimeout sTimeOut;
	sTimeOut.Init();
	return &sTimeOut;

#else
	SessionMessageHeader* header = (SessionMessageHeader*)readBuffer_;
	if( ReadSync(header, sizeof(*header)) == false)
	{
		return nullptr;
	}

	auto msgBodySize = (int)(header->bytes - sizeof(SessionMessageHeader));
	if (ReadSync(((int8_t*)header) + sizeof(*header), (DWORD)msgBodySize) == false)
	{
		return nullptr;
	}
	readBufferOffset_ += (header->bytes + 8) & (~7);

	Debug::Print(L"Read Message Body Bytes=%d\n", msgBodySize);
#endif
	if (header->msgType == SessionMessageType::Standard)
	{
		auto stdHead = header->Cast<StandardSessionMessageHeader>();
		PVL_DBG_PRINT(L" Standard Message:%d\n", stdHead->Message);
	}
	return (SessionMessageHeader*)header;

}

/// <summary>
/// 指定したメッセージを内部の書き込みハンドルへ送信する。isOverlapped_がtrueの場合はオーバーラップI/Oを使用して完了を待機し、falseの場合は同期的にWriteFileを呼び出す。debug_が有効であればメッセージ情報を出力する。
/// </summary>
/// <param name="msg">送信するメッセージのヘッダー。msg.bytesでバイト数が指定され、WriteFileにそのサイズで渡される。debug_が有効な場合はmsg.msgTypeとmsg.bytesがログ出力される。</param>
/// <returns>送信が成功した場合はtrue、失敗した場合はfalseを返すことを意図している。ただし、isOverlapped_がtrueのコードパスでは現行実装が戻り値を返さない可能性があり、未定義動作になる恐れがある。</returns>
bool CSessionMessage::SendCommMessage(const SessionMessageHeader& msg, const wchar_t* msgName)
{
	PVL_DBG_PRINT(L"Send Message: %s, Type=%d, Bytes=%d\n", msgName, (int)msg.msgType, msg.bytes);

	return WriteSync(&msg, msg.bytes);
}

int CSessionMessage::ThreadFunc()
{
	PVL_DBG_PRINT(L"Start session loop\n");
	isRunning_ = true;
	auto tm = GetTickCount64();
	int timeout = 0;
	int ret = 0;
	while (1)
	{
		auto ntm = GetTickCount64();
		if (ntm > tm + 1000)
		{
			CoreSessionMessageKeepAlive ka;
			ka.Init();
			if (SendCommMessage(ka, L"KeepAlive") == FALSE)
			{
				ret = -3;
				break;
			}
			tm = GetTickCount64();
		}
		auto status = Update();
		if (status == SessionUpdateResult::Enum::Succcess)
		{
			timeout = 0;
			continue;
		}
		else if (status == SessionUpdateResult::Enum::Exit)
		{
			ret = 0;
			break;
		}
		else if (status == SessionUpdateResult::Enum::Timeout)
		{
			if (++timeout >= TimeoutNgThreshold)
			{
				ret = -2;
				break;
			}
			continue;
		}
		else
		{
			ret = -1;
			PVL_DBG_PRINT(L"Iliegal update status(%d = 0x%08x)\n", status, status);
			//想定してないエラー
			break;
		}
	}
	isRunning_ = false;
	PVL_DBG_PRINT(L"Terminated session loop(%d).\n", ret);
	return ret;
}

/// <summary>
/// オーバーラップ構造体をプールから取得、ない場合は新規作成
/// </summary>
/// <param name="ol"></param>
OVERLAPPED* CSessionMessage::GetOverlapped()
{
	overlapPoolCs_.Enter();
	if (overlapPool_.empty())
	{
		overlapPoolCs_.Leave();
		auto ol = new OVERLAPPED();
		ZeroMemory(ol, sizeof(OVERLAPPED));
		ol->Internal = 0;
		ol->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		return ol;
	}
	else
	{
		auto ol = overlapPool_.back();
		overlapPool_.pop_back();
		overlapPoolCs_.Leave();
		auto evt = ol->hEvent;
		ZeroMemory(ol, sizeof(OVERLAPPED));
		ol->Internal = 0;
		ol->hEvent = evt;
		return ol;
	}
}

/// <summary>
/// オーバーラップ構造体をプールへ返却
/// </summary>
/// <param name="ol"></param>
void CSessionMessage::ReleaseOverlapped(OVERLAPPED* ol)
{
	ol->Internal = 0;
	ResetEvent(ol->hEvent);

	CCriticalSectionLock lock(overlapPoolCs_);
	overlapPool_.push_back(ol);
}

SessionUpdateResult CSessionMessage::Update()
{
	auto msg = ReadMessageBase();
	if (nullptr == msg->Cast<CoreSessionMessageHeader>())
	{
		Debug::Print(L"Read Message Bytes=%d msgType=%d\n", msg->bytes, msg->msgType);
	}
	//PVL_TRACE();
	if (msg)
	{
		return UpdateInner(msg);
	}
	return SessionUpdateResult::Enum::NoneMessage;
}

SessionUpdateResult CSessionMessage::UpdateInner(SessionMessageHeader* msg)
{
	//PVL_DBG_PRINT(L"CSessionMessage::UpdateInner\n");
	if (auto coreMsg = msg->Cast<CoreSessionMessageHeader>())
	{
		//PVL_DBG_PRINT(L"core\n");
		switch (coreMsg->message)
		{
		case SessionCoreMessage::None:
		case SessionCoreMessage::KeepAlive:
		case SessionCoreMessage::Timeout:
			return SessionUpdateResult::Enum::Succcess;
		}
	}
	//PVL_DBG_PRINT(L"invalid\n");
	return SessionUpdateResult::Enum::InvlidMessageType;
}

