#include "stdafx.h"
#include "Pipe.h"

using namespace PvlIpc;


Pipe::Pipe()
{
	SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };
	CreatePipe(&read_, &write_, &sa, 0);
}
Pipe::Pipe(const char* readName, const char* writeName, bool isOverlaped)
{
	SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };
	isOverlaped_ = isOverlaped;
	int withOverlapped = isOverlaped ? FILE_FLAG_OVERLAPPED : 0;
	read_ = CreateNamedPipeA(readName, PIPE_ACCESS_INBOUND | withOverlapped, 0, 2, 4096, 4096, 0, &sa);
	write_ = CreateNamedPipeA(writeName, PIPE_ACCESS_OUTBOUND | withOverlapped, 0, 2, 4096, 4096, 0, &sa);
}
Pipe::Pipe(const wchar_t* readName, const wchar_t* writeName, bool isOverlaped)
{
	SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };
	isOverlaped_ = isOverlaped;
	int withOverlapped = isOverlaped ? FILE_FLAG_OVERLAPPED : 0;
	read_ = CreateNamedPipeW(readName, PIPE_ACCESS_INBOUND | withOverlapped, 0, 2, 4096, 4096, 0, &sa);
	write_ = CreateNamedPipeW(writeName, PIPE_ACCESS_OUTBOUND | withOverlapped, 0, 2, 4096, 4096, 0, &sa);
}

Pipe::~Pipe()
{
	CloseHandle(read_);
	CloseHandle(write_);
}

