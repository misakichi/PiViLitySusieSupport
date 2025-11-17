#include "stdafx.h"
#include "StandardSession.h"
#include "InstanceSession.h"

#pragma comment(lib, "Ole32.lib")

using namespace PvlIpc;

CStandardSession::CStandardSession()
{
}

CStandardSession::~CStandardSession()
{
}

/// <summary>
/// メッセージの読み取り
/// </summary>
/// <returns></returns>
SpiBridgeStandardMessageHeader* CStandardSession::ReadMessage()
{
	auto msg = ReadMessageBase();
	return msg ? msg->Cast<SpiBridgeStandardMessageHeader>() : nullptr;
}

struct SessionInfo
{
	HANDLE InPipe;
	HANDLE OutPipe;
};
DWORD __stdcall CStandardSession::SessionThreadEntry(void* p)
{
	auto instance = ((CInstanceSession*)p);
	while (instance->Update())
	{
	}
	delete instance;

	return 0;
}

/// <summary>
/// update session.
/// read message and process it.
/// </summary>
/// <returns>
/// true: continue, false: exit
/// </returns>
bool CStandardSession::Update()
{
	auto msg = ReadMessage();

	if (msg->message == SpiBridgeStandardMessageHeader::Message)
	{
		return true;
	}
	else if (auto newSessionMsg = msg->Cast<SpiBridgeStandardMessageNewSession>())
	{
		//新規セッション
		auto inId = sessionId_++;
		auto outId = sessionId_++;
		auto procId = GetCurrentProcessId();
		Name256Buffer inName, outName;
		bool named = CraetePipeName(procId, inId, L"_InBridgePipe", inName);
		named &= CraetePipeName(procId, outId, L"_InBridgePipe", outName);
		if (named)
		{
			auto instance = new CInstanceSession();
			if(instance->InitServer(inName.data(), outName.data(), true))
			{
				//サーバーは用意ができたので、処理スレッドを立ち上げて成功を返す
				CreateThread(NULL, 0, SessionThreadEntry, instance, 0, NULL);
				if (SendResponseCreatedSession(procId, inId, outId))
				{
				}				
			}
			else
			{
				delete instance;
			}
		}
		return true;
	}
	else if (auto createdSession = msg->Cast<SpiBridgeStandardMessageResponseCreatedSession>())
	{
		//セッション作成応答
		if (newSessionResultCallback_)
		{
			auto procId = createdSession->procId;
			Name256Buffer inName, outName;
			CraetePipeName(procId, createdSession->inId, L"_InBridgePipe", inName);
			CraetePipeName(procId, createdSession->outId, L"_InBridgePipe", outName);
			auto instance = new CInstanceSession();
			if(instance->InitClient(inName.data(), outName.data(), true))
			{
				newSessionResultCallback_(instance, newSessionResultCallbackArg_);
			}
			else
			{
				delete instance;
				newSessionResultCallback_(nullptr, newSessionResultCallbackArg_);
			}
		}
	}
	else if (auto failedSession = msg->Cast<SpiBridgeStandardMessageResponseFailedNewSession>())
	{
		//セッション作成応答
		if (newSessionResultCallback_)
		{
			newSessionResultCallback_(nullptr, newSessionResultCallbackArg_);
		}
	}
	else if (auto freeSessionMsg = msg->Cast<SpiBridgeStandardMessageFreeSession>())
	{
		//セッション解放
		return true;
	}
	else if (auto exitMsg = msg->Cast<SpiBridgeStandardMessageExit>())
	{
		//終了
		exitCode_ = 0;
		return false;
	}
	return msg != nullptr;
}

/// <summary>
/// return exit code
/// </summary>
/// <returns></returns>
int CStandardSession::ExitCode() const
{
	return exitCode_;
}

/// <summary>
/// 何もしないメッセージ送信
/// </summary>
bool CStandardSession::SendNone()
{
	SpiBridgeStandardMessageNone msg;
	msg.Init();
	return SendCommMessage(msg);
}

/// <summary>
/// セッションの終了メッセージを送信します。
/// </summary>
bool CStandardSession::SendExit()
{
	SpiBridgeStandardMessageExit msg;
	msg.Init();
	return SendCommMessage(msg);
}

/// <summary>
/// インスタンスセッション作成要求メッセージ送信
/// </summary>
/// <returns></returns>
bool CStandardSession::SendNewSession()
{
	if(!newSessionResultCallback_)
	{
		//コールバックが設定されていない場合は送信しない
		return false;
	}

	SpiBridgeStandardMessageNewSession msg;
	msg.Init();
	return SendCommMessage(msg);
}

/// <summary>
/// インスタンスセッション作成応答メッセージ送信
/// </summary>
/// <param name="_ProocId"></param>
/// <param name="_InPipeId"></param>
/// <param name="_OutPipeId"></param>
/// <returns></returns>
bool CStandardSession::SendResponseCreatedSession(int _ProocId, uint32_t _InPipeId, uint32_t _OutPipeId)
{
	SpiBridgeStandardMessageResponseCreatedSession msg;
	msg.Init();
	msg.procId = _ProocId;
	msg.inId = _InPipeId;
	msg.outId = _OutPipeId;
	return SendCommMessage(msg);
}

/// <summary>
/// セッションIDに基づいてパイプ名を生成します。
/// </summary>
/// <param name="_SessionId">パイプ名の生成に使用するセッションID。</param>
/// <param name="name">生成されたパイプ名を格納するName256Buffer型の参照。</param>
bool CStandardSession::CraetePipeName(int _ProcId, int _SessionId, const wchar_t* _SuffixStr, Name256Buffer& _Name)
{
	if (StringFromGUID2(s_NamedPipeGuid, _Name.data(), (int)_Name.size()) == 0)
		return false;
		
	wchar_t addStr[64] = {};
	swprintf_s(addStr, L"_%x_%s_%x", _ProcId, _SuffixStr, _SessionId);
	wcscat_s(_Name.data(), _Name.size(), addStr);

	return true;
}
