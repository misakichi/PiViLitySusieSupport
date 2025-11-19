#include "stdafx.h"

#include "Session/BridgeMessage.h"
#include "Session/StandardSession.h"
#include <vector>
#include <string>
#include <stdio.h>

int WINAPI WinMain(
	_In_ HINSTANCE hInstance, 
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ LPSTR lpCmdLine, 
	_In_ int nShowCmd)
{
	PvlIpc::CStandardSession session;

	//コマンドライン引数にパイプ指定がないかチェック
	std::vector<const char*> args;
	auto cmdLineLength = strlen(lpCmdLine);
	auto cmdLineStr = new char[cmdLineLength + 2];
	auto pStr = cmdLineStr;
	strcpy_s(pStr, cmdLineLength + 1, lpCmdLine);
	pStr[cmdLineLength] = ' ';
	pStr[cmdLineLength + 1] = '\0';
	while (auto next = strchr(pStr, ' '))
	{
		*next = '\0';
		if (*pStr != '\0')
			args.push_back(pStr);
		pStr = next + 1;
	}

	HANDLE stdIn = INVALID_HANDLE_VALUE;
	HANDLE stdOut = INVALID_HANDLE_VALUE;
	ALLOC_CONSOLE_OPTIONS options = {};
	options.mode = ALLOC_CONSOLE_MODE_NEW_WINDOW;
	AllocConsoleWithOptions(&options, nullptr);
	stdIn = GetStdHandle(STD_INPUT_HANDLE);
	stdOut = GetStdHandle(STD_OUTPUT_HANDLE);


	// 標準入出力をコンソールに再関連付け
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);  // printf 出力用
	freopen_s(&fp, "CONOUT$", "w", stderr);  // fprintf(stderr, ...) 用
	freopen_s(&fp, "CONIN$", "r", stdin);   // scanf 入力用
	//constexpr size_t lineBufSize = 32 * 1024;
	//char* lbuf = new char[lineBufSize];
	//ZeroMemory(lbuf, lineBufSize);
	//setvbuf(stdout, lbuf, _IOLBF, lineBufSize);
	setvbuf(stdout, NULL, _IONBF, 0);

	// printf が使える
	printf("Hello from printf!\n");
	printf("Input something: ");


	const char* pipeName = nullptr;
	int withOverlap = 0;
	for(auto arg : args)
	{
		constexpr auto prefixPipe = "-pipe=";
		constexpr auto prefixDbg = "-dbgConsole";
		constexpr auto prefixOverlap = "-is-overlap";
		if (strncmp(arg, prefixPipe, strlen(prefixPipe)) == 0)
		{
			pipeName = arg + strlen(prefixPipe);
			continue;
		}
		else if (strncmp(arg, prefixDbg, strlen(prefixDbg)) == 0)
		{
			continue;
		}
		else if (strncmp(arg, prefixOverlap, strlen(prefixOverlap)) == 0)
		{
			withOverlap = FILE_FLAG_OVERLAPPED;
			continue;
		}
	}

	printf("PipeName=%s\n", pipeName ? pipeName : "(null)");


	HANDLE pipeIo = NULL;
	if (pipeName)
	{
		printf("Connecting to pipe %s\n", pipeName);
		pipeIo = CreateFileA(
			pipeName,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			withOverlap,
			NULL);
		auto result = GetLastError();

		printf("Connecting to pipes(%s) \n 0x%p(0x%08x)\n", withOverlap!=0 ? "overlaped" : "", pipeIo, result);

		session.InitIo(pipeIo, pipeIo, withOverlap != 0);
	}
	else
	{
		session.InitFromStdIo();
	}
	delete[] cmdLineStr;
	while (session.Update()==PvlIpc::SessionUpdateResult::Enum::Succcess)
		;

	FreeConsole();

	//メッセージ切れによる終了は不正
	auto exitCode =  session.ExitCode();

	return exitCode;
}
