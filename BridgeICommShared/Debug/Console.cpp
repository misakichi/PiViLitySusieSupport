#include "stdafx.h"
#include "Console.h"
#include <stdio.h>

#pragma unmanaged
bool Debug::g_useDebug= false;

/// <summary>
/// 可変引数付きデバッグ出力。g_useDebug が true の場合にのみ動作する。
/// </summary>
/// <param name="string"></param>
/// <param name=""></param>
void Debug::Print(const wchar_t* string, ...)
{
	if (g_useDebug)
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

/// <summary>
/// GetLastError からエラーコードを取得し、コードが 0 でない場合は FormatMessageW でメッセージをフォーマットしてデバッグ出力に表示する。
/// </summary>
/// <returns>GetLastError から取得したエラーコード (DWORD)。エラーがなければ 0 を返す。</returns>
DWORD Debug::PrintLastErrorMessage()
{
	auto le = GetLastError();
	if (le != 0)
	{
		wchar_t* msgBuf = nullptr;
		FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			le,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&msgBuf,
			0,
			NULL);

		Print(L"LastError(%d) : %s\n", le, msgBuf ? msgBuf : L"Unknown error");
		if (msgBuf)
			LocalFree(msgBuf);
	}
	return le;
}

#pragma managed