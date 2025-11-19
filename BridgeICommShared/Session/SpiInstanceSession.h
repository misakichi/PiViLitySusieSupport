#pragma once
#include "InstanceSession.h"

namespace PvlIpc
{
	enum class SpiSessionMessage : int32_t
	{
		None = 10000,
		LoadPLugin = 10100,
	};

	using SpiSessionMessageHeader = SessionMessageHeaderBase<SpiSessionMessage, SessionMessageType::Instance, SpiSessionMessage::None>;
	using SpiSessionMessageNone = SpiSessionMessageHeader;

	struct SpiSessionMessageRequest : public SpiSessionMessageHeader
	{
		static constexpr SessionMessageType MsgType = SpiSessionMessageHeader::MsgType;
		static constexpr SpiSessionMessage Message = SpiSessionMessageHeader::Message;
		int requestId = 0;
	};

	struct SpiSessionMessageLoadPlugin : public SpiSessionMessageRequest
	{
		static constexpr SessionMessageType MsgType = SessionMessageType::Instance;
		static constexpr SpiSessionMessage Message = SpiSessionMessage::LoadPLugin;
		void Init()
		{
			bytes = sizeof(*this);
			msgType = MsgType;
			message = Message;
		}

		wchar_t pluginPath[1];
	};


    /// <summary>
	/// Susie Pluginインスタンスセッションクラス
    /// </summary>
    class CSpiInstanceSession : public CInstanceSession
    {
    public:
		using Base = CInstanceSession;
        CSpiInstanceSession();
		~CSpiInstanceSession();

		/// <summary>
		/// プラグインのロード（同期関数）
		/// </summary>
		/// <param name="pluginPath"></param>
		/// <returns>リクエストID</returns>
		int LoadPlugin(const wchar_t* pluginPath);

    protected:
		enum PluginStatus
		{
			None,
			Loading,
			Loaded,
			Failed,
		};
		SessionUpdateResult UpdateInner(SessionMessageHeader* msg) override;

		HMODULE pluginModule_ = nullptr;
		PluginStatus pluginStatus_ = PluginStatus::None;
		int requestIdGenerator_ = 1;

		HANDLE loadPluginEvent_ = INVALID_HANDLE_VALUE;
    };

}