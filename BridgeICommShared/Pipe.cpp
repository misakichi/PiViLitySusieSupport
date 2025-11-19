#include "stdafx.h"
#include "Pipe.h"
#include "stdio.h"
using namespace PvlIpc;



static PipeCreateOptions s_defaultPipeCreateOptions;
static PipeConnectOptions s_defaultPipeConnectOptions;

/// <summary>
/// MBS版名前付きパイプ作成コンストラクタ
/// </summary>
/// <param name="name"></param>
/// <param name="options"></param>
bool Pipe::Create(const char* name, const PipeCreateOptions* options)
{
	if(options ==nullptr)
	{
		options = &s_defaultPipeCreateOptions;
	}
	SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };
	isOverlaped_ = options->isOverlaped;
	int withOverlapped = isOverlaped_ ? FILE_FLAG_OVERLAPPED : 0;
	handle_ = CreateNamedPipeA(name, (int)options->direction | withOverlapped, PIPE_TYPE_MESSAGE, 2, options->outBufferSize, options->inBufferSize, options->defaultWaitTimeMs, &sa);
	return handle_ != INVALID_HANDLE_VALUE;
}

/// <summary>
/// Unicode版名前付きパイプ作成コンストラクタ
/// </summary>
/// <param name="name"></param>
/// <param name="options"></param>
bool Pipe::Create(const wchar_t* name, const PipeCreateOptions* options)
{
	if(options ==nullptr)
	{
		options = &s_defaultPipeCreateOptions;
	}
	SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };
	isOverlaped_ = options->isOverlaped;
	int withOverlapped = isOverlaped_ ? FILE_FLAG_OVERLAPPED : 0;
	handle_ = CreateNamedPipeW(name, (int)options->direction | withOverlapped, PIPE_TYPE_MESSAGE, 2, options->outBufferSize, options->inBufferSize, options->defaultWaitTimeMs, &sa);
	return handle_ != INVALID_HANDLE_VALUE;
}
bool Pipe::Connect(const char* name, const PipeConnectOptions* options)
{
	if (options == nullptr)
		options = &s_defaultPipeConnectOptions;

	int access = 0;
	access |= (options->direction == PipeDirection::In || options->direction == PipeDirection::InOut) ? GENERIC_READ : 0;
	access |= (options->direction == PipeDirection::Out || options->direction == PipeDirection::InOut) ? GENERIC_WRITE : 0;
	isOverlaped_ = options->isOverlaped;
	int flag = isOverlaped_ ? FILE_FLAG_OVERLAPPED : 0;
	auto pipe = CreateFileA(name, access, 0, NULL, OPEN_EXISTING, flag, NULL);
	return pipe != INVALID_HANDLE_VALUE;
}

bool Pipe::Connect(const wchar_t* name, const PipeConnectOptions* options)
{
	if (options == nullptr)
		options = &s_defaultPipeConnectOptions;

	int access = 0;
	access |= (options->direction == PipeDirection::In || options->direction == PipeDirection::InOut) ? GENERIC_READ : 0;
	access |= (options->direction == PipeDirection::Out || options->direction == PipeDirection::InOut) ? GENERIC_WRITE : 0;
	isOverlaped_ = options->isOverlaped;
	int flag = isOverlaped_ ? FILE_FLAG_OVERLAPPED : 0;
	auto pipe = CreateFileW(name, access, 0, NULL, OPEN_EXISTING, flag, NULL);
	return pipe != INVALID_HANDLE_VALUE;
}

/// <summary>
/// デストラクタ
/// </summary>
Pipe::~Pipe()
{
	if (INVALID_HANDLE_VALUE != handle_)
	{
		printf("Close pipe handle(0x%p)\n", handle_);
		CloseHandle(handle_);
	}
}

