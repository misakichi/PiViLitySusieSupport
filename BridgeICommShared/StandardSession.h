#pragma once
#include "SessionMessage.h"
#include "BridgeMessage.h"

class CStandardSession : public CSessionMessage
{
public:
	CStandardSession();
	~CStandardSession();
	SpiBridgeStandardMessageHeader* ReadMessage();
	bool Update();
	int ExitCode() const;

	bool SendNone();
	bool SendExit();

private:
	int exitCode_ = -1;
};

