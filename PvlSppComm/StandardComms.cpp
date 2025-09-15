#include "stdafx.h"
#include "StandardComms.h"

#include "BridgeMessage.h"
#include "StandardSession.h"

using namespace PvlSppComm;

Pipe::Pipe()
{
	SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };
	CreatePipe(&read_, &write_, &sa, 0);
}

Pipe::~Pipe()
{
	CloseHandle(read_);
	CloseHandle(write_);
}


StandardComms::StandardComms()
{
}

bool StandardComms::Initialize()
{
	if (isRunnning_)
		return false;

	pipeIn_ = new Pipe();
	pipeOut_ = new Pipe();
	//pipeErr_ = new Pipe();

    STARTUPINFOW si = {};
    si.cb =sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdOutput = pipeOut_->write_;
    si.hStdInput = pipeIn_->read_;
    //si.hStdError = pipeErr_->write_;

    PROCESS_INFORMATION pi = {};

    const wchar_t* cmd = L"PvlWin32SpiBridge.exe";
    wchar_t command[MAX_PATH];
    wcscpy_s(command, cmd);
    isRunnning_ = CreateProcessW(
        nullptr,
        command,
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi
    );
    if (isRunnning_)
    {
        CloseHandle(pi.hThread);
        process_ = pi.hProcess;
        session_ = new CStandardSession();
        session_->InitIo(pipeOut_->read_, pipeIn_->write_);
    }


	return isRunnning_;
}

bool StandardComms::Terminate()
{
    if (isRunnning_)
    {
        SendExit();

        WaitForSingleObject(process_, 60 * 1000);//1 minites
        delete session_;
        delete pipeIn_;
        delete pipeOut_;
        delete pipeErr_;
        CloseHandle(process_);
        pipeIn_ = nullptr;
        pipeOut_ = nullptr;
        pipeErr_ = nullptr;
        process_ = nullptr;
        session_ = nullptr;
        isRunnning_ = false;
    }
    return true;
}


bool StandardComms::SendNone()
{
    return session_->SendNone();
}

bool StandardComms::SendExit()
{
    return session_->SendExit();
}
