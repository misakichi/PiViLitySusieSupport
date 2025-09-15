#include "stdafx.h"

#include "BridgeMessage.h"
#include "StandardSession.h"

int WINAPI WinMain(
	_In_ HINSTANCE hInstance, 
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ LPSTR lpCmdLine, 
	_In_ int nShowCmd)
{
	CStandardSession session;
	session.InitFromStdIo();
	while (session.Update())
		;
		
	//���b�Z�[�W�؂�ɂ��I���͕s��
	auto exitCode =  session.ExitCode();
	return exitCode;
}
