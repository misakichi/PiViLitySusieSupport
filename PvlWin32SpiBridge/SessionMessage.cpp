#include "stdafx.h"
#include "SessionMessage.h"
#include "BridgeMessage.h"
#include "assert.h"
CSessionMessage::CSessionMessage()
	: in_(INVALID_HANDLE_VALUE)
	, out_(INVALID_HANDLE_VALUE)

{
}

CSessionMessage::~CSessionMessage()
{
}

void CSessionMessage::InitIo(HANDLE in, HANDLE out)
{
	in_ = in;
	out_ = out;

	
		//==FILE_TYPE_PIPE

}

/// <summary>
/// メッセージの読み込み
/// </summary>
/// <returns></returns>
SpiBridgeMessageHeader* CSessionMessage::ReadMessageBase()
{
	thread_local uint64_t buffer[256];//2048 bytes
	SpiBridgeMessageHeader* header = (SpiBridgeMessageHeader*)buffer;
	if( ReadFile(in_, header, sizeof(*header), nullptr, nullptr) == false)
	{
		return nullptr;
	}

	auto msgBodySize = header->bytes - sizeof(SpiBridgeMessageHeader);
	if (ReadFile(in_, ((int8_t*)header) + sizeof(*header), msgBodySize, nullptr, nullptr) == false)
	{
		return nullptr;
	}

	return (SpiBridgeMessageHeader*)header;

}