#include "stdafx.h"
#include "Console.h"
#include <stdio.h>

#pragma unmanaged
#if defined(_DEBUG) || defined(DEBUG)
bool Debug::g_useDebug = true;
#else
bool Debug::g_useDebug= false;
#endif

namespace Debug
{
	void Print(const wchar_t* string, va_list args)
	{
		if (g_useDebug)
		{
			wchar_t buffer[4096];
			vwprintf(string, args);
			_vsnwprintf_s(buffer, _countof(buffer), _TRUNCATE, string, args);
			OutputDebugStringW(buffer);
		}
	}
	void PrintA(const char* string, va_list args)
	{
		if (g_useDebug)
		{
			char buffer[4096];
			vprintf(string, args);
			vsprintf_s(buffer, _countof(buffer), string, args);
			OutputDebugStringA(buffer);
		}
	}
}
/// <summary>
/// 可変引数付きデバッグ出力。g_useDebug が true の場合にのみ動作する。
/// </summary>
/// <param name="string"></param>
/// <param name=""></param>
void Debug::Print(const wchar_t* string, ...)
{
	if (g_useDebug)
	{
		va_list args;
		va_start(args, string);
		Print(string, args);
		va_end(args);
	}
}
void Debug::PrintA(const char* string, ...)
{
	if (g_useDebug)
	{
		va_list args;
		va_start(args, string);
		PrintA(string, args);
		va_end(args);
	}
}

/// <summary>
/// GetLastError からエラーコードを取得し、コードが 0 でない場合は FormatMessageW でメッセージをフォーマットしてデバッグ出力に表示する。
/// </summary>
/// <returns>GetLastError から取得したエラーコード (DWORD)。エラーがなければ 0 を返す。</returns>
DWORD Debug::PrintLastErrorMessage(DWORD le, const wchar_t* string, ...)
{
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

		Print(L"LastError(%d) : %s", le, msgBuf ? msgBuf : L"Unknown error\n");
		if(g_useDebug && string)
		{
			va_list args;
			va_start(args, string);
			Print(string, args);
			va_end(args);
		}
		if (msgBuf)
			LocalFree(msgBuf);
	}
	return le;
}

/// <summary>
/// 
/// </summary>
/// <param name="string"></param>
/// <param name=""></param>
/// <returns></returns>
DWORD Debug::PrintLastErrorMessageA(DWORD le, const char* string, ...)
{
	if (le != 0)
	{
		char* msgBuf = nullptr;
		FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			le,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&msgBuf,
			0,
			NULL);

		PrintA("LastError(%d) : %s", le, msgBuf ? msgBuf : "Unknown error\n");
		if (g_useDebug && string)
		{
			va_list args;
			va_start(args, string);
			PrintA(string, args);
			va_end(args);
		}
		if (msgBuf)
			LocalFree(msgBuf);
	}
	return le;
}

#pragma managed