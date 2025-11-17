#pragma once

namespace PvlIpc
{
	class Pipe
	{
	public:
		Pipe();
		Pipe(const char* readName, const char* writeName, bool isOverlaped);
		Pipe(const wchar_t* readName, const wchar_t* writeName, bool isOverlaped);
		~Pipe();
		HANDLE write_;
		HANDLE read_;
		bool isOverlaped_ = false;
	};
}