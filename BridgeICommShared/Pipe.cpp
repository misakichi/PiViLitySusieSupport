#include "stdafx.h"
#include "Debug/Console.h"
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
	handle_ = PVL_CHECK_HANDLE(CreateNamedPipeA(name, (int)options->direction | withOverlapped, PIPE_TYPE_MESSAGE, 2, options->outBufferSize, options->inBufferSize, options->defaultWaitTimeMs, &sa));
	

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
	handle_ = PVL_CHECK_HANDLE(CreateNamedPipeW(name, (int)options->direction | withOverlapped, PIPE_TYPE_MESSAGE, 2, options->outBufferSize, options->inBufferSize, options->defaultWaitTimeMs, &sa));

	PVL_DBG_PRINT(
		L"CreateNamedPipeW\n"
		L" name:%s\n"
		L" open mode:%08x\n"
		L" pipe mode:%d\n"
		L" max instances:%d\n"
		L" out buffer size:%d\n"
		L" in buffer size:%d\n"
		L" default time out ms:%d\n"
		L" sa{cb=%d, lp=%p, bInheritHandle=%d\n\n"
		, name
		, (int)options->direction | withOverlapped
		, PIPE_TYPE_MESSAGE, 2
		, options->outBufferSize, options->inBufferSize, options->defaultWaitTimeMs,
		sa.nLength, sa.lpSecurityDescriptor, sa.bInheritHandle
	);

	return handle_ != INVALID_HANDLE_VALUE;
}

/// <summary>
/// 指定した名前付きパイプに接続する。オプションに基づきアクセス権とオーバーラップ I/O フラグを設定して CreateFileA を呼び出す。
/// </summary>
/// <param name="name">接続する名前付きパイプの名前（null終端のC文字列）。</param>
/// <param name="options">接続オプションへのポインタ。nullptrの場合は内部の既定オプションが使用される。options->directionによりGENERIC_READ/GENERIC_WRITEが決まり、options->isOverlapedはisOverlaped_メンバに設定される。</param>
/// <returns>接続に成功して有効なハンドルが得られればtrue、INVALID_HANDLE_VALUEが返された場合はfalse（接続失敗）。</returns>
bool Pipe::Connect(const char* name, const PipeConnectOptions* options)
{
	if (options == nullptr)
		options = &s_defaultPipeConnectOptions;

	PVL_CHECK_BOOL(WaitNamedPipeA(name, 5000));
	int access = 0;
	access |= (options->direction == PipeDirection::In || options->direction == PipeDirection::InOut) ? GENERIC_READ : 0;
	access |= (options->direction == PipeDirection::Out || options->direction == PipeDirection::InOut) ? GENERIC_WRITE : 0;
	isOverlaped_ = options->isOverlaped;
	int flag = isOverlaped_ ? FILE_FLAG_OVERLAPPED : 0;
	handle_ = PVL_CHECK_HANDLE(CreateFileA(name, access, 0, NULL, OPEN_EXISTING, flag, NULL));
	return handle_ != INVALID_HANDLE_VALUE;
}

/// <summary>
/// 指定した名前付きパイプに接続する。オプションに基づきアクセス権とオーバーラップ I/O フラグを設定して CreateFileW を呼び出す。
/// </summary>
/// <param name="name">接続先のパイプ名（ワイド文字列）。CreateFileW に渡されるパス。</param>
/// <param name="options">接続オプションへのポインタ。nullptr の場合は s_defaultPipeConnectOptions が使用される。options->direction により読み取り/書き込みアクセスが決まり、options->isOverlaped の値が内部メンバ isOverlaped_ に設定される。</param>
/// <returns>接続に成功した場合は true を返す（CreateFileW が INVALID_HANDLE_VALUE 以外のハンドルを返した場合）。失敗した場合は false を返す。</returns>
bool Pipe::Connect(const wchar_t* name, const PipeConnectOptions* options)
{
	if (options == nullptr)
		options = &s_defaultPipeConnectOptions;

	PVL_CHECK_BOOL(WaitNamedPipeW(name, 5000));
	int access = 0;
	access |= (options->direction == PipeDirection::In || options->direction == PipeDirection::InOut) ? GENERIC_READ : 0;
	access |= (options->direction == PipeDirection::Out || options->direction == PipeDirection::InOut) ? GENERIC_WRITE : 0;
	isOverlaped_ = options->isOverlaped;
	int flag = isOverlaped_ ? FILE_FLAG_OVERLAPPED : 0;
	handle_ = PVL_CHECK_HANDLE(CreateFileW(name, access, 0, NULL, OPEN_EXISTING, flag, NULL));
	return handle_ != INVALID_HANDLE_VALUE;
}

/// <summary>
/// デストラクタ
/// </summary>
Pipe::~Pipe()
{
	if (INVALID_HANDLE_VALUE != handle_)
	{
		Debug::PrintA("Close pipe handle(0x%p)\n", handle_);
		CloseHandle(handle_);
	}
}

