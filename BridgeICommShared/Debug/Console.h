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

	/// <summary>
	/// GetLastError からエラーコードを取得し、コードが 0 でない場合は FormatMessageW でメッセージをフォーマットしてデバッグ出力に表示する。
	/// </summary>
	/// <returns>GetLastError から取得したエラーコード (DWORD)。エラーがなければ 0 を返す。</returns>
	extern DWORD PrintLastErrorMessage();
}
