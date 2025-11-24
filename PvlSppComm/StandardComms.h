#pragma once
namespace  PvlIpc
{
	class CInstanceSession;
	class CSpiInstanceSession;
	class CStandardSession;
	class Pipe;
}
namespace PvlSppComm
{
	ref class SpiInstanceComms;

	public delegate void ReciveNewSessionDelegate(SpiInstanceComms^ newInstance);

	public ref class StandardComms
	{
	public:
		StandardComms();
		~StandardComms();

		bool StartupServer();
		bool Terminate();

		bool SendNone();
		bool SendExit();
		bool SendNewSession();


		property bool IsRunning{ bool get() { return isRunnning_; } }

		void RecieveNewSession(PvlIpc::CInstanceSession*);
		event ReciveNewSessionDelegate^ OnReciveNewSession
		{
			void add(ReciveNewSessionDelegate^ h) {
				newSpiSessionoHandler_ = static_cast<ReciveNewSessionDelegate^>(System::Delegate::Combine(newSpiSessionoHandler_, h));
			}
			void remove(ReciveNewSessionDelegate^ h) {
				newSpiSessionoHandler_ = static_cast<ReciveNewSessionDelegate^>(System::Delegate::Remove(newSpiSessionoHandler_, h));
			}
		}


	private:
		HANDLE	process_ = NULL;
		bool	isRunnning_ = false;
		PvlIpc::Pipe* ioPipe_ = nullptr;
		PvlIpc::CStandardSession* session_ = nullptr;
		void* thisHandle_ = nullptr;

		ReciveNewSessionDelegate^ newSpiSessionoHandler_;
	};
}
