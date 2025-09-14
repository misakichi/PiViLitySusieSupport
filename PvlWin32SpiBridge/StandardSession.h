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

private:
	int exitCode_ = -1;
};

