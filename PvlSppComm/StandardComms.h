#pragma once

class CStandardSession;

namespace PvlSppComm
{
	class Pipe
	{
	public:
		Pipe();
		~Pipe();
		HANDLE write_;
		HANDLE read_;
	};

	public ref class StandardComms 
	{
	public:
		StandardComms();
		bool Initialize();
		bool Terminate();

		bool SendNone();
		bool SendExit();

		property bool IsRunning { bool get() { return isRunnning_; } }
	private:
		Pipe*	pipeErr_ = NULL;
		Pipe*	pipeIn_ = NULL;
		Pipe*	pipeOut_ = NULL;
		HANDLE	process_ = NULL;
		bool	isRunnning_ = false;
		CStandardSession* session_ = nullptr;
	};


}