#include "stdafx.h"

#include "BridgeMessage.h"
#include "StandardSession.h"
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


	const char*inPipeName = nullptr;
	const char* outPipeName = nullptr;
	int withOverlap = 0;
	for(auto arg : args)
	{
		constexpr auto prefixIn = "-i=";
		constexpr auto prefixOut = "-o=";
		constexpr auto prefixDbg = "-dbgConsole";
		constexpr auto prefixOverlap = "-is-overlap";
		if (strncmp(arg, prefixIn, strlen(prefixIn)) == 0)
		{
			inPipeName = arg + strlen(prefixIn);
			continue;
		}
		else if (strncmp(arg, prefixOut, strlen(prefixOut)) == 0)
		{
			outPipeName = arg + strlen(prefixOut);
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

	printf("inPipeName=%s\n", inPipeName ? inPipeName : "(null)");
	printf("outPipeName=%s\n", outPipeName ? outPipeName : "(null)");
	getchar(); // 入力待ち


	HANDLE inPipe = NULL;
	HANDLE outPipe = NULL;
	if (inPipeName && outPipeName)
	{
		printf("Connecting to pipes In=%s, Out=%s\n", inPipeName, outPipeName);
		inPipe = CreateFileA(
			inPipeName,
			GENERIC_READ,
			0,
			NULL,
			OPEN_EXISTING,
			withOverlap,
			NULL);
		auto inResult = GetLastError();

		outPipe = CreateFileA(
			outPipeName,
			GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			withOverlap,
			NULL);
		auto outResult = GetLastError();

		printf("Connecting to pipes(%s) \n In=0x%08x(0x%08x)\n Out=0x%08x(0x%08x)\n", withOverlap!=0 ? "overlaped" : "", inPipe, inResult, outPipe, outResult);
		getchar();

		session.InitIo(inPipe, outPipe, withOverlap != 0);
	}
	else
	{
		session.InitFromStdIo();
	}
	delete[] cmdLineStr;
	while (session.Update())
		;

	FreeConsole();

	//メッセージ切れによる終了は不正
	auto exitCode =  session.ExitCode();

	//fflush(stdout);
	//setvbuf(stdout, NULL, _IONBF, 0);
	//delete[]lbuf;
	return exitCode;
}
