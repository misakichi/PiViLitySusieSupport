#pragma once
#include <vector>

namespace PvlIpc
{

	struct SpiBridgeMessageHeader;

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


	class CCriticalSection
	{
	public:
		CCriticalSection();
		~CCriticalSection();
		void Enter();
		void Leave();

		//std::lock_guard用
		inline void lock() { Enter(); }
		inline void unlock() { Leave(); }
	private:
		CRITICAL_SECTION cs_;
	};

	class CCriticalSectionLock
	{
	public:
		CCriticalSectionLock(CCriticalSection& cs) : cs_(cs)
		{
			cs_.Enter();
		}
		~CCriticalSectionLock()
		{
			cs_.Leave();
		}
	private:
		CCriticalSection& cs_;
	};



	/// <summary>
	/// IPCセッションメッセージ基底クラス
	/// </summary>
	class CSessionMessage
	{
	public:
		CSessionMessage();
		~CSessionMessage();
		void InitFromStdIo();
		void InitIo(HANDLE _read, HANDLE _write, bool isOverlaped);
		void SetEnableDebug(bool enable) { debug_ = true; }

	protected:
		SpiBridgeMessageHeader* ReadMessageBase();
		bool SendCommMessage(const SpiBridgeMessageHeader& msg, const char* msgName = "");


		OVERLAPPED* GetOverlapped();
		void ReleaseOverlapped(OVERLAPPED* ol);

	protected:
		HANDLE	read_;
		HANDLE	write_;
		bool	isOverlapped_ = false;
		bool	debug_ = false;

		CCriticalSection overlapPoolCs_;
		std::vector<OVERLAPPED*> overlapPool_;
	};

} //PvlIpc
