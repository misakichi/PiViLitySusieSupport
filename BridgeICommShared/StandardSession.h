#pragma once
#include "SessionMessage.h"
#include "BridgeMessage.h"

namespace PvlIpc
{
	class CInstanceSession;
	using NewSessionResultCallback = void(*)(CInstanceSession* newClientSession, void* arg);

	/// <summary>
	/// IPCPメインセッション用メッセージ処理クラス
	/// </summary>
	class CStandardSession : public CSessionMessage
	{
	public:
		CStandardSession();
		~CStandardSession();
		SpiBridgeStandardMessageHeader* ReadMessage();
		bool Update();
		int ExitCode() const;

		/// <summary>
		/// 無意味なメッセージ送信
		/// </summary>
		/// <returns></returns>
		bool SendNone();

		/// <summary>
		/// 終了メッセージ送信
		/// </summary>
		/// <returns></returns>
		bool SendExit();

		/// <summary>
		/// 新規インスタンスセッション作成要求メッセージ送信
		/// </summary>
		/// <returns></returns>
		bool SendNewSession();

		/// <summary>
		/// インスタンスセッション作成応答メッセージ送信
		/// </summary>
		/// <param name="_ProcId"></param>
		/// <param name="_InPipeId"></param>
		/// <param name="_OutPipeId"></param>
		/// <returns></returns>
		bool SendResponseCreatedSession(int _ProcId, uint32_t _InPipeId, uint32_t _OutPipeId);


		/// <summary>
		/// インスタンスセッション応答受信コールバックの設定
		/// </summary>
		/// <param name="callback"></param>
		void SetNewSessionResultCallback(NewSessionResultCallback callback, void* p)
		{
			newSessionResultCallback_ = callback;
			newSessionResultCallbackArg_ = p;
		}

	private:
		static DWORD __stdcall SessionThreadEntry(void* p);

		bool CraetePipeName(int procId, int _SessionId, const wchar_t* _SuffixStr, Name256Buffer& _Name);
		int exitCode_ = -1;

		//１つのスレッドでしか変化しないのでatomicである必要はない
		uint32_t sessionId_ = 0;

		//セッション生成応答のコールバック
		NewSessionResultCallback	newSessionResultCallback_ = nullptr;
		void*						newSessionResultCallbackArg_ = nullptr;
	};

} //PvlIpc
