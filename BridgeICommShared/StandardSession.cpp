#include "stdafx.h"
#include "StandardSession.h"

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
		return true;
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

