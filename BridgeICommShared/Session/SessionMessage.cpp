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
}

CSessionMessage::~CSessionMessage()
{
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


/// <summary>
/// メッセージの読み込み
/// </summary>
/// <returns></returns>
SpiBridgeMessageHeader* CSessionMessage::ReadMessageBase()
{
	thread_local uint64_t buffer[256];//2048 bytes
	SpiBridgeMessageHeader* header = (SpiBridgeMessageHeader*)buffer;

	if( ReadFile(read_, header, sizeof(*header), nullptr, nullptr) == false)
	{
		return nullptr;
	}

	auto msgBodySize = header->bytes - sizeof(SpiBridgeMessageHeader);
	if (ReadFile(read_, ((int8_t*)header) + sizeof(*header), (DWORD)msgBodySize, nullptr, nullptr) == false)
	{
		return nullptr;
	}
	
	if (debug_)
	{
		printf("Recive Message Type=%d, Bytes=%d\n", (int)header->msgType, header->bytes);
	}

	return (SpiBridgeMessageHeader*)header;

}

/// <summary>
/// 指定したメッセージを内部の書き込みハンドルへ送信する。isOverlapped_がtrueの場合はオーバーラップI/Oを使用して完了を待機し、falseの場合は同期的にWriteFileを呼び出す。debug_が有効であればメッセージ情報を出力する。
/// </summary>
/// <param name="msg">送信するメッセージのヘッダー。msg.bytesでバイト数が指定され、WriteFileにそのサイズで渡される。debug_が有効な場合はmsg.msgTypeとmsg.bytesがログ出力される。</param>
/// <returns>送信が成功した場合はtrue、失敗した場合はfalseを返すことを意図している。ただし、isOverlapped_がtrueのコードパスでは現行実装が戻り値を返さない可能性があり、未定義動作になる恐れがある。</returns>
bool CSessionMessage::SendCommMessage(const SpiBridgeMessageHeader& msg, const char* msgName)
{
	if (debug_) 
	{
		char buf[4096] = {};
		sprintf_s(buf, "Send Message Type=%d, Bytes=%d\n", (int)msg.msgType, msg.bytes);
		printf(buf);
		OutputDebugStringA(buf);
	}

	return WriteFile(write_, &msg, msg.bytes, nullptr, nullptr);
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
		ol->Internal = STATUS_PENDING;
		ol->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		return ol;
	}
	else
	{
		auto ol = overlapPool_.back();
		overlapPool_.pop_back();
		overlapPoolCs_.Leave();
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
		return UpdateInner((SpiBridgeStandardMessageHeader*)msg);
	}
	return SessionUpdateResult::NoneMessage;
}

SessionUpdateResult CSessionMessage::UpdateInner(SpiBridgeStandardMessageHeader* msg)
{
	return SessionUpdateResult::InvlidMessageType;
}


