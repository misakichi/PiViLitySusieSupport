#pragma once

namespace Debug
{
	extern bool g_useDebug;


	/// <summary>
	/// 可変引数付きデバッグ出力。g_useDebug が true の場合にのみ動作する。
	/// </summary>
	/// <param name="string"></param>
	/// <param name=""></param>	
	extern void Print(const wchar_t* string, ...);
	extern void PrintA(const char* string, ...);

	/// <summary>
	/// GetLastError からエラーコードを取得し、コードが 0 でない場合は FormatMessageW でメッセージをフォーマットしてデバッグ出力に表示する。
	/// </summary>
	/// <returns>GetLastError から取得したエラーコード (DWORD)。エラーがなければ 0 を返す。</returns>
	extern DWORD PrintLastErrorMessage(DWORD le, const wchar_t* string = nullptr, ...);
	extern DWORD PrintLastErrorMessageA(DWORD le, const char* string = nullptr, ...);
	inline DWORD PrintLastErrorMessage() { return PrintLastErrorMessage(GetLastError()); }

	inline bool boolFuncCheck(bool expr, const char* exprStr, const char* file, int line, const char* func)
	{
		auto le = GetLastError();
		if (!expr)
		{
			::Debug::PrintLastErrorMessageA(le, "Check failed: %s on %s(%d) : %s\n", exprStr, file, line, func);
			return false;
		}
		else
		{
			::Debug::PrintA("Check OK: %s on %s(%d) : %s\n", exprStr, file, line, func);
		}
		return true;
	}
	inline HRESULT hrFuncCheck(HRESULT hr, const char* exprStr, const char* file, int line, const char* func)
	{
		auto le = GetLastError();
		if (FAILED(hr))
		{
			::Debug::PrintLastErrorMessageA(le, "Check failed: %s on %s(%d) : %s\n", exprStr, file, line, func);
		}
		else
		{
			::Debug::PrintA("Check OK: %s. result:0x%08x on %s(%d) : %s\n", exprStr, hr, file, line, func);
		}
		return hr;
	}
	inline HANDLE handleFuncCheck(HANDLE handle, const char* handleStr, const char* file, int line, const char* func)
	{
		auto le = GetLastError();
		if (handle == INVALID_HANDLE_VALUE)
		{
			::Debug::PrintLastErrorMessageA(le, "Check failed: %s on %s(%d) : %s\n", handleStr, file, line, func);
		}
		else
		{
			::Debug::PrintA("Check OK: %s. result:0x%08x  on %s(%d) : %s\n", handleStr, handle, file, line, func);
		}
		return handle;
	}
}

#define PVL_TRACE()			::Debug::PrintA("[Trace] %s(%d)\n", __FILE__, __LINE__)
#define PVL_DBG_PRINT(...)		::Debug::Print(__VA_ARGS__);
#define PVL_DBG_PRINT_A(...)	::Debug::PrintA(__VA_ARGS__);
#define PVL_DBG_CHECK_ERROR()	::Debug::PrintLastErrorMessageA("on %s(%d) : %s", __FILE__, __LINE__, __func__);
#define PVL_CHECK_BOOL(expr)  ::Debug::boolFuncCheck((expr), #expr, __FILE__, __LINE__, __func__)
#define PVL_CHECK_HR(hr)  ::Debug::hrFuncCheck((hr), #hr, __FILE__, __LINE__, __func__)
#define PVL_CHECK_HANDLE(handle)  ::Debug::handleFuncCheck((handle), #handle, __FILE__, __LINE__, __func__)