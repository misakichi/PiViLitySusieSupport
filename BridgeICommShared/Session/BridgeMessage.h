#pragma once
#include <type_traits>
#include <array>

#ifndef PVL_ARRAY_SIZE
#define PVL_ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

namespace PvlIpc
{
	using Name256Buffer = std::array<wchar_t, 256>;


	// {68803E18-6AB1-4C4E-8F71-F62C9006FB89}
	static constexpr GUID s_NamedPipeGuid =
	{ 0x68803e18, 0x6ab1, 0x4c4e, { 0x8f, 0x71, 0xf6, 0x2c, 0x90, 0x6, 0xfb, 0x89 } };

	constexpr size_t MessageSizeMax = 256;
	constexpr uint32_t PipeDefaultWait = 1000;
	/// <summary>
	/// 標準メッセージコマンドタイプ
	/// </summary>
	enum class SessionMessageType : int32_t
	{
		None = 0,
		Standard = 1,
		Instance = 2,
	};

	/// <summary>
	/// 基本やりとりメッセージ
	/// </summary>
#pragma pack(push, 4) 
	struct SessionMessageHeader
	{
		SessionMessageType msgType;
		int32_t bytes;

		template<typename T>
		T* Cast();
	};

	/// <summary>
	/// 標準メッセージコマンド
	/// </summary>
	enum class SessionStdMessage : int32_t
	{
		None = 0,

		// 新規セッション要求
		NewSession = 100,
		// 新規セッション応答
		CreatedNewSession = 110,
		// 新規セッション応答(失敗)
		FailedNewSession = 120,

		// セッション解放要求
		FreeSession = 200,

		// 終了要求
		Exit = 1000,
	};

	template<typename MesssageType, SessionMessageType thisMsgType, MesssageType thisMsg>
	struct SessionMessageHeaderBase : SessionMessageHeader
	{
		static constexpr SessionMessageType MsgType = thisMsgType;
		static constexpr MesssageType Message = thisMsg;
		void Init()
		{
			bytes = sizeof(*this);
			msgType = MsgType;
			message = Message;
		}
		MesssageType message;
	};

	/// <summary>
	/// 標準メッセージヘッダー
	/// </summary>
	using StandardSessionMessageHeader = SessionMessageHeaderBase<SessionStdMessage, SessionMessageType::Standard, SessionStdMessage::None>;
	//struct StandardSessionMessageHeader : SessionMessageHeader
	//{
	//	static constexpr SessionMessageType MsgType = SessionMessageType::Standard;
	//	static constexpr SessionStdMessage Message = SessionStdMessage::None;
	//	void Init()
	//	{
	//		bytes = sizeof(*this);
	//		msgType = MsgType;
	//		message = Message;
	//	}
	//	SessionStdMessage message;
	//};
	using StandardSessionMessageNone = StandardSessionMessageHeader;

	/// <summary>
	/// 新規セッション要求メッセージ
	/// </summary>
	struct StandardSessionMessageNewSession : StandardSessionMessageHeader
	{
		static constexpr SessionMessageType MsgType = SessionMessageType::Standard;
		static constexpr SessionStdMessage Message = SessionStdMessage::NewSession;
		void Init()
		{
			bytes = sizeof(*this);
			msgType = MsgType;
			message = Message;
		}
	};

	/// <summary>
	/// 新規作成通知メッセージ
	/// </summary>
	struct StandardSessionMessageResponseCreatedSession : StandardSessionMessageHeader
	{
		static constexpr SessionMessageType MsgType = SessionMessageType::Standard;
		static constexpr SessionStdMessage Message = SessionStdMessage::CreatedNewSession;
		void Init()
		{
			bytes = sizeof(*this);
			msgType = MsgType;
			message = Message;
		}
		int procId;
		uint32_t inId;
		uint32_t outId;
	};

	/// <summary>
	/// 新規失敗通史メッセージ
	/// </summary>
	struct StandardSessionMessageResponseFailedNewSession : StandardSessionMessageHeader
	{
		static constexpr SessionMessageType MsgType = SessionMessageType::Standard;
		static constexpr SessionStdMessage Message = SessionStdMessage::FailedNewSession;
		void Init()
		{
			bytes = sizeof(*this);
			msgType = MsgType;
			message = Message;
		}
		int32_t errorCode;
	};

	struct StandardSessionMessageFreeSession : StandardSessionMessageHeader
	{
		static constexpr SessionMessageType MsgType = SessionMessageType::Standard;
		static constexpr SessionStdMessage Message = SessionStdMessage::FreeSession;
		void Init()
		{
			bytes = sizeof(*this);
			msgType = MsgType;
			message = Message;
		}
		int32_t processId;
		int32_t sessionId;
	};

	struct StandardSessionMessageExit : StandardSessionMessageHeader
	{
		static constexpr SessionMessageType MsgType = SessionMessageType::Standard;
		static constexpr SessionStdMessage Message = SessionStdMessage::Exit;
		void Init()
		{
			bytes = sizeof(*this);
			msgType = MsgType;
			message = Message;
		}
	};
#pragma pack(pop) 

	template<typename T>
	inline T* SessionMessageHeader::Cast()
	{
		static_assert(sizeof(T) < MessageSizeMax, "Message Structure is over the tls buffer size.");
		auto tThis = reinterpret_cast<T*>(this);
		bool isOk = T::MsgType == tThis->msgType;
		if constexpr (!std::is_same_v < StandardSessionMessageHeader, T>)
		{
			isOk = T::Message == tThis->message ? isOk : false;
		}
		return isOk ? tThis : nullptr;
	}


}	//PvlIpc

