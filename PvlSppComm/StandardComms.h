#pragma once
namespace  PvlIpc
{
	class CInstanceSession;
	class CStandardSession;
	class Pipe;
}
namespace PvlSppComm
{
	public ref class StandardComms
	{
	public:
		StandardComms();
		bool Initialize();
		bool Terminate();

		bool SendNone();
		bool SendExit();
		bool SendNewSession();


		property bool IsRunning{ bool get() { return isRunnning_; } }

		void RecieveNewSession(PvlIpc::CInstanceSession*);

	private:
		PvlIpc::Pipe* ioPipe_ = NULL;
		HANDLE	process_ = NULL;
		bool	isRunnning_ = false;
		PvlIpc::CStandardSession* session_ = nullptr;
	};
}
