#pragma once

namespace PvlIpc
{
	enum class PipeDirection : int
	{
		In = PIPE_ACCESS_INBOUND,
		Out = PIPE_ACCESS_OUTBOUND,
		InOut = PIPE_ACCESS_DUPLEX,
	};
	/// <summary>
	/// 名前付きパイプ作成時のオプションを表す構造体。入出力バッファサイズ、オーバーラップモード、デフォルトの待機時間を指定します。
	/// </summary>
	struct PipeCreateOptions
	{
		DWORD outBufferSize = 4096;
		DWORD inBufferSize = 4096;
		bool isOverlaped = false;
		DWORD defaultWaitTimeMs = 0;
		PipeDirection direction = PipeDirection::InOut;
	};

	struct PipeConnectOptions
	{
		bool isOverlaped = false;
		PipeDirection direction = PipeDirection::InOut;
	};

	class Pipe
	{
	public:
		bool Create(const char* name, const PipeCreateOptions* options = nullptr);
		bool Create(const wchar_t* name, const PipeCreateOptions* options = nullptr);
		bool Connect(const char* name, const PipeConnectOptions* options= nullptr);
		bool Connect(const wchar_t* name, const PipeConnectOptions* options = nullptr);
		~Pipe();

		bool IsOverlaped() const { return isOverlaped_; }
		operator HANDLE() const { return handle_; }

	private:
		HANDLE handle_ = INVALID_HANDLE_VALUE;
		bool isOverlaped_ = false;
	};
}