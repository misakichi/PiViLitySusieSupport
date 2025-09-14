#include "stdafx.h"
#include "StandardSession.h"

CStandardSession::CStandardSession()
{
	InitIo(GetStdHandle(STD_INPUT_HANDLE), GetStdHandle(STD_OUTPUT_HANDLE));
}

CStandardSession::~CStandardSession()
{
}

/// <summary>
/// ���b�Z�[�W�̓ǂݎ��
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
		//�V�K�Z�b�V����
		return true;
	}
	else if (auto freeSessionMsg = msg->Cast<SpiBridgeStandardMessageFreeSession>())
	{
		//�Z�b�V�������
		return true;
	}
	else if (auto exitMsg = msg->Cast<SpiBridgeStandardMessageExit>())
	{
		//�I��
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
