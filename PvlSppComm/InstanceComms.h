#pragma once

namespace  PvlIpc
{
	class CStandardSession;
}

namespace PvlSppComm
{
	ref class InstanceComms
	{
	public:
		InstanceComms(PvlIpc::CStandardSession* session);
		bool Initialize();
		bool Terminate();

	private:
		PvlIpc::CStandardSession* session_ = nullptr;
	};

}