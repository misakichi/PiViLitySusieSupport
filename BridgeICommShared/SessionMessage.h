#pragma once

struct SpiBridgeMessageHeader;

class CSessionMessage
{
public:
	CSessionMessage();
	~CSessionMessage();
	void InitIo(HANDLE in, HANDLE out);
	void InitFromStdIo();
protected:
	SpiBridgeMessageHeader* ReadMessageBase();
	bool SendCommMessage(const SpiBridgeMessageHeader& msg);

private:
	HANDLE	in_;
	HANDLE	out_;

};

