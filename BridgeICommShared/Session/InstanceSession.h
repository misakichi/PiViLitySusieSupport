#pragma once

#include "SessionMessage.h"
#include "BridgeMessage.h"


namespace PvlIpc
{

	/// <summary>
	/// インスタンスセッションクラス
	/// </summary>
	class CInstanceSession : public CSessionMessage
	{
	public:
		using Base = CSessionMessage;
		CInstanceSession();
		virtual ~CInstanceSession() {}

		bool InitServer(const wchar_t* name, bool isOverlaped);
		bool InitClient(const wchar_t* name, bool isOverlaped);
		bool IsRunning() { return running_; }
	protected:
		void Terminate();
		void StartProcessThread();
		SessionUpdateResult UpdateInner(SessionMessageHeader* msg) override;

	private:
		static constexpr size_t OverlapHandles = 2;
		HANDLE		overlapEvent_[OverlapHandles];
		OVERLAPPED	overlap_[OverlapHandles];
		bool running_ = false;
	};

}

