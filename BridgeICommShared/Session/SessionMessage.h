#pragma once
#include <vector>
#include "CriticalSection.h"

namespace PvlIpc
{

	struct SpiBridgeMessageHeader;
	struct SpiBridgeStandardMessageHeader;

	enum class SessionUpdateResult : int
	{
		Succcess = 0,
		Exit,
		Unknown = (int)0x80000000,
		NoneMessage,
		InvlidMessageType,
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
		SessionUpdateResult Update();

	protected:
		SpiBridgeMessageHeader* ReadMessageBase();
		bool SendCommMessage(const SpiBridgeMessageHeader& msg, const char* msgName = "");


		OVERLAPPED* GetOverlapped();
		void ReleaseOverlapped(OVERLAPPED* ol);

		virtual SessionUpdateResult UpdateInner(SpiBridgeStandardMessageHeader* msg);

	protected:
		HANDLE	read_;
		HANDLE	write_;
		bool	isOverlapped_ = false;
		bool	debug_ = false;

		CCriticalSection overlapPoolCs_;
		std::vector<OVERLAPPED*> overlapPool_;
	};

} //PvlIpc
