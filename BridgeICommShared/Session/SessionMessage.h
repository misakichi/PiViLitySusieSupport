#pragma once
#include <vector>
#include "CriticalSection.h"

namespace PvlIpc
{

	struct SessionMessageHeader;
	//struct StandardSessionMessageHeader;

	template<typename ENUM_TYPE>
	class EnumForResult
	{
	public:
		using Enum = ENUM_TYPE;
		using ThisType = EnumForResult<Enum>;
		EnumForResult() : value(static_cast<Enum>(0)) {}
		EnumForResult(Enum v) : value(v) {}
		EnumForResult(const ThisType& v) : value(static_cast<Enum>(v)) {}
		operator Enum() const { return value; }
		bool operator==(const Enum& rv) const { return value == rv; }
		bool operator!=(const Enum& rv) const { return value != rv; }
		bool operator<(const Enum& rv) const { return value < rv; }
		bool operator>(const Enum& rv) const { return value > rv; }
		bool IsSuccessed() const { return value == static_cast<Enum>(0); }
		bool IsFailed() const { return static_cast<int>(value) < 0; }

	private:
		Enum value;
	};

	enum class SessionUpdateResultType : int
	{
		Succcess = 0,
		Timeout,
		Exit,
		Unknown = (int)0x80000000,
		NoneMessage,
		InvlidMessageType,
	};
	using SessionUpdateResult = EnumForResult<SessionUpdateResultType>;
	class Pipe;

	/// <summary>
	/// IPCセッションメッセージ基底クラス
	/// </summary>
	class CSessionMessage
	{
	public:
		CSessionMessage();
		virtual ~CSessionMessage();
		void InitFromStdIo();
		void InitIo(HANDLE duplex, bool isOverlaped);
		void InitIo(HANDLE _read, HANDLE _write, bool isOverlaped);
		void SetEnableDebug(bool enable) { debug_ = true; }
		SessionUpdateResult Update();

	protected:
		SessionMessageHeader* ReadMessageBase();
		bool SendCommMessage(const SessionMessageHeader& msg, const char* msgName = "");


		OVERLAPPED* GetOverlapped();
		void ReleaseOverlapped(OVERLAPPED* ol);

		virtual SessionUpdateResult UpdateInner(SessionMessageHeader* msg);
		void DebugPrint(const wchar_t* format, ...);

		BOOL ReadSync(void* buffer, size_t bytes, LPDWORD lpReaded = nullptr);
		BOOL WriteSync(const void* buffer, size_t bytes, LPDWORD lpWrited = nullptr);

	protected:
		Pipe* pipe_ = nullptr;
		HANDLE	read_;
		HANDLE	write_;
		bool	isOverlapped_ = false;
		bool	debug_ = false;

		uint8_t* readBuffer_ = nullptr;
		size_t	readBufferSize_ = 0;
		size_t	readBufferOffset_ = 0;

		CCriticalSection overlapPoolCs_;
		std::vector<OVERLAPPED*> overlapPool_;
	};

} //PvlIpc
