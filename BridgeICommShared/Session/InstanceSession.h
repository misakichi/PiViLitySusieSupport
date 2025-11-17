#pragma once
#include "SessionMessage.h"
#include "BridgeMessage.h"


namespace PvlIpc
{


	class CInstanceSession : public CSessionMessage
	{
	public:
		CInstanceSession();
		~CInstanceSession();

		bool InitServer(const wchar_t* inName, const wchar_t* outName, bool isOverlaped);
		bool InitClient(const wchar_t* inName, const wchar_t* outName, bool isOverlaped);

		bool Update();

	protected:
		void Terminate();
		
	private:
		static constexpr size_t OverlapHandles = 2;
		HANDLE		overlapEvent_[OverlapHandles];
		OVERLAPPED	overlap_[OverlapHandles];
	};

}

