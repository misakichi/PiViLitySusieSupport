#include "stdafx.h"
#include "SessionMessage.h"
#include "BridgeMessage.h"
#include "assert.h"
#include <string>
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

void CSessionMessage::InitIo(HANDLE duplex, bool isOverlaped)
{
	InitIo(duplex, duplex, isOverlaped);
}


void CSessionMessage::InitIo(HANDLE _read, HANDLE _write, bool isOverlaped)
{
	read_ = _read;
	write_ = _write;
	isOverlapped_ = isOverlaped;
}

void CSessionMessage::InitFromStdIo()
{
	auto stdIn = GetStdHandle(STD_INPUT_HANDLE);
	auto stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	InitIo(stdIn, stdOut, false);
}

BOOL CSessionMessage::ReadSync(void* buffer, size_t bytes, LPDWORD lpReaded)
{
	if (isOverlapped_)
	{
		OVERLAPPED* ol = GetOverlapped();
		if (!ReadFile(read_, buffer, (DWORD)bytes, nullptr, ol))
		{
			auto le = GetLastError();
			if (le != ERROR_IO_PENDING)
			{
				DebugPrint(L"ReadSync Failed. LastError=0x%08x\n", le);
				return FALSE;
			}
		}
		DWORD readed = 0;
		auto result = GetOverlappedResult(read_, ol, &readed, TRUE);
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
		OVERLAPPED* ol = GetOverlapped();
		if (!WriteFile(write_, buffer, (DWORD)bytes, nullptr, ol))
		{
			auto le = GetLastError();
			if (le != ERROR_IO_PENDING)
			{
				DebugPrint(L"WriteSync Failed. LastError=0x%08x\n", le);
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

	DebugPrint(L"Read Message Body Bytes=%d\n", msgBodySize);

	return (SessionMessageHeader*)header;

}

/// <summary>
/// 指定したメッセージを内部の書き込みハンドルへ送信する。isOverlapped_がtrueの場合はオーバーラップI/Oを使用して完了を待機し、falseの場合は同期的にWriteFileを呼び出す。debug_が有効であればメッセージ情報を出力する。
/// </summary>
/// <param name="msg">送信するメッセージのヘッダー。msg.bytesでバイト数が指定され、WriteFileにそのサイズで渡される。debug_が有効な場合はmsg.msgTypeとmsg.bytesがログ出力される。</param>
/// <returns>送信が成功した場合はtrue、失敗した場合はfalseを返すことを意図している。ただし、isOverlapped_がtrueのコードパスでは現行実装が戻り値を返さない可能性があり、未定義動作になる恐れがある。</returns>
bool CSessionMessage::SendCommMessage(const SessionMessageHeader& msg, const char* msgName)
{
	DebugPrint(L"Send Message: %S, Type=%d, Bytes=%d\n", msgName, (int)msg.msgType, msg.bytes);

	return WriteSync(&msg, msg.bytes);
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
	if (msg)
	{
		return UpdateInner(msg);
	}
	return SessionUpdateResult::Enum::NoneMessage;
}

SessionUpdateResult CSessionMessage::UpdateInner(SessionMessageHeader* msg)
{
	return SessionUpdateResult::Enum::InvlidMessageType;
}


void CSessionMessage::DebugPrint(const wchar_t* string, ...)
{
	if (debug_)
	{
		wchar_t buffer[4096];

		va_list args;
		va_start(args, string);
		vwprintf(string, args);
		_vsnwprintf_s(buffer, _countof(buffer), _TRUNCATE, string, args);
		va_end(args);
		OutputDebugStringW(buffer);
	}
}
