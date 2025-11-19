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
	public ref class SpiInstanceComms
	{
	public:
		SpiInstanceComms(PvlIpc::CSpiInstanceSession* instance)
			: instance_(instance)
		{
		}

		System::Threading::Tasks::Task<bool>^ LoadPlugin(System::String^ pluginPath);

	private:
		bool LoadPluginInner(Object^ pluginPath);

		PvlIpc::CSpiInstanceSession* instance_ = nullptr;
	};
}